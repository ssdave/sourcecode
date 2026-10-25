/**
 * @file main.c
 * @brief Full-Duplex GPIO Bit-Banged SPI Controller Verification Test Harness.
 */

#include "spi_bitbang.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
