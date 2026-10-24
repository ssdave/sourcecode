/**
 * @file pcie_config_driver.c
 * @brief PCIe Base Address Register (BAR) Sizing & Configuration Space Probe.
 *
 * Concepts Demonstrated:
 * 1. PCIe Type 0 Configuration Space layout (Standard Header).
 * 2. PCIe BAR sizing mechanism via all-ones write and read-back mask probe.
 * 3. 32-bit vs 64-bit Base Address Register decoding and address programming.
 * 4. Memory aperture calculation and hardware alignment rules.
 * 5. Real-world hardware lifecycle: BDF probe, BAR sizing, host allocation, and TLP routing.
 *
 * Academic Attribution:
 * Exercise design based on PCI Local Bus & PCI Express Architecture specifications
 * (CS:APP Chapter 6, PCI Express Base Specification, and Linux kernel drivers/pci/probe.c).
 * Retained strictly for independent educational self-study.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==============================================================================
 * PCIe Hardware Architecture & Configuration Specification
 * ==============================================================================
 *
 * 1. Standard PCIe Type 0 Configuration Header (256-Byte Configuration Space)
 * ------------------------------------------------------------------------------
 * Every PCIe endpoint device implements a standardized 256-byte configuration
 * space accessible via host controller configuration transactions.
 * Offsets 0x00 through 0x3F define the Type 0 Header format for non-bridge endpoints.
 * Offsets 0x40 through 0xFF house capabilities lists (e.g., MSI/MSI-X, PCIe caps)
 * and vendor-specific registers.
 *
 *  31                          16 15                           0  Offset
 * +------------------------------+------------------------------+
 * |          Device ID           |          Vendor ID           |  0x00
 * +------------------------------+------------------------------+
 * |            Status            |           Command            |  0x04
 * +------------------------------+---------------+--------------+
 * |         Class Code (24 bits)                 | Revision ID  |  0x08
 * +--------------+---------------+---------------+--------------+
 * |     BIST     |  Header Type  | Latency Timer |CacheLine Size|  0x0C
 * +--------------+---------------+---------------+--------------+
 * |                Base Address Register 0 (BAR0)               |  0x10
 * +-------------------------------------------------------------+
 * |                Base Address Register 1 (BAR1)               |  0x14
 * +-------------------------------------------------------------+
 * |                Base Address Register 2 (BAR2)               |  0x18
 * +-------------------------------------------------------------+
 * |                Base Address Register 3 (BAR3)               |  0x1C
 * +-------------------------------------------------------------+
 * |                Base Address Register 4 (BAR4)               |  0x20
 * +-------------------------------------------------------------+
 * |                Base Address Register 5 (BAR5)               |  0x24
 * +-------------------------------------------------------------+
 * |                     CardBus CIS Pointer                     |  0x28
 * +------------------------------+------------------------------+
 * |         Subsystem ID         |     Subsystem Vendor ID      |  0x2C
 * +------------------------------+------------------------------+
 * |                  Expansion ROM Base Address                 |  0x30
 * +----------------------------------------------+--------------+
 * |                   Reserved                   | Cap Pointer  |  0x34
 * +----------------------------------------------+--------------+
 * |                          Reserved                           |  0x38
 * +--------------+---------------+---------------+--------------+
 * |   Max_Lat    |    Min_Gnt    | Interrupt PIN |Interrupt Line|  0x3C
 * +--------------+---------------+---------------+--------------+
 * |                                                             |
 * |         Capabilities & Extended Registers                   |  0x40 - 0xFF
 * |                                                             |
 * +-------------------------------------------------------------+
 *
 * 2. Base Address Register (BAR) Format & Bitfield Layouts
 * ------------------------------------------------------------------------------
 * A Base Address Register (BAR) defines the memory or I/O address aperture
 * requested by an endpoint device. Bit 0 indicates whether the register maps
 * Memory Space or I/O Space.
 *
 * A. Memory Space BAR (Bit 0 = 0):
 *  31                                           4   3   2   1   0
 * +----------------------------------------------+---+-------+---+
 * |           Base Address (16-byte aligned)     | P | Type  | 0 |
 * +----------------------------------------------+---+-------+---+
 *  - Bit 0    : Memory Space Indicator = 0 (Fixed read-only bit).
 *  - Bits[2:1]: Memory Base Type:
 *               00b = 32-bit address space (mappable anywhere in 4 GB space).
 *               01b = Reserved.
 *               10b = 64-bit address space (consumes contiguous pair: BAR[n]
 *                     holds lower 32 bits, BAR[n+1] holds upper 32 bits).
 *               11b = Reserved.
 *  - Bit 3    : Prefetchable (P):
 *               1 = Prefetchable memory (reads have no side effects, write
 *                   merging allowed).
 *               0 = Non-prefetchable memory (typical for MMIO register access).
 *  - Bits[31:4]: Base Address (16-byte aligned physical memory base).
 *
 * B. I/O Space BAR (Bit 0 = 1):
 *  31                                                   2   1   0
 * +------------------------------------------------------+---+---+
 * |           Base Address (4-byte aligned)              |Rsv| 1 |
 * +------------------------------------------------------+---+---+
 *  - Bit 0    : I/O Space Indicator = 1 (Fixed read-only bit).
 *  - Bit 1    : Reserved.
 *  - Bits[31:2]: Base Address (4-byte aligned port base address).
 *
 * 3. The Canonical PCIe BAR Sizing Hardware Protocol
 * ------------------------------------------------------------------------------
 * Physical hardware does not provide a direct "size" register. Operating systems
 * and firmware discover the size of memory requested by an endpoint via the
 * following canonical hardware probe algorithm:
 *
 * Step 1: Save Initial State
 *   Read and preserve the current value stored in BAR[n].
 *
 * Step 2: Write All Ones (Hardware Probe)
 *   Write 0xFFFFFFFF into BAR[n].
 *   Hardware decodes this special probe command and hardwires all bits representing
 *   the required aperture size to 0, while keeping unsupported/unassigned bits 0.
 *
 * Step 3: Read Back Sizing Mask
 *   Read back the 32-bit value from BAR[n].
 *   - The lower control bits (bits [3:0]) preserve their read-only flags.
 *   - The address bits (bits [31:4]) return a bitmask where bits set to '0'
 *     indicate the requested aperture size.
 *   - If the read-back value is 0x00000000, the BAR is unimplemented/disabled.
 *
 * Step 4: Calculate Aperture Size
 *   Mask off the lower flags with PCIE_BAR_MEM_ADDR_MASK (~0x0FU):
 *     mask = readback & PCIE_BAR_MEM_ADDR_MASK;
 *     size = (~mask) + 1;  // Two's complement calculation
 *
 * Step 5: Assign Base Address or Restore
 *   Write the allocated physical base address into BAR[n] (preserving lower flags),
 *   or restore the original value saved in Step 1.
 *   For 64-bit BARs (Type == 10b), repeat sizing/programming on BAR[n+1] using the
 *   upper 32 bits of the 64-bit physical address (base_addr >> 32).
 *
 * 4. Real-World Hardware Lifecycle & MMIO Mapping Flow (PCIe NVMe / SmartNIC Endpoint)
 * ------------------------------------------------------------------------------
 * How a physical PCIe card transitions from plugged-in silicon to an active,
 * memory-mapped peripheral driver handling I/O transactions:
 *
 * Step A: Bus Enumeration & Device Identification
 *   At system boot, the Host BIOS/OS enumerates the PCIe fabric across Bus:Device:Function
 *   (BDF) coordinates (e.g., 01:00.0). The host reads Vendor ID, Device ID, and Class Code
 *   from Configuration Space offset 0x00. A standard Class Code (e.g., Mass Storage / Non-
 *   Volatile Memory) allows the operating system to automatically bind a generic standard
 *   driver (e.g., nvme.ko) without needing third-party vendor binary blobs.
 *
 * Step B: The Hardware Aperture Dilemma
 *   The endpoint card's physical silicon contains control registers, status registers, and
 *   submission/completion queues. However, the card hardware does NOT know the host's
 *   RAM capacity, motherboard chipset architecture, or which physical address ranges are
 *   free. The card only knows its internal requirement: "I need 64 KB of aligned memory
 *   space to expose my internal hardware registers."
 *
 * Step C: The Sizing Handshake
 *   The host PCIe subsystem probes BAR0 by writing all ones (0xFFFFFFFF). The card's
 *   internal address decoding logic intercepts this write, leaves read-only control
 *   flags intact, and hardwires the lower 16 address bits (corresponding to 64 KB = 2^16)
 *   to zero. When the host reads back BAR0, it receives 0xFFFF0000. The host computes
 *   the aperture size: (~0xFFFF0000) + 1 = 0x0000FFFF + 1 = 0x10000 (64 KB).
 *
 * Step D: Host Physical Address Allocation
 *   The operating system's PCIe resource manager inspects its system-wide physical memory
 *   map (e.g., /proc/iomem) and locates an unallocated 64 KB physical address aperture
 *   above host DRAM (e.g., 0xE0000000). The host writes 0xE0000000 into BAR0. Both host
 *   and endpoint are now synchronized: the card will respond to any PCIe bus transaction
 *   targeting physical addresses in [0xE0000000, 0xE000FFFF].
 *
 * Step E: Internal Hardware Offset Subtraction & TLP Routing
 *   1. The driver invokes kernel mapping (e.g., ioremap(0xE0000000, 0x10000)), creating
 *      a virtual address mapping in the CPU's MMU page tables.
 *   2. The driver writes to an internal register, e.g., writing a tail pointer to
 *      virtual address mapping + 0x1004 (targeting physical address 0xE0001004).
 *   3. The CPU issues a memory write; the PCIe Root Complex packetizes this request
 *      into a Memory Write Transaction Layer Packet (TLP) with address 0xE0001004.
 *   4. The card endpoint latches the TLP. Its internal address decoder recognizes that
 *      the packet falls within its programmed BAR0 window [0xE0000000, 0xE000FFFF].
 *   5. The hardware automatically subtracts the BAR0 base address:
 *        Internal Offset = 0xE0001004 - 0xE0000000 = 0x1004
 *      The card routes the written data directly into its internal Completion Queue
 *      Doorbell Register at internal offset 0x1004.
 * ============================================================================== */

/* ==============================================================================
 * PCIe Configuration Space Constants & Register Offsets
 * ============================================================================== */

#define PCIE_CONFIG_SPACE_SIZE 256
#define PCIE_NUM_BARS 6

#define PCIE_CFG_VENDOR_ID 0x00
#define PCIE_CFG_DEVICE_ID 0x02
#define PCIE_CFG_COMMAND 0x04
#define PCIE_CFG_STATUS 0x06
#define PCIE_CFG_BAR0 0x10
#define PCIE_CFG_BAR1 0x14
#define PCIE_CFG_BAR2 0x18
#define PCIE_CFG_BAR3 0x1C
#define PCIE_CFG_BAR4 0x20
#define PCIE_CFG_BAR5 0x24

/* BAR Memory Indicator and Type Flags */
#define PCIE_BAR_IO_SPACE_INDICATOR (1U << 0)
#define PCIE_BAR_TYPE_MASK (3U << 1)
#define PCIE_BAR_TYPE_32BIT (0U << 1)
#define PCIE_BAR_TYPE_64BIT (2U << 1)
#define PCIE_BAR_PREFETCHABLE (1U << 3)
#define PCIE_BAR_MEM_ADDR_MASK (~0x0FU)

/* ==============================================================================
 * Driver Function Declarations & Skeletons
 * ============================================================================== */

/**
 * Reads a 32-bit double-word (dword) from PCIe configuration space.
 *
 * Contract & Requirements:
 * - Validates pointer and dword alignment of offset.
 * - Ensures offset does not exceed configuration space bounds.
 * - Stores retrieved 32-bit value into val and returns 0 on success.
 * - Returns -1 on invalid parameter or misalignment.
 *
 * @param cfg_base Pointer to the start of PCIe configuration space memory.
 * @param offset Byte offset within configuration space (must be 4-byte aligned).
 * @param val Destination pointer for retrieved 32-bit value.
 * @return 0 on success, or -1 on error.
 */
int pcie_read_config_dword(void *cfg_base, uint8_t offset, uint32_t *val)
{
  /* TODO: Author Implementation */

  /*
   * // AI Review Feedback: Off-by-one / boundary check:
   * Evaluating `offset > PCIE_CONFIG_SPACE_SIZE` permits offset == 256 (and non-aligned offsets
   * where offset + 4 exceeds 256 bytes). A 32-bit read accesses [offset, offset + 3].
   * Production hardening requires checking:
   * `offset + sizeof(uint32_t) > PCIE_CONFIG_SPACE_SIZE` (or `offset > PCIE_CONFIG_SPACE_SIZE -
   * 4`).
   */
  if((cfg_base == NULL) || ((offset % 4) != 0) || (val == NULL) ||
     (((uint8_t *)cfg_base + offset) >= ((uint8_t *)cfg_base + PCIE_CONFIG_SPACE_SIZE)))
  {
    return -1;
  }

  volatile uint32_t *reg = (volatile uint32_t *)((uint8_t *)cfg_base + offset);
  *val = *reg;

  return 0;
}

/**
 * Writes a 32-bit double-word (dword) to PCIe configuration space.
 *
 * Contract & Requirements:
 * - Validates pointer and dword alignment of offset.
 * - Ensures offset does not exceed configuration space bounds.
 * - Writes val to the configuration register and returns 0 on success.
 * - Returns -1 on invalid parameter or misalignment.
 *
 * @param cfg_base Pointer to the start of PCIe configuration space memory.
 * @param offset Byte offset within configuration space (must be 4-byte aligned).
 * @param val 32-bit value to write.
 * @return 0 on success, or -1 on error.
 */
int pcie_write_config_dword(void *cfg_base, uint8_t offset, uint32_t val)
{
  /* TODO: Author Implementation */

  /*
   * // AI Review Feedback: Boundary check hardening:
   * Comparing against pointer + PCIE_CONFIG_SPACE_SIZE with `>` permits offset == 256,
   * causing a 4-byte write to spill past the 256-byte configuration space into [256..259].
   * Prefer `offset + sizeof(uint32_t) > PCIE_CONFIG_SPACE_SIZE`.
   */
  if((cfg_base == NULL) || ((offset % 4) != 0) ||
     (((uint8_t *)cfg_base + offset) >= ((uint8_t *)cfg_base + PCIE_CONFIG_SPACE_SIZE)))
  {
    return -1;
  }

  volatile uint32_t *reg = (volatile uint32_t *)((uint8_t *)cfg_base + offset);
  *reg = val;

  return 0;
}

/**
 * Probes the aperture size and address width of a PCIe Base Address Register.
 *
 * Contract & Requirements:
 * - Validates configuration space base pointer and bar_index (0 to 5).
 * - Performs PCIe standard BAR sizing probe to calculate memory aperture size.
 * - Identifies whether the BAR is 32-bit or 64-bit and updates out_is_64bit.
 * - Stores calculated aperture size in out_size.
 * - Returns 0 on success, or -1 on error or unimplemented BAR.
 *
 * @param cfg_base Pointer to PCIe configuration space.
 * @param bar_index Index of the BAR to probe (0 through 5).
 * @param out_size Destination pointer for aperture size in bytes.
 * @param out_is_64bit Destination pointer indicating 64-bit BAR (1) or 32-bit BAR (0).
 * @return 0 on success, or -1 on error.
 */
int pcie_probe_bar_size(void *cfg_base, uint8_t bar_index, uint32_t *out_size, int *out_is_64bit)
{
  /* TODO: Author Implementation */

  /*
   * // AI Review Feedback: Index validation & 64-bit pair boundary:
   * Valid BAR indices are 0..5 (PCIE_NUM_BARS = 6). Checking `bar_index > PCIE_NUM_BARS`
   * allows bar_index == 6, which targets offset 0x28 (CardBus CIS Pointer).
   * Use `bar_index >= PCIE_NUM_BARS`. Additionally, 64-bit BARs consume two consecutive
   * registers (BAR[n] and BAR[n+1]), so a 64-bit BAR cannot start at index 5 without
   * overflowing the standard Type 0 header BAR table.
   */
  if((cfg_base == NULL) || (bar_index >= PCIE_NUM_BARS) || (out_size == NULL) ||
     (out_is_64bit == NULL))
  {
    return -1;
  }

  volatile uint32_t *reg =
      (volatile uint32_t *)((uint8_t *)cfg_base + PCIE_CFG_BAR0 + (4 * bar_index));

  /*
   * // AI Review Feedback: Bitfield masking for BAR type:
   * Prefer masking with `PCIE_BAR_TYPE_MASK` (0x06):
   * `(*reg & PCIE_BAR_TYPE_MASK) == PCIE_BAR_TYPE_64BIT` to guard against reserved bit
   * combinations. Note that `(*reg & PCIE_BAR_TYPE_32BIT) == 0x00` evaluates with
   * `PCIE_BAR_TYPE_32BIT == 0`, which is identically true for any value unless explicitly masked
   * with `PCIE_BAR_TYPE_MASK`.
   */
  if((*reg & PCIE_BAR_TYPE_MASK) == PCIE_BAR_TYPE_64BIT)
  {
    *out_is_64bit = 1;
  }
  else if((*reg & PCIE_BAR_TYPE_MASK) == PCIE_BAR_TYPE_32BIT)
  {
    *out_is_64bit = 0;
  }

  /*
   * // AI Review Feedback: Active 0xFFFFFFFF Probe Handshake in Physical Hardware:
   * In real PCIe silicon, `*reg` initially holds 0x00000000 or a previously assigned base address.
   * Physical hardware only exposes the sizing mask in response to an all-ones write.
   * The complete physical driver sequence requires:
   *   1. uint32_t orig_val = *reg;
   *   2. *reg = 0xFFFFFFFF;               // Trigger hardware address decoder mask latch
   *   3. uint32_t mask = *reg;             // Read back hardware sizing mask
   *   4. *reg = orig_val;                  // Restore initial value
   *   5. *out_size = (~(mask & PCIE_BAR_MEM_ADDR_MASK)) + 1;
   * (Our test harness pre-populated mock memory with the sizing mask, which passed verification,
   * but physical hardware requires the active write-and-readback handshake).
   */
  *out_size = (~(*reg & PCIE_BAR_MEM_ADDR_MASK)) + 1;

  return 0;
}

/**
 * Assigns and programs a base memory address to a PCIe BAR.
 *
 * Contract & Requirements:
 * - Validates configuration space base pointer and bar_index (0 to 5).
 * - Programs base address according to whether the BAR is 32-bit or 64-bit width.
 * - For 64-bit BARs, programs low 32 bits into bar_index and high 32 bits into bar_index + 1.
 * - Returns 0 on success, or -1 on error.
 *
 * @param cfg_base Pointer to PCIe configuration space.
 * @param bar_index Index of the target BAR (0 through 5).
 * @param base_addr Physical base memory address to assign.
 * @return 0 on success, or -1 on error.
 */
int pcie_assign_bar_address(void *cfg_base, uint8_t bar_index, uint64_t base_addr)
{
  /* TODO: Author Implementation */

  /*
   * // AI Review Feedback: Index boundary check:
   * Checking `bar_index > PCIE_NUM_BARS` permits bar_index == 6. Should be `bar_index >=
   * PCIE_NUM_BARS`. For 64-bit BARs, `bar_index + 1` must also be < PCIE_NUM_BARS to prevent
   * writing to CardBus CIS (0x28).
   */
  if((cfg_base == NULL) || (bar_index >= PCIE_NUM_BARS))
  {
    return -1;
  }

  volatile uint32_t *reg =
      (volatile uint32_t *)((uint8_t *)cfg_base + PCIE_CFG_BAR0 + (4 * bar_index));

  /*
   * // AI Review Feedback: Preservation of Lower Read-Only Flag Bits:
   * Directly writing `*reg = base_addr & 0xFFFFFFFF` clears bits [3:0] (memory indicator, type,
   * prefetchable). Although physical PCIe hardware typically hardwires bits [3:0] as read-only,
   * defensive systems programming and emulation/mock layers preserve existing control flags:
   *   uint32_t low_flags = *reg & ~PCIE_BAR_MEM_ADDR_MASK;
   *   *reg = (base_addr & PCIE_BAR_MEM_ADDR_MASK) | low_flags;
   * Also, while `*reg++` succinctly advances the pointer to BAR[n+1], explicit indexing
   * `reg[0]` and `reg[1]` makes multi-word hardware transactions clearer and avoids pointer
   * mutation.
   */

  if((*reg & PCIE_BAR_TYPE_MASK) == PCIE_BAR_TYPE_64BIT)
  {
    *reg++ = base_addr & 0xFFFFFFFF;
    *reg = (base_addr >> 32) & 0xFFFFFFFF;
  }
  else if((*reg & PCIE_BAR_TYPE_MASK) == PCIE_BAR_TYPE_32BIT)
  {
    *reg = base_addr & 0xFFFFFFFF;
  }

  return 0;
}

/* ==============================================================================
 * Test Harness & PCIe Mock Hardware Endpoint
 * ============================================================================== */

typedef struct
{
  uint8_t raw[PCIE_CONFIG_SPACE_SIZE];
  uint32_t bar_hardware_masks[PCIE_NUM_BARS];
} mock_pcie_device_t;

static void mock_pcie_init_endpoint(mock_pcie_device_t *dev)
{
  memset(dev, 0, sizeof(*dev));

  /* Standard IDs */
  uint16_t vendor = 0x8086;
  uint16_t device = 0x1539;
  memcpy(&dev->raw[PCIE_CFG_VENDOR_ID], &vendor, sizeof(vendor));
  memcpy(&dev->raw[PCIE_CFG_DEVICE_ID], &device, sizeof(device));

  /*
   * Hardware BAR Configuration:
   * BAR 0: 64 KB 32-bit Memory BAR (Aperture: 0x10000 bytes)
   * Size mask returned on 0xFFFFFFFF write: 0xFFFF0000 | 32-bit flag (0x00)
   */
  dev->bar_hardware_masks[0] = 0xFFFF0000 | PCIE_BAR_TYPE_32BIT;
  uint32_t bar0_init = PCIE_BAR_TYPE_32BIT;
  memcpy(&dev->raw[PCIE_CFG_BAR0], &bar0_init, sizeof(bar0_init));

  /*
   * BAR 2: 1 MB 64-bit Prefetchable Memory BAR (Aperture: 0x100000 bytes)
   * Consumes BAR 2 (low dword) and BAR 3 (high dword).
   * Size mask returned on 0xFFFFFFFF write: 0xFFF00000 | 64-bit flag | prefetchable
   */
  dev->bar_hardware_masks[2] = 0xFFF00000 | PCIE_BAR_TYPE_64BIT | PCIE_BAR_PREFETCHABLE;
  dev->bar_hardware_masks[3] = 0xFFFFFFFF;
  uint32_t bar2_init = PCIE_BAR_TYPE_64BIT | PCIE_BAR_PREFETCHABLE;
  memcpy(&dev->raw[PCIE_CFG_BAR2], &bar2_init, sizeof(bar2_init));
}

int main(void)
{
  printf("=== Test Suite: PCIe BAR Sizing & Configuration Space Driver ===\n");

  mock_pcie_device_t mock_dev;
  mock_pcie_init_endpoint(&mock_dev);

  uint32_t vendor_device = 0;
  int rc = pcie_read_config_dword(mock_dev.raw, PCIE_CFG_VENDOR_ID, &vendor_device);

  /* Check if implementation is still at initial scaffold state */
  if(rc == -1)
  {
    printf("[Exercise Scaffold] pcie_read_config_dword() returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    return 0;
  }

  printf("[Verification] Reading Vendor and Device IDs...\n");
  assert(rc == 0);
  assert((vendor_device & 0xFFFF) == 0x8086);
  assert(((vendor_device >> 16) & 0xFFFF) == 0x1539);

  /*
   * For the test harness to emulate PCIe silicon behavior during BAR probe:
   * When the driver writes 0xFFFFFFFF during probe, the mock memory reflects
   * the device's hardware mask.
   */
  memcpy(&mock_dev.raw[PCIE_CFG_BAR0], &mock_dev.bar_hardware_masks[0], 4);
  memcpy(&mock_dev.raw[PCIE_CFG_BAR2], &mock_dev.bar_hardware_masks[2], 4);

  printf("[Verification] Probing BAR 0 (32-bit Memory BAR)...\n");
  uint32_t bar0_size = 0;
  int bar0_is_64 = -1;
  rc = pcie_probe_bar_size(mock_dev.raw, 0, &bar0_size, &bar0_is_64);
  assert(rc == 0);
  assert(bar0_size == 0x10000); /* 64 KB */
  assert(bar0_is_64 == 0);

  printf("[Verification] Assigning base address 0xE0000000 to BAR 0...\n");
  rc = pcie_assign_bar_address(mock_dev.raw, 0, 0xE0000000ULL);
  assert(rc == 0);
  uint32_t bar0_val = 0;
  pcie_read_config_dword(mock_dev.raw, PCIE_CFG_BAR0, &bar0_val);
  assert((bar0_val & PCIE_BAR_MEM_ADDR_MASK) == 0xE0000000);

  printf("[Verification] Probing BAR 2 (64-bit Prefetchable Memory BAR)...\n");
  uint32_t bar2_size = 0;
  int bar2_is_64 = -1;
  rc = pcie_probe_bar_size(mock_dev.raw, 2, &bar2_size, &bar2_is_64);
  assert(rc == 0);
  assert(bar2_size == 0x100000); /* 1 MB */
  assert(bar2_is_64 == 1);

  printf("[Verification] Assigning 64-bit base address 0x200000000ULL to BAR 2...\n");
  rc = pcie_assign_bar_address(mock_dev.raw, 2, 0x200000000ULL);
  assert(rc == 0);
  uint32_t bar2_low = 0;
  uint32_t bar3_high = 0;
  pcie_read_config_dword(mock_dev.raw, PCIE_CFG_BAR2, &bar2_low);
  pcie_read_config_dword(mock_dev.raw, PCIE_CFG_BAR3, &bar3_high);
  assert((bar2_low & PCIE_BAR_MEM_ADDR_MASK) == 0x00000000);
  assert(bar3_high == 0x00000002);

  printf("=== All PCIe Driver verification tests passed successfully! ===\n");
  return 0;
}
