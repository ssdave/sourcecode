/**
 * @file mmio_uart_driver.c
 * @brief Memory-Mapped I/O (MMIO) Hardware Peripheral Controller.
 *
 * Concepts Demonstrated:
 * 1. Memory-Mapped I/O (MMIO) register layout using volatile hardware structs.
 * 2. Polling-driven peripheral TX/RX data transfers and status synchronization.
 * 3. Atomic bit set/reset operations (BSRR) avoiding read-modify-write race conditions.
 * 4. Peripheral error detection (Overrun, Parity, Framing).
 *
 * Academic Attribution:
 * Exercise design based on computer architecture and hardware interfacing principles
 * (CS:APP Chapter 6, ARM Architecture Reference Manual, and OSTEP Chapter 36).
 * Retained strictly for independent educational self-study.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==============================================================================
 * Hardware Specification & Register Memory Map
 * ==============================================================================
 *
 * 1. Register Memory Map
 * ------------------------------------------------------------------------------
 * Contiguous 16-byte memory block aligned to a 32-bit boundary.
 * All registers are 32 bits wide and must be accessed via 32-bit word loads
 * and stores (volatile uint32_t *).
 *
 * +--------+--------------------------+---------+-------------+-------------+
 * | Offset | Register Name            | Acronym | Access Mode | Reset Value |
 * +--------+--------------------------+---------+-------------+-------------+
 * | 0x00   | Status Register          | SR      | Read-Only   | 0x00000080  |
 * | 0x04   | Data Register            | DR      | Read/Write  | 0x00000000  |
 * | 0x08   | Control Register         | CR      | Read/Write  | 0x00000000  |
 * | 0x0C   | Bit Set / Reset Register | BSRR    | Write-Only  | 0x00000000  |
 * +--------+--------------------------+---------+-------------+-------------+
 *
 * 2. Register Bitfield Layouts
 * ------------------------------------------------------------------------------
 *
 * Status Register (SR) - Offset 0x00:
 *  31                                 8   7     6    5     4    3    2    1    0
 * +------------------------------------+-----+-----+-----+----+----+----+----+----+
 * |              RESERVED              | TXE |  -  |RXNE | -  |ORE | -  | FE | PE |
 * +------------------------------------+-----+-----+-----+----+----+----+----+----+
 *  - PE   (Bit 0): Parity Error flag. Hardware sets 1 on parity mismatch.
 *  - FE   (Bit 1): Framing Error flag. Hardware sets 1 when stop bit missing.
 *  - ORE  (Bit 3): Overrun Error flag. Hardware sets 1 when new byte overwrites unread DR.
 *  - RXNE (Bit 5): Read Data Register Not Empty. 1 = byte ready in DR; cleared on DR read.
 *  - TXE  (Bit 7): Transmit Data Register Empty. 1 = buffer ready for next write to DR.
 *
 * Data Register (DR) - Offset 0x04:
 *  31                                                                    8 7     0
 * +-----------------------------------------------------------------------+-------+
 * |                               RESERVED                                | DATA  |
 * +-----------------------------------------------------------------------+-------+
 *  - Bits [7:0]: 8-bit serial data payload.
 *  - Write to DR: Transmits byte, automatically de-asserts TXE in hardware.
 *  - Read from DR: Fetches received byte, automatically de-asserts RXNE in hardware.
 *
 * Control Register (CR) - Offset 0x08:
 *  31                                                  5    4    3    2      1     0
 * +-----------------------------------------------------+----+----+------+-----+----+
 * |                      RESERVED                       | RE | TE |RXNEIE|TXEIE| UE |
 * +-----------------------------------------------------+----+----+------+-----+----+
 *  - UE     (Bit 0): UART Enable. 1 = peripheral active; 0 = peripheral in reset.
 *  - TXEIE  (Bit 1): TX Empty Interrupt Enable.
 *  - RXNEIE (Bit 2): RX Not Empty Interrupt Enable.
 *  - TE     (Bit 3): Transmitter Enable (1 = enables serializer / TX line driver).
 *  - RE     (Bit 4): Receiver Enable (1 = enables deserializer / RX sampler).
 *
 * Bit Set / Reset Register (BSRR) - Offset 0x0C:
 *  31                            16 15                             0
 * +--------------------------------+--------------------------------+
 * |       CLEAR BITS (BR[15:0])    |       SET BITS (BS[15:0])      |
 * +--------------------------------+--------------------------------+
 *  - BS[15:0] (Bits [15:0]):  Writing 1 to bit y sets bit y in CR (CR |= (1 << y)).
 *                             Writing 0 has no effect.
 *  - BR[15:0] (Bits [31:16]): Writing 1 to bit (y + 16) clears bit y in CR (CR &= ~(1 << y)).
 *                             Writing 0 has no effect.
 *  - Hardware Invariant: Writing to BSRR provides atomic single-cycle bit modification
 *    without requiring a read-modify-write cycle on CR. BSRR is write-only.
 *
 * 3. Peripheral Operational Protocols
 * ------------------------------------------------------------------------------
 * - Initialization: Validate base address pointer, configure CR with UE, TE, and RE.
 * - Polling Transmission: Spin on SR until TXE is asserted (or timeout). Write byte to DR.
 * - Polling Reception: Check SR error flags (ORE, FE, PE). Spin on SR until RXNE is
 *   asserted (or timeout). Read byte from DR.
 * - Atomic Control Modification: Use BSRR lower halfword to set bits and upper halfword
 *   to clear bits without software read-modify-write races.
 * ============================================================================== */

/* ==============================================================================
 * Hardware Register Definitions & Bitmasks
 * ============================================================================== */

/* Status Register (SR) bitmasks */
#define MMIO_UART_SR_PE (1U << 0)   /* Parity Error */
#define MMIO_UART_SR_FE (1U << 1)   /* Framing Error */
#define MMIO_UART_SR_ORE (1U << 3)  /* Overrun Error */
#define MMIO_UART_SR_RXNE (1U << 5) /* Read Data Register Not Empty */
#define MMIO_UART_SR_TXE (1U << 7)  /* Transmit Data Register Empty */

/* Control Register (CR) bitmasks */
#define MMIO_UART_CR_UE (1U << 0)     /* UART Enable */
#define MMIO_UART_CR_TXEIE (1U << 1)  /* TX Empty Interrupt Enable */
#define MMIO_UART_CR_RXNEIE (1U << 2) /* RX Not Empty Interrupt Enable */
#define MMIO_UART_CR_TE (1U << 3)     /* Transmitter Enable */
#define MMIO_UART_CR_RE (1U << 4)     /* Receiver Enable */

/**
 * MMIO Peripheral Register Layout
 * All fields are 32-bit word-aligned and declared volatile.
 */
typedef struct
{
  volatile uint32_t SR;   /* Offset 0x00: Status Register */
  volatile uint32_t DR;   /* Offset 0x04: Data Register */
  volatile uint32_t CR;   /* Offset 0x08: Control Register */
  volatile uint32_t BSRR; /* Offset 0x0C: Bit Set / Reset Register */
} mmio_uart_regs_t;

/* Maximum polling spin count before reporting timeout */
#define MMIO_UART_TIMEOUT_CYCLES 100000

/* ==============================================================================
 * Driver Function Declarations & Skeletons
 * ============================================================================== */

/**
 * Initializes the UART hardware peripheral at the specified base address.
 *
 * Contract & Requirements:
 * - Validates base_addr pointer.
 * - Configures initial peripheral state, enabling transmitter, receiver, and main subsystem.
 *
 * @param base_addr Pointer to the MMIO peripheral register block.
 */
// AI Review Feedback: Global Peripheral State vs Multi-Instance Hardware:
// A single global boolean `init` functions for single-instance unit testing, but in
// multi-peripheral SoCs (e.g., UART0 and UART1), it is not multi-instance safe. In production
// hardware drivers, peripheral state is queried directly from hardware registers
// (e.g., `(regs->CR & MMIO_UART_CR_UE) != 0`) or tracked per-instance in a device context struct.
bool init = false;
void mmio_uart_init(void *base_addr)
{
  /* TODO: Author Implementation */
  if(base_addr == NULL)
  {
    return;
  }

  volatile mmio_uart_regs_t *regs = (volatile mmio_uart_regs_t *)base_addr;

  regs->CR = MMIO_UART_CR_UE | MMIO_UART_CR_TE | MMIO_UART_CR_RE;
  init = true;

  return;
}

/**
 * Transmits a single byte over UART using polling status synchronization.
 *
 * Contract & Requirements:
 * - Polls until the transmit data register is empty and ready for data.
 * - Detects peripheral errors and returns a negative error code if an error occurs.
 * - Writes byte into the data register when ready and returns 0 on success.
 * - Aborts with -1 if polling timeout expires.
 *
 * @param base_addr Pointer to the MMIO peripheral register block.
 * @param byte Data byte to transmit.
 * @return 0 on success, or negative error code on failure/timeout.
 */
int mmio_uart_send_byte_polling(void *base_addr, uint8_t byte)
{
  /* TODO: Author Implementation */
  uint32_t timeout = MMIO_UART_TIMEOUT_CYCLES;

  if((base_addr == NULL) || (init == false))
  {
    return -1;
  }

  volatile mmio_uart_regs_t *regs = (volatile mmio_uart_regs_t *)base_addr;

  while((regs->SR & MMIO_UART_SR_TXE) != MMIO_UART_SR_TXE)
  {
    if(--timeout == 0)
    {
      return -1;
    }
  }

  regs->DR = byte;

  return 0;
}

/**
 * Reads a single byte from UART using polling status synchronization.
 *
 * Contract & Requirements:
 * - Polls until incoming data is available in the receive data register.
 * - Checks for parity, framing, or overrun hardware error flags, returning negative code on error.
 * - Retrieves received byte into out_byte and returns 0 on success.
 * - Aborts with -1 if polling timeout expires.
 *
 * @param base_addr Pointer to the MMIO peripheral register block.
 * @param out_byte Pointer to destination storage for received byte.
 * @return 0 on success, or negative error code on failure/timeout.
 */
int mmio_uart_read_byte_polling(void *base_addr, uint8_t *out_byte)
{
  /* TODO: Author Implementation */
  uint32_t timeout = MMIO_UART_TIMEOUT_CYCLES;

  if((base_addr == NULL) || (out_byte == NULL) || (init == false))
  {
    return -1;
  }

  volatile mmio_uart_regs_t *regs = (volatile mmio_uart_regs_t *)base_addr;

  // AI Review Feedback: Fail-Fast Hardware Error Detection vs Polling Timeout:
  // Checking error flags (PE, FE, ORE) after the while loop means if hardware experiences an
  // overrun (ORE) or framing error (FE) where RXNE is not asserted, the driver spins through all
  // 100,000 cycles before exiting on timeout rather than immediately failing fast.
  // Checking `(regs->SR & (PE | FE | ORE))` inside the spin loop provides immediate fail-fast
  // error response (0-1 cycles vs 100k cycles) and enables distinct status codes
  // (e.g., -1 for timeout vs -2 for peripheral hardware error) for clearer debugging.
  while((regs->SR & MMIO_UART_SR_RXNE) != MMIO_UART_SR_RXNE)
  {
    if(--timeout == 0)
    {
      return -1;
    }
  }

  if((regs->SR & MMIO_UART_SR_PE) || (regs->SR & MMIO_UART_SR_FE) || (regs->SR & MMIO_UART_SR_ORE))
  {
    return -1;
  }

  *out_byte = regs->DR & 0xFF;

  return 0;
}

/**
 * Atomically sets control register flags via hardware bit-set-reset register.
 *
 * Contract & Requirements:
 * - Sets the requested control flags without read-modify-write race conditions.
 *
 * @param base_addr Pointer to the MMIO peripheral register block.
 * @param flags Bitmask of control flags to enable.
 */
#define MMIO_UART_CR_VALID_MASK                                                                    \
  (MMIO_UART_CR_UE | MMIO_UART_CR_TXEIE | MMIO_UART_CR_RXNEIE | MMIO_UART_CR_TE | MMIO_UART_CR_RE)

void mmio_uart_set_control_flags_atomic(void *base_addr, uint32_t flags)
{
  /* TODO: Author Implementation */
  if(base_addr == NULL)
  {
    return;
  }

  volatile mmio_uart_regs_t *regs = (volatile mmio_uart_regs_t *)base_addr;
  regs->BSRR = flags & MMIO_UART_CR_VALID_MASK;
  return;
}

/**
 * Atomically clears control register flags via hardware bit-set-reset register.
 *
 * Contract & Requirements:
 * - Clears the requested control flags without read-modify-write race conditions.
 *
 * @param base_addr Pointer to the MMIO peripheral register block.
 * @param flags Bitmask of control flags to disable.
 */
void mmio_uart_clear_control_flags_atomic(void *base_addr, uint32_t flags)
{
  /* TODO: Author Implementation */
  if(base_addr == NULL)
  {
    return;
  }

  volatile mmio_uart_regs_t *regs = (volatile mmio_uart_regs_t *)base_addr;
  regs->BSRR = (flags & MMIO_UART_CR_VALID_MASK) << 16;
  return;
}

/* ==============================================================================
 * Test Harness & Hardware Simulation
 * ============================================================================== */

static void mock_hw_clock_step(mmio_uart_regs_t *hw)
{
  /* Hardware latch simulation: updates CR atomically from BSRR */
  uint32_t set_bits = hw->BSRR & 0xFFFF;
  uint32_t clear_bits = (hw->BSRR >> 16) & 0xFFFF;

  if(set_bits != 0)
  {
    hw->CR |= set_bits;
  }
  if(clear_bits != 0)
  {
    hw->CR &= ~clear_bits;
  }
  hw->BSRR = 0;
}

int main(void)
{
  printf("=== Test Suite: MMIO UART Hardware Peripheral Driver ===\n");

  mmio_uart_regs_t mock_hw;
  memset(&mock_hw, 0, sizeof(mock_hw));

  mmio_uart_init(&mock_hw);
  mock_hw_clock_step(&mock_hw);

  /* Check if implementation is still at initial scaffold state */
  if(mock_hw.CR == 0)
  {
    printf("[Exercise Scaffold] mmio_uart_init() not yet implemented.\n");
    printf("                     Awaiting author implementation.\n");
    return 0;
  }

  printf("[Verification] Checking peripheral initialization...\n");
  assert((mock_hw.CR & MMIO_UART_CR_UE) != 0);
  assert((mock_hw.CR & MMIO_UART_CR_TE) != 0);
  assert((mock_hw.CR & MMIO_UART_CR_RE) != 0);

  printf("[Verification] Checking atomic BSRR bit set operation...\n");
  mmio_uart_set_control_flags_atomic(&mock_hw, MMIO_UART_CR_TXEIE);
  mock_hw_clock_step(&mock_hw);
  assert((mock_hw.CR & MMIO_UART_CR_TXEIE) != 0);

  printf("[Verification] Checking atomic BSRR bit clear operation...\n");
  mmio_uart_clear_control_flags_atomic(&mock_hw, MMIO_UART_CR_TXEIE);
  mock_hw_clock_step(&mock_hw);
  assert((mock_hw.CR & MMIO_UART_CR_TXEIE) == 0);

  printf("[Verification] Checking polling transmission...\n");
  mock_hw.SR = MMIO_UART_SR_TXE;
  int tx_res = mmio_uart_send_byte_polling(&mock_hw, 0xA5);
  assert(tx_res == 0);
  assert((uint8_t)mock_hw.DR == 0xA5);

  printf("[Verification] Checking polling reception...\n");
  mock_hw.SR = MMIO_UART_SR_RXNE;
  mock_hw.DR = 0x5A;
  uint8_t rx_data = 0;
  int rx_res = mmio_uart_read_byte_polling(&mock_hw, &rx_data);
  assert(rx_res == 0);
  assert(rx_data == 0x5A);

  printf("[Verification] Checking hardware error detection on overrun...\n");
  mock_hw.SR = MMIO_UART_SR_ORE;
  rx_res = mmio_uart_read_byte_polling(&mock_hw, &rx_data);
  assert(rx_res < 0);

  printf("=== All MMIO UART Driver verification tests passed successfully! ===\n");
  return 0;
}
