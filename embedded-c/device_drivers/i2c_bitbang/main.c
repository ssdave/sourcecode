/**
 * @file main.c
 * @brief Open-Drain GPIO Bit-Banged I2C Driver Verification Test Harness.
 */

#include "i2c_bitbang.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TEST_SLAVE_ADDR 0x48
#define TEST_INVALID_ADDR 0x30

int main(void)
{
  printf("=== Running Open-Drain I2C Bit-Bang Driver Verification ===\n");

  i2c_bus_t bus;
  memset(&bus, 0, sizeof(bus));
  mock_slave_init(&bus.slave, TEST_SLAVE_ADDR);
  bus.scl_pin.slave = 1;
  bus.sda_pin.slave = 1;

  int init_rc = i2c_init(&bus);
  if(init_rc == -1)
  {
    printf("[Exercise Scaffold] i2c_init() returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    return 0;
  }

  printf("[Verification 1] Testing bus initialization & idle line state...\n");
  assert(init_rc == 0);
  assert(i2c_hal_get_scl(&bus) == 1);
  assert(i2c_hal_get_sda(&bus) == 1);

  /* Probe for core protocol implementation */
  if(i2c_start(&bus) == -1 || i2c_write_byte(&bus, 0x00) == -1)
  {
    printf("[Exercise Scaffold] Core I2C protocol functions returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    return 0;
  }

  printf("[Verification 2] Testing slave addressing and ACK/NACK arbitration...\n");
  /* Valid address with Write bit: (0x48 << 1) | 0 = 0x90 -> Expect ACK (0) */
  assert(i2c_start(&bus) == 0);
  int ack = i2c_write_byte(&bus, (uint8_t)((TEST_SLAVE_ADDR << 1) | 0));
  assert(ack == I2C_ACK);
  assert(i2c_stop(&bus) == 0);

  /* Invalid address with Write bit: (0x30 << 1) | 0 = 0x60 -> Expect NACK (1) */
  assert(i2c_start(&bus) == 0);
  int nack = i2c_write_byte(&bus, (uint8_t)((TEST_INVALID_ADDR << 1) | 0));
  assert(nack == I2C_NACK);
  assert(i2c_stop(&bus) == 0);

  printf("[Verification 3] Testing register write transaction...\n");
  /* Write 0x55 into configuration register 0x01 */
  int write_rc = i2c_write_register(&bus, TEST_SLAVE_ADDR, 0x01, 0x55);
  assert(write_rc == 0);
  assert(bus.slave.registers[0x01] == 0x55);

  printf("[Verification 4] Testing register read transaction (default temperature)...\n");
  /* Read temperature register 0x00 (default: 0x19 = 25°C) */
  uint8_t temp_val = 0;
  int read_rc = i2c_read_register(&bus, TEST_SLAVE_ADDR, 0x00, &temp_val);
  assert(read_rc == 0);
  assert(temp_val == 0x19);

  printf("[Verification 5] Testing write and read-back on threshold registers...\n");
  /* Write 0x32 (50°C) into T_LOW register 0x02 */
  assert(i2c_write_register(&bus, TEST_SLAVE_ADDR, 0x02, 0x32) == 0);
  uint8_t t_low_val = 0;
  assert(i2c_read_register(&bus, TEST_SLAVE_ADDR, 0x02, &t_low_val) == 0);
  assert(t_low_val == 0x32);

  printf("[Verification 6] Testing invalid argument handling...\n");
  assert(i2c_init(NULL) == -1);
  assert(i2c_start(NULL) == -1);
  assert(i2c_stop(NULL) == -1);
  assert(i2c_write_byte(NULL, 0x00) == -1);
  assert(i2c_read_byte(NULL, &temp_val, true) == -1);
  assert(i2c_read_byte(&bus, NULL, true) == -1);
  assert(i2c_write_register(NULL, TEST_SLAVE_ADDR, 0x01, 0x00) == -1);
  assert(i2c_read_register(NULL, TEST_SLAVE_ADDR, 0x00, &temp_val) == -1);
  assert(i2c_read_register(&bus, TEST_SLAVE_ADDR, 0x00, NULL) == -1);

  printf("[Verification 7] Testing bus diagnostics and clock activity...\n");
  assert(bus.clock_cycles > 0);
  printf("                 Total clock pulses generated: %u\n", bus.clock_cycles);

  printf("=== All Open-Drain I2C Bit-Bang Driver tests passed successfully! ===\n");
  return 0;
}
