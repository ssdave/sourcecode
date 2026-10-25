/**
 * @file spi_bitbang_driver.c
 * @brief Full-Duplex GPIO Bit-Banged SPI Bus Controller and Peripheral Driver.
 *
 * Concepts Demonstrated:
 * 1. Synchronous 4-wire serial bus protocol: SCLK, MOSI, MISO, and CS (active-low).
 * 2. Complete SPI Mode support: Mode 0, Mode 1, Mode 2, and Mode 3 (CPOL and CPHA matrix).
 * 3. Simultaneous full-duplex 8-bit MSB-first byte transmission and reception.
 * 4. Multi-byte buffer streaming and chip-select framing invariants.
 * 5. High-level peripheral command orchestration against a mock SPI Flash/EEPROM device
 *    (JEDEC ID readout, status register polling, write latching, and memory read/write).
 *
 * SPI (Serial Peripheral Interface) Bus Protocol & Timing Invariants:
 *
 * 4-Wire Physical Bus Lines:
 * - SCLK (Serial Clock): Master-driven clock signal.
 * - MOSI (Master Out Slave In): Master-driven data line (push-pull).
 * - MISO (Master In Slave Out): Slave-driven data line (tri-stated when CS is HIGH).
 * - CS_N (Chip Select): Master-driven active-low slave selection line.
 *
 * SPI Mode Matrix:
 * +------+------+------+-----------+-------------------+--------------------+
 * | Mode | CPOL | CPHA | SCLK Idle | Sample Edge       | Shift/Setup Edge   |
 * +------+------+------+-----------+-------------------+--------------------+
 * |  0   |  0   |  0   |  LOW (0)  | Leading (Rising)  | Trailing (Falling) |
 * |  1   |  0   |  1   |  LOW (0)  | Trailing (Falling)| Leading (Rising)   |
 * |  2   |  1   |  0   |  HIGH (1) | Leading (Falling) | Trailing (Rising)  |
 * |  3   |  1   |  1   |  HIGH (1) | Trailing (Rising) | Leading (Falling)  |
 * +------+------+------+-----------+-------------------+--------------------+
 *
 * Mode 0 (CPOL=0, CPHA=0) Timing Diagram:
 *
 *         +-------------------------------------------------------+
 *   CS:   |                                                       \______ Active LOW ______/
 *
 *                 +---+       +---+       +---+       +---+
 *   SCLK: ________/   \_______/   \_______/   \_______/   \_______/   \_______ (Idle LOW)
 *
 *           +-------+   +-------+   +-------+   +-------+
 *   MOSI: --| Bit 7 |---| Bit 6 |---| Bit 5 |---| Bit 4 |---  (Setup before SCLK rise)
 *           +-------+   +-------+   +-------+   +-------+
 *               ^           ^           ^           ^
 *             Sample      Sample      Sample      Sample (Sampled on RISING edge)
 *
 * Mode 3 (CPOL=1, CPHA=1) Timing Diagram:
 *
 *         +-------------------------------------------------------+
 *   CS:   |                                                       \______ Active LOW ______/
 *
 *         --------\   /-------\   /-------\   /-------\   /-------+
 *   SCLK:         \___/       \___/       \___/       \___/       (Idle HIGH)
 *
 *                 +-------+   +-------+   +-------+   +-------+
 *   MOSI: --------| Bit 7 |---| Bit 6 |---| Bit 5 |---| Bit 4 |-- (Shift on FALLING edge)
 *                 +-------+   +-------+   +-------+   +-------+
 *                     ^           ^           ^           ^
 *                   Sample      Sample      Sample      Sample (Sampled on RISING edge)
 *
 * Academic Attribution:
 * Exercise design based on computer architecture and embedded peripheral bus standards
 * (CS:APP Chapter 6, OSTEP Chapter 36, and Motorola/NXP SPI Block Guide V04.01).
 * Retained strictly for independent educational self-study.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==============================================================================
 * Hardware Abstraction Layer (HAL) & Protocol Definitions
 * ============================================================================== */

/**
 * SPI Operating Modes (Clock Polarity & Clock Phase)
 */
typedef enum
{
  SPI_MODE_0 = 0, /* CPOL = 0, CPHA = 0: Idle LOW, sample leading (rising)  */
  SPI_MODE_1 = 1, /* CPOL = 0, CPHA = 1: Idle LOW, sample trailing (falling) */
  SPI_MODE_2 = 2, /* CPOL = 1, CPHA = 0: Idle HIGH, sample leading (falling) */
  SPI_MODE_3 = 3  /* CPOL = 1, CPHA = 1: Idle HIGH, sample trailing (rising)  */
} spi_mode_t;

/**
 * Standard SPI Flash / EEPROM Command Opcodes
 */
#define SPI_CMD_WREN 0x06  /* Write Enable Latch */
#define SPI_CMD_WRDI 0x04  /* Write Disable */
#define SPI_CMD_RDSR 0x05  /* Read Status Register */
#define SPI_CMD_WRSR 0x01  /* Write Status Register */
#define SPI_CMD_READ 0x03  /* Read Memory Array */
#define SPI_CMD_WRITE 0x02 /* Page Program / Write Memory Array */
#define SPI_CMD_RDID 0x9F  /* Read JEDEC Identification (3 bytes) */

/**
 * SPI Flash Status Register Bitmasks
 */
#define SPI_SR_WIP 0x01 /* Write In Progress (busy) */
#define SPI_SR_WEL 0x02 /* Write Enable Latch status */

/**
 * Mock SPI Slave Protocol States
 */
typedef enum
{
  SLAVE_STATE_IDLE = 0,
  SLAVE_STATE_CMD,
  SLAVE_STATE_ADDR,
  SLAVE_STATE_DATA_IN,
  SLAVE_STATE_DATA_OUT,
  SLAVE_STATE_READ_ID,
  SLAVE_STATE_READ_STATUS
} mock_spi_slave_state_t;

/**
 * Mock SPI Slave Device Model (256-byte Memory, Status Reg, JEDEC ID)
 */
typedef struct
{
  mock_spi_slave_state_t state;
  uint8_t memory[256];
  uint8_t status_reg;
  uint8_t jedec_id[3]; /* [0] = Manufacturer, [1] = Memory Type, [2] = Capacity */
  uint8_t current_cmd;
  uint8_t current_addr;
  uint8_t shift_in;  /* Shift register for incoming MOSI bits */
  uint8_t shift_out; /* Shift register for outgoing MISO bits */
  uint8_t bit_count; /* Bits shifted in current byte */
  uint8_t id_index;  /* Index for multi-byte ID readout */
  bool selected;     /* True when CS is LOW */
  spi_mode_t mode;   /* Slave configured SPI mode */
} mock_spi_slave_t;

/**
 * SPI Bus Controller Context
 */
typedef struct
{
  uint8_t sclk;           /* SCLK line state: 0 or 1 */
  uint8_t mosi;           /* MOSI line state: 0 or 1 */
  uint8_t miso;           /* MISO line state: 0 or 1 */
  uint8_t cs_n;           /* CS line state: 0 (asserted/active) or 1 (idle) */
  spi_mode_t mode;        /* Configured SPI mode (0, 1, 2, or 3) */
  mock_spi_slave_t slave; /* Connected mock peripheral */
  uint32_t clock_ticks;   /* Total clock edge transitions recorded */
} spi_bus_t;

/* Forward declaration for internal mock slave event dispatcher */
static void mock_slave_on_sclk_edge(spi_bus_t *bus, uint8_t edge_type);
static void mock_slave_on_cs_change(spi_bus_t *bus, uint8_t old_cs, uint8_t new_cs);

/**
 * Samples the physical logic level of the MISO data line.
 *
 * @param bus Pointer to the SPI bus structure.
 * @return 1 or 0 (or 1 if CS is inactive / tri-state pulled high).
 */
uint8_t spi_hal_get_miso(const spi_bus_t *bus)
{
  if(bus == NULL)
  {
    return 1;
  }
  return bus->miso;
}

/**
 * Sets the master driver level for the SCLK clock line.
 *
 * @param bus Pointer to the SPI bus structure.
 * @param level 0 for LOW, 1 for HIGH.
 */
void spi_hal_set_sclk(spi_bus_t *bus, uint8_t level)
{
  if(bus == NULL)
  {
    return;
  }

  uint8_t old_sclk = bus->sclk;
  bus->sclk = (level != 0) ? 1 : 0;

  if(old_sclk != bus->sclk)
  {
    bus->clock_ticks++;
    /* edge_type: 1 = rising (0 -> 1), 0 = falling (1 -> 0) */
    mock_slave_on_sclk_edge(bus, bus->sclk);
  }
}

/**
 * Sets the master driver level for the MOSI data line.
 *
 * @param bus Pointer to the SPI bus structure.
 * @param level 0 for LOW, 1 for HIGH.
 */
void spi_hal_set_mosi(spi_bus_t *bus, uint8_t level)
{
  if(bus == NULL)
  {
    return;
  }
  bus->mosi = (level != 0) ? 1 : 0;
}

/**
 * Sets the master driver level for the active-low CS line.
 *
 * @param bus Pointer to the SPI bus structure.
 * @param level 0 to assert CS (select device), 1 to deassert CS (deselect device).
 */
void spi_hal_set_cs(spi_bus_t *bus, uint8_t level)
{
  if(bus == NULL)
  {
    return;
  }

  uint8_t old_cs = bus->cs_n;
  bus->cs_n = (level != 0) ? 1 : 0;

  if(old_cs != bus->cs_n)
  {
    mock_slave_on_cs_change(bus, old_cs, bus->cs_n);
  }
}

/**
 * Simulated bus timing delay for half-clock cycles and setup/hold intervals.
 */
void spi_hal_delay(void)
{
  /* Simulated hardware propagation and clock period delay */
}

/* ==============================================================================
 * Mock SPI Slave Peripheral Implementation
 * ============================================================================== */

/**
 * Initializes the mock SPI slave device (JEDEC ID, initial registers).
 *
 * @param slv Pointer to the mock slave structure.
 * @param mode Configured SPI mode matching the controller.
 */
void mock_spi_slave_init(mock_spi_slave_t *slv, spi_mode_t mode)
{
  if(slv == NULL)
  {
    return;
  }

  memset(slv, 0, sizeof(*slv));
  slv->mode = mode;
  slv->state = SLAVE_STATE_IDLE;
  slv->selected = false;
  slv->status_reg = 0x00;

  /* Standard JEDEC Flash ID: Manufacturer 0xEF, Memory Type 0x40, Capacity 0x18 */
  slv->jedec_id[0] = 0xEF;
  slv->jedec_id[1] = 0x40;
  slv->jedec_id[2] = 0x18;

  /* Prepopulate mock memory array with known test patterns */
  for(size_t i = 0; i < sizeof(slv->memory); i++)
  {
    slv->memory[i] = (uint8_t)(i ^ 0xAA);
  }
}

/**
 * Internal handler invoked on CS state transitions.
 */
static void mock_slave_on_cs_change(spi_bus_t *bus, uint8_t old_cs, uint8_t new_cs)
{
  mock_spi_slave_t *slv = &bus->slave;

  /* CS Falling Edge (1 -> 0): Device selected */
  if(old_cs == 1 && new_cs == 0)
  {
    slv->selected = true;
    slv->state = SLAVE_STATE_CMD;
    slv->bit_count = 0;
    slv->shift_in = 0;
    slv->shift_out = 0xFF;
    slv->id_index = 0;
    bus->miso = 1;

    /* For CPHA=0: First MISO bit must be presented immediately upon CS assertion */
    uint8_t cpha = (uint8_t)(slv->mode & 0x01);
    if(cpha == 0)
    {
      /* Initial default MISO state */
      bus->miso = 1;
    }
  }
  /* CS Rising Edge (0 -> 1): Device deselected */
  else if(old_cs == 0 && new_cs == 1)
  {
    slv->selected = false;
    slv->state = SLAVE_STATE_IDLE;
    slv->bit_count = 0;
    bus->miso = 1; /* Tri-state pulled high */
  }
}

/**
 * Internal handler invoked on every SCLK edge transition.
 */
static void mock_slave_on_sclk_edge(spi_bus_t *bus, uint8_t edge_type)
{
  mock_spi_slave_t *slv = &bus->slave;
  if(!slv->selected)
  {
    return;
  }

  uint8_t cpol = (uint8_t)((slv->mode >> 1) & 0x01);
  uint8_t cpha = (uint8_t)(slv->mode & 0x01);

  /*
   * Determine whether this edge is a sample edge or a shift edge based on mode:
   * Mode 0 (CPOL=0, CPHA=0): Sample on Rising (1), Shift on Falling (0)
   * Mode 1 (CPOL=0, CPHA=1): Shift on Rising (1), Sample on Falling (0)
   * Mode 2 (CPOL=1, CPHA=0): Sample on Falling (0), Shift on Rising (1)
   * Mode 3 (CPOL=1, CPHA=1): Shift on Falling (0), Sample on Rising (1)
   */
  bool is_sample_edge = false;
  if(cpha == 0)
  {
    /* CPHA = 0: Sample on leading edge (edge_type != cpol) */
    is_sample_edge = (edge_type != cpol);
  }
  else
  {
    /* CPHA = 1: Sample on trailing edge (edge_type == cpol) */
    is_sample_edge = (edge_type == cpol);
  }

  if(is_sample_edge)
  {
    /* Shift in MOSI bit (MSB first) */
    slv->shift_in = (uint8_t)((slv->shift_in << 1) | (bus->mosi & 0x01));
    slv->bit_count++;

    if(slv->bit_count == 8)
    {
      /* Completed 8-bit byte reception */
      uint8_t received_byte = slv->shift_in;
      slv->bit_count = 0;
      slv->shift_in = 0;

      switch(slv->state)
      {
      case SLAVE_STATE_CMD:
        slv->current_cmd = received_byte;
        if(received_byte == SPI_CMD_RDID)
        {
          slv->state = SLAVE_STATE_READ_ID;
          slv->id_index = 0;
          slv->shift_out = slv->jedec_id[0];
        }
        else if(received_byte == SPI_CMD_RDSR)
        {
          slv->state = SLAVE_STATE_READ_STATUS;
          slv->shift_out = slv->status_reg;
        }
        else if(received_byte == SPI_CMD_WREN)
        {
          slv->status_reg |= SPI_SR_WEL;
          slv->state = SLAVE_STATE_IDLE;
        }
        else if(received_byte == SPI_CMD_WRDI)
        {
          slv->status_reg &= (uint8_t)~SPI_SR_WEL;
          slv->state = SLAVE_STATE_IDLE;
        }
        else if(received_byte == SPI_CMD_READ)
        {
          slv->state = SLAVE_STATE_ADDR;
        }
        else if(received_byte == SPI_CMD_WRITE)
        {
          slv->state = SLAVE_STATE_ADDR;
        }
        else
        {
          slv->state = SLAVE_STATE_IDLE;
        }
        break;

      case SLAVE_STATE_ADDR:
        slv->current_addr = received_byte;
        if(slv->current_cmd == SPI_CMD_READ)
        {
          slv->state = SLAVE_STATE_DATA_OUT;
          slv->shift_out = slv->memory[slv->current_addr];
        }
        else if(slv->current_cmd == SPI_CMD_WRITE)
        {
          slv->state = SLAVE_STATE_DATA_IN;
        }
        break;

      case SLAVE_STATE_DATA_IN:
        if((slv->status_reg & SPI_SR_WEL) != 0)
        {
          slv->memory[slv->current_addr++] = received_byte;
        }
        break;

      case SLAVE_STATE_DATA_OUT:
        slv->current_addr++;
        slv->shift_out = slv->memory[slv->current_addr];
        break;

      case SLAVE_STATE_READ_ID:
        slv->id_index++;
        if(slv->id_index < 3)
        {
          slv->shift_out = slv->jedec_id[slv->id_index];
        }
        else
        {
          slv->shift_out = 0xFF;
        }
        break;

      case SLAVE_STATE_READ_STATUS:
        slv->shift_out = slv->status_reg;
        break;

      default:
        break;
      }
    }
  }
  else
  {
    /* Shift/Setup Edge: Output next MISO bit (MSB first) */
    bus->miso = (uint8_t)((slv->shift_out >> 7) & 0x01);
    slv->shift_out = (uint8_t)(slv->shift_out << 1);
  }
}

/* ==============================================================================
 * Driver Function Declarations & Detailed Protocol Contracts
 * ============================================================================== */

/**
 * Initializes the SPI bus controller, configures the operating mode, and drives lines to idle.
 *
 * Bus State at Idle:
 * - CS_N is deasserted HIGH (1), deselecting all slave peripherals.
 * - SCLK is driven to its mode-dependent idle state:
 *     * Mode 0 & Mode 1 (CPOL = 0): SCLK idles LOW (0).
 *     * Mode 2 & Mode 3 (CPOL = 1): SCLK idles HIGH (1).
 * - MOSI is driven LOW (0).
 * - MISO is sampled as input (pulled HIGH in idle/tri-state).
 *
 * @param bus Pointer to the SPI bus structure.
 * @param mode SPI operating mode (SPI_MODE_0 through SPI_MODE_3).
 * @return 0 on success, or -1 on invalid argument (bus == NULL or invalid mode).
 */
int spi_init(spi_bus_t *bus, spi_mode_t mode)
{
  (void)bus;
  (void)mode;
  /* TODO: Author Implementation */
  return -1;
}

/**
 * Asserts the active-low Chip Select (CS) line to begin a bus transaction.
 *
 * Physical Line Invariants:
 * - Drives CS_N LOW (0).
 * - Must be called prior to byte/buffer transfers.
 * - For CPHA=0 modes, the first data bit must be placed on MOSI/MISO following CS assertion
 *   before the leading clock edge occurs.
 *
 * @param bus Pointer to the SPI bus structure.
 * @return 0 on success, or -1 on invalid argument (bus == NULL).
 */
int spi_chip_select(spi_bus_t *bus)
{
  (void)bus;
  /* TODO: Author Implementation */
  return -1;
}

/**
 * Deasserts the active-low Chip Select (CS) line to complete a bus transaction.
 *
 * Physical Line Invariants:
 * - Drives CS_N HIGH (1).
 * - Ensures SCLK has returned to its idle level according to the configured CPOL.
 * - Releases the slave's MISO output into tri-state.
 *
 * @param bus Pointer to the SPI bus structure.
 * @return 0 on success, or -1 on invalid argument (bus == NULL).
 */
int spi_chip_deselect(spi_bus_t *bus)
{
  (void)bus;
  /* TODO: Author Implementation */
  return -1;
}

/**
 * Performs a simultaneous full-duplex 8-bit byte transfer over the SPI bus.
 *
 * Architectural Role:
 * Low-level hardware serialization primitive. Transmits an 8-bit payload on MOSI
 * (MSB first, bit 7 down to bit 0) while simultaneously clocking in an 8-bit payload on MISO.
 *
 * Mode Timing Invariants:
 * - Bit serialization is strictly MSB-first.
 * - For CPOL=0, SCLK idle is 0; for CPOL=1, SCLK idle is 1.
 * - For CPHA=0: Data must be set up on MOSI before the leading SCLK edge, sampled on the
 *   leading SCLK edge, and shifted on the trailing SCLK edge.
 * - For CPHA=1: Data is shifted on the leading SCLK edge and sampled on the trailing SCLK edge.
 * - After 8 clock pulses, SCLK must return to its configured idle level.
 *
 * @param bus Pointer to the SPI bus structure.
 * @param tx_byte 8-bit data byte to transmit on MOSI.
 * @return 8-bit data byte received simultaneously on MISO, or 0xFF on error.
 */
uint8_t spi_transfer_byte(spi_bus_t *bus, uint8_t tx_byte)
{
  (void)bus;
  (void)tx_byte;
  /* TODO: Author Implementation */
  return 0xFF;
}

/**
 * Transfers a contiguous buffer of bytes in full-duplex mode over the SPI bus.
 *
 * Architectural Role:
 * Streams multi-byte payloads. Transmits bytes from tx_buf while storing received
 * bytes into rx_buf.
 *
 * Contract:
 * - Iterates through len bytes, calling the byte serialization primitive for each.
 * - If tx_buf is NULL, transmits dummy bytes (0x00 or 0xFF) to clock in RX data.
 * - If rx_buf is NULL, discards incoming RX data (write-only transfer).
 * - Does NOT manipulate CS; caller must assert/deassert CS around the transfer sequence.
 *
 * @param bus Pointer to the SPI bus structure.
 * @param tx_buf Pointer to source transmit buffer (or NULL for clocking reads).
 * @param rx_buf Pointer to destination receive buffer (or NULL for discard).
 * @param len Number of bytes to transfer.
 * @return 0 on success, or -1 on invalid argument (bus == NULL or len == 0).
 */
int spi_transfer(spi_bus_t *bus, const uint8_t *tx_buf, uint8_t *rx_buf, size_t len)
{
  (void)bus;
  (void)tx_buf;
  (void)rx_buf;
  (void)len;
  /* TODO: Author Implementation */
  return -1;
}

/**
 * Reads the 3-byte JEDEC identification code from an SPI peripheral (Opcode 0x9F).
 *
 * Protocol Framing:
 *   CS LOW -> [Opcode 0x9F] -> [Read Byte 0 (Mfg)] -> [Read Byte 1 (Type)] -> [Read Byte 2 (Cap)]
 * -> CS HIGH
 *
 * Transaction Contract:
 * - Asserts CS, transmits the 0x9F command opcode, clocks in 3 ID bytes into id_buf,
 *   and deasserts CS.
 *
 * @param bus Pointer to the SPI bus structure.
 * @param id_buf Destination buffer (minimum 3 bytes allocated).
 * @param id_len Number of ID bytes to read (typically 3).
 * @return 0 on success, or -1 on invalid argument (bus == NULL, id_buf == NULL, id_len == 0).
 */
int spi_read_device_id(spi_bus_t *bus, uint8_t *id_buf, size_t id_len)
{
  (void)bus;
  (void)id_buf;
  (void)id_len;
  /* TODO: Author Implementation */
  return -1;
}

/**
 * Reads data bytes from the memory array of an SPI peripheral (Opcode 0x03).
 *
 * Protocol Framing:
 *   CS LOW -> [Opcode 0x03] -> [8-bit Address] -> [Read Data 0] -> ... -> [Read Data N-1] -> CS
 * HIGH
 *
 * Transaction Contract:
 * - Asserts CS, transmits Read opcode (0x03) followed by start address.
 * - Clocks in len bytes into data_buf.
 * - Deasserts CS to terminate memory read transaction.
 *
 * @param bus Pointer to the SPI bus structure.
 * @param addr 8-bit memory starting address.
 * @param data_buf Destination buffer for received data.
 * @param len Number of bytes to read.
 * @return 0 on success, or -1 on invalid argument.
 */
int spi_read_memory(spi_bus_t *bus, uint8_t addr, uint8_t *data_buf, size_t len)
{
  (void)bus;
  (void)addr;
  (void)data_buf;
  (void)len;
  /* TODO: Author Implementation */
  return -1;
}

/**
 * Writes data bytes to the memory array of an SPI peripheral (Opcodes 0x06 and 0x02).
 *
 * Protocol Framing:
 *   1. Write Enable: CS LOW -> [Opcode 0x06 (WREN)] -> CS HIGH
 *   2. Page Program: CS LOW -> [Opcode 0x02 (WRITE)] -> [8-bit Address] -> [Data 0] -> ... -> CS
 * HIGH
 *
 * Transaction Contract:
 * - Must issue WREN transaction first to set the Write Enable Latch (WEL bit in status register).
 * - Issues Page Program transaction with starting address and payload bytes.
 * - Deasserts CS to commit the write operation.
 *
 * @param bus Pointer to the SPI bus structure.
 * @param addr 8-bit memory starting address.
 * @param data_buf Pointer to source data buffer to write.
 * @param len Number of bytes to write.
 * @return 0 on success, or -1 on invalid argument.
 */
int spi_write_memory(spi_bus_t *bus, uint8_t addr, const uint8_t *data_buf, size_t len)
{
  (void)bus;
  (void)addr;
  (void)data_buf;
  (void)len;
  /* TODO: Author Implementation */
  return -1;
}

/* ==============================================================================
 * Comprehensive Verification Harness & Test Driver
 * ============================================================================== */

static void test_initialization_and_idle_levels(void)
{
  printf("[TEST 1] Verifying SPI initialization & line idle levels across modes...\n");

  spi_bus_t bus;
  memset(&bus, 0, sizeof(bus));

  /* Test NULL check */
  assert(spi_init(NULL, SPI_MODE_0) == -1);

  /* Mode 0: CPOL=0 (SCLK idle LOW) */
  assert(spi_init(&bus, SPI_MODE_0) == 0);
  assert(bus.cs_n == 1);
  assert(bus.sclk == 0);
  assert(bus.mode == SPI_MODE_0);

  /* Mode 2: CPOL=1 (SCLK idle HIGH) */
  assert(spi_init(&bus, SPI_MODE_2) == 0);
  assert(bus.cs_n == 1);
  assert(bus.sclk == 1);
  assert(bus.mode == SPI_MODE_2);

  /* Mode 3: CPOL=1 (SCLK idle HIGH) */
  assert(spi_init(&bus, SPI_MODE_3) == 0);
  assert(bus.cs_n == 1);
  assert(bus.sclk == 1);
  assert(bus.mode == SPI_MODE_3);

  printf("  -> PASS: Line idle levels verified for Mode 0, Mode 2, and Mode 3.\n");
}

static void test_chip_select_assertion(void)
{
  printf("[TEST 2] Verifying Chip Select assertion & deassertion invariants...\n");

  spi_bus_t bus;
  assert(spi_init(&bus, SPI_MODE_0) == 0);
  mock_spi_slave_init(&bus.slave, SPI_MODE_0);

  assert(bus.cs_n == 1);
  assert(!bus.slave.selected);

  /* Select */
  assert(spi_chip_select(&bus) == 0);
  assert(bus.cs_n == 0);
  assert(bus.slave.selected);

  /* Deselect */
  assert(spi_chip_deselect(&bus) == 0);
  assert(bus.cs_n == 1);
  assert(!bus.slave.selected);

  printf("  -> PASS: Chip Select transition logic and slave tracking verified.\n");
}

static void test_mode0_full_duplex_byte_transfer(void)
{
  printf("[TEST 3] Verifying Mode 0 full-duplex byte serialization...\n");

  spi_bus_t bus;
  assert(spi_init(&bus, SPI_MODE_0) == 0);
  mock_spi_slave_init(&bus.slave, SPI_MODE_0);

  /* Read JEDEC ID command (0x9F) */
  assert(spi_chip_select(&bus) == 0);
  uint8_t dummy_rx = spi_transfer_byte(&bus, SPI_CMD_RDID);
  (void)dummy_rx;

  /* Clock in 3 ID bytes by transmitting dummy bytes */
  uint8_t id0 = spi_transfer_byte(&bus, 0x00);
  uint8_t id1 = spi_transfer_byte(&bus, 0x00);
  uint8_t id2 = spi_transfer_byte(&bus, 0x00);
  assert(spi_chip_deselect(&bus) == 0);

  assert(id0 == 0xEF);
  assert(id1 == 0x40);
  assert(id2 == 0x18);

  printf("  -> PASS: Mode 0 byte transfer verified (JEDEC ID: 0x%02X 0x%02X 0x%02X).\n", id0, id1,
         id2);
}

static void test_high_level_jedec_id_transaction(void)
{
  printf("[TEST 4] Verifying high-level JEDEC ID transaction API...\n");

  spi_bus_t bus;
  assert(spi_init(&bus, SPI_MODE_0) == 0);
  mock_spi_slave_init(&bus.slave, SPI_MODE_0);

  uint8_t id[3] = {0};
  assert(spi_read_device_id(&bus, id, sizeof(id)) == 0);

  assert(id[0] == 0xEF);
  assert(id[1] == 0x40);
  assert(id[2] == 0x18);

  printf("  -> PASS: JEDEC ID API matches expected manufacturer and device IDs.\n");
}

static void test_memory_read_write_transaction(void)
{
  printf("[TEST 5] Verifying multi-byte memory read and write transactions...\n");

  spi_bus_t bus;
  assert(spi_init(&bus, SPI_MODE_0) == 0);
  mock_spi_slave_init(&bus.slave, SPI_MODE_0);

  uint8_t write_payload[8] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
  uint8_t read_payload[8] = {0};

  /* Write payload to address 0x20 */
  assert(spi_write_memory(&bus, 0x20, write_payload, sizeof(write_payload)) == 0);

  /* Read payload back from address 0x20 */
  assert(spi_read_memory(&bus, 0x20, read_payload, sizeof(read_payload)) == 0);

  assert(memcmp(write_payload, read_payload, sizeof(write_payload)) == 0);

  printf("  -> PASS: Memory write & read verified (8 bytes match bit-for-bit).\n");
}

static void test_mode3_operation(void)
{
  printf("[TEST 6] Verifying SPI Mode 3 (CPOL=1, CPHA=1) clocking and data transfer...\n");

  spi_bus_t bus;
  assert(spi_init(&bus, SPI_MODE_3) == 0);
  mock_spi_slave_init(&bus.slave, SPI_MODE_3);

  uint8_t id[3] = {0};
  assert(spi_read_device_id(&bus, id, sizeof(id)) == 0);

  assert(id[0] == 0xEF);
  assert(id[1] == 0x40);
  assert(id[2] == 0x18);

  printf("  -> PASS: SPI Mode 3 operation verified against mock peripheral.\n");
}

int main(void)
{
  printf("==============================================================================\n");
  printf("Running SPI Bit-Banged Bus Controller & Driver Test Suite\n");
  printf("==============================================================================\n");

  /* Probe for scaffold completion */
  spi_bus_t probe_bus;
  memset(&probe_bus, 0, sizeof(probe_bus));
  if(spi_init(&probe_bus, SPI_MODE_0) == -1)
  {
    printf("[Exercise Scaffold] spi_init() returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    return 0;
  }

  test_initialization_and_idle_levels();
  test_chip_select_assertion();
  test_mode0_full_duplex_byte_transfer();
  test_high_level_jedec_id_transaction();
  test_memory_read_write_transaction();
  test_mode3_operation();

  printf("==============================================================================\n");
  printf("All SPI Driver Verification Tests Passed Successfully!\n");
  printf("==============================================================================\n");

  return 0;
}
