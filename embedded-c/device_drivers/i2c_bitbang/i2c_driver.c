/**
 * @file i2c_driver.c
 * @brief Open-Drain GPIO Bit-Banged I2C Bus Controller Primitives & Transactions.
 */

#include "i2c_bitbang.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Initializes the I2C bus controller and releases both bus lines to idle.
 *
 * Bus State at Idle:
 * Both SCL and SDA are open-drain and pulled HIGH by external resistors (logic 1).
 *
 * @param bus Pointer to the I2C bus structure.
 * @return 0 on success, or -1 on invalid argument (bus == NULL).
 */
int i2c_init(i2c_bus_t *bus)
{
  /* TODO: Author Implementation */
  if(bus == NULL)
  {
    return -1;
  }

  bus->clock_cycles = 0;
  i2c_hal_set_scl(bus, 1);
  i2c_hal_set_sda(bus, 1);

  return 0;
}

/**
 * Generates an I2C START (or REPEATED START) condition on the bus.
 *
 * Protocol Timing Invariant:
 * - START condition: SDA transitions HIGH -> LOW while SCL remains HIGH.
 * - For REPEATED START: SCL may start LOW from previous byte; SDA must be
 *   released HIGH while SCL is LOW first, then SCL raised HIGH, then SDA pulled LOW.
 * - SCL is pulled LOW at the end to hold the bus for data transmission.
 *
 * @param bus Pointer to the I2C bus structure.
 * @return 0 on success, or -1 on invalid argument (bus == NULL).
 */
int i2c_start(i2c_bus_t *bus)
{
  /* TODO: Author Implementation */
  if(bus == NULL)
  {
    return -1;
  }

  // AI Review Feedback: Clean START / REPEATED START timing. Pre-releasing SDA while SCL is LOW
  // before raising SCL guarantees SDA falls only when SCL is confirmed HIGH, satisfying the strict
  // I2C START transition invariant.
  i2c_hal_set_sda(bus, 1);
  i2c_hal_delay();
  i2c_hal_set_scl(bus, 1);
  i2c_hal_delay();
  i2c_hal_set_sda(bus, 0);
  i2c_hal_delay();
  i2c_hal_set_scl(bus, 0);
  i2c_hal_delay();

  return 0;
}

/**
 * Generates an I2C STOP condition on the bus.
 *
 * Protocol Timing Invariant:
 * - STOP condition: SDA transitions LOW -> HIGH while SCL remains HIGH.
 * - SCL starts LOW from previous byte; SDA must be pulled LOW while SCL is LOW first,
 *   then SCL raised HIGH, then SDA released HIGH.
 * - Both lines remain released (HIGH) at idle.
 *
 * @param bus Pointer to the I2C bus structure.
 * @return 0 on success, or -1 on invalid argument (bus == NULL).
 */
int i2c_stop(i2c_bus_t *bus)
{
  /* TODO: Author Implementation */
  if(bus == NULL)
  {
    return -1;
  }

  i2c_hal_set_sda(bus, 0);
  i2c_hal_delay();
  i2c_hal_set_scl(bus, 1);
  i2c_hal_delay();
  i2c_hal_set_sda(bus, 1);
  i2c_hal_delay();

  return 0;
}

/**
 * Transmits an 8-bit byte over the I2C bus (MSB first) and samples the 9th ACK/NACK bit.
 *
 * Architectural Role:
 * Low-level byte serialization primitive. Transmits an 8-bit raw payload on the bus
 * and captures the slave's acknowledgement response on the 9th clock pulse.
 *
 * Physical Line Invariants:
 * - Data bits are transferred MSB-first (bit 7 down to bit 0).
 * - SDA must transition only while SCL is LOW, and remain stable while SCL is HIGH.
 * - On the 9th clock pulse, the master releases SDA (floats HIGH) so the slave can
 *   drive the line LOW (ACK) or leave it HIGH (NACK).
 *
 * @param bus Pointer to the I2C bus structure.
 * @param byte 8-bit raw data payload to transmit.
 * @return 0 if ACK (0) was received from slave,
 *         1 if NACK (1) was received from slave,
 *        -1 on invalid argument (bus == NULL).
 */
int i2c_write_byte(i2c_bus_t *bus, uint8_t byte)
{
  /* TODO: Author Implementation */
  if(bus == NULL)
  {
    return -1;
  }

  i2c_hal_set_scl(bus, 0);

  // AI Review Feedback: Clean MSB-first bit serialization. Using a scoped 'for(int iter = 7; iter
  // >= 0; iter--)' loop is slightly more idiomatic in C99+, but the decrementing while-loop is
  // functionally sound and clear.
  int iter = 7;
  while(iter >= 0)
  {
    i2c_hal_set_sda(bus, (byte >> iter) & 1);
    i2c_hal_delay();
    i2c_hal_set_scl(bus, 1);
    i2c_hal_delay();
    i2c_hal_set_scl(bus, 0);
    i2c_hal_delay();

    iter--;
  }

  i2c_hal_set_sda(bus, 1);
  i2c_hal_delay();
  i2c_hal_set_scl(bus, 1);
  i2c_hal_delay();
  uint8_t ack = i2c_hal_get_sda(bus);
  i2c_hal_set_scl(bus, 0);
  i2c_hal_delay();

  return ack;
}

/**
 * Receives an 8-bit byte over the I2C bus (MSB first) and drives the 9th ACK/NACK bit.
 *
 * Architectural Role:
 * Low-level byte reception primitive. Clocks in 8 data bits from the slave while the
 * master releases SDA, and generates the master's ACK or NACK on the 9th clock pulse.
 *
 * Physical Line Invariants:
 * - Master releases SDA (floats HIGH) throughout the 8 data bit clocks so the slave drives data.
 * - Master samples SDA while SCL is HIGH.
 * - On the 9th clock pulse, the master drives SDA according to send_ack (ACK = LOW, NACK = HIGH).
 *
 * @param bus Pointer to the I2C bus structure.
 * @param byte Pointer to destination buffer where received byte is stored.
 * @param send_ack True to drive ACK (0) on 9th clock; False to drive NACK (1).
 * @return 0 on success, or -1 on invalid argument (bus == NULL || byte == NULL).
 */
int i2c_read_byte(i2c_bus_t *bus, uint8_t *byte, bool send_ack)
{
  /* TODO: Author Implementation */
  if((bus == NULL) || (byte == NULL))
  {
    return -1;
  }

  i2c_hal_set_scl(bus, 0);
  i2c_hal_delay();
  i2c_hal_set_sda(bus, 1);
  i2c_hal_delay();

  // AI Review Feedback: Robust bit assembly with explicit byte initialization. SCL rising edge
  // sampling followed by falling edge hold adheres strictly to standard I2C receiver timing
  // invariants.
  int iter = 7;
  *byte = 0;
  while(iter >= 0)
  {
    i2c_hal_set_scl(bus, 1);
    i2c_hal_delay();
    uint8_t data = i2c_hal_get_sda(bus);
    if(data != 0)
    {
      *byte |= (uint8_t)(1 << iter);
    }
    i2c_hal_set_scl(bus, 0);
    i2c_hal_delay();

    iter--;
  }

  /* Drive 9th clock tick ACK (0) or NACK (1) to slave */
  i2c_hal_set_sda(bus, (send_ack ? 0 : 1));
  i2c_hal_delay();
  i2c_hal_set_scl(bus, 1);
  i2c_hal_delay();
  i2c_hal_set_scl(bus, 0);
  i2c_hal_delay();

  /* Release SDA line back to float HIGH */
  i2c_hal_set_sda(bus, 1);
  i2c_hal_delay();

  return 0;
}

/**
 * Performs a complete I2C register write transaction to a slave device.
 *
 * Protocol Framing:
 *   +-------+-----------------------------+-----+--------------+-----+-----------+-----+------+
 *   | START | Device Addr (7b) | Write(0) | ACK | Reg Addr(8b) | ACK | Data (8b) | ACK | STOP |
 *   +-------+-----------------------------+-----+--------------+-----+-----------+-----+------+
 *
 * Transaction Contract:
 * - Generates START, transmits slave address (7 bits) + write bit (0), register address,
 *   and data byte, verifying slave ACK at each stage.
 * - Generates STOP to release the bus upon completion or if any stage is NACKed.
 *
 * @param bus Pointer to the I2C bus structure.
 * @param dev_addr 7-bit slave device address (0x00 - 0x7F).
 * @param reg_addr 8-bit target peripheral register offset.
 * @param data 8-bit data byte to write into target register.
 * @return 0 on success, or -1 on bus/NACK error or invalid argument.
 */
int i2c_write_register(i2c_bus_t *bus, uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
  /* TODO: Author Implementation */
  if(bus == NULL)
  {
    return -1;
  }

  int ret = i2c_start(bus);
  if(ret != 0)
  {
    ret = i2c_stop(bus);
    return -1;
  }

  // AI Review Feedback: Excellent defensive bus cleanup ensuring i2c_stop() is issued on any NACK
  // or transmission error so the bus lines are not left pulled LOW. Note: 'if(ret != 0)' cleanly
  // covers both NACK (1) and error (-1).
  ret = i2c_write_byte(bus, (dev_addr << 1) | 0);
  if((ret == 1) || (ret == -1))
  {
    ret = i2c_stop(bus);
    return -1;
  }

  ret = i2c_write_byte(bus, reg_addr);
  if((ret == 1) || (ret == -1))
  {
    ret = i2c_stop(bus);
    return -1;
  }

  ret = i2c_write_byte(bus, data);
  if((ret == 1) || (ret == -1))
  {
    ret = i2c_stop(bus);
    return -1;
  }

  ret = i2c_stop(bus);
  if(ret != 0)
  {
    return -1;
  }

  return 0;
}

/**
 * Performs a complete I2C register read transaction from a slave device.
 *
 * Protocol Framing (Combined Write-Address + Read-Data Frame):
 *   +-------+-----------------------------+-----+--------------+-----+
 *   | START | Device Addr (7b) | Write(0) | ACK | Reg Addr(8b) | ACK |
 *   +-------+-----------------------------+-----+--------------+-----+
 *   +----------------+----------------------------+-----+-----------+------+------+
 *   | REPEATED START | Device Addr (7b) | Read(1) | ACK | Data (8b) | NACK | STOP |
 *   +----------------+----------------------------+-----+-----------+------+------+
 *
 * Transaction Contract:
 * - Writes the target register address using a write header.
 * - Issues a REPEATED START with a read header to reverse bus direction.
 * - Reads data byte, asserting NACK on the final byte to signal read completion, then STOP.
 * - If any addressing stage is NACKed, terminates with STOP and returns error.
 *
 * @param bus Pointer to the I2C bus structure.
 * @param dev_addr 7-bit slave device address (0x00 - 0x7F).
 * @param reg_addr 8-bit target peripheral register offset.
 * @param data Pointer to destination buffer where received byte is stored.
 * @return 0 on success, or -1 on bus/NACK error or invalid argument.
 */
int i2c_read_register(i2c_bus_t *bus, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data)
{
  /* TODO: Author Implementation */
  if((bus == NULL) || (data == NULL))
  {
    return -1;
  }

  int ret = i2c_start(bus);
  if(ret != 0)
  {
    ret = i2c_stop(bus);
    return -1;
  }

  // AI Review Feedback: Excellent defensive bus cleanup ensuring i2c_stop() is issued on any NACK
  // or transmission error so the bus lines are not left pulled LOW. Note: 'if(ret != 0)' cleanly
  // covers both NACK (1) and error (-1).
  ret = i2c_write_byte(bus, (dev_addr << 1) | 0);
  if((ret == 1) || (ret == -1))
  {
    ret = i2c_stop(bus);
    return -1;
  }

  ret = i2c_write_byte(bus, reg_addr);
  if((ret == 1) || (ret == -1))
  {
    ret = i2c_stop(bus);
    return -1;
  }

  // AI Review Feedback: Clean repeated-START sequencing reverses bus direction to READ without
  // yielding bus ownership.
  ret = i2c_start(bus);
  if(ret != 0)
  {
    ret = i2c_stop(bus);
    return -1;
  }

  ret = i2c_write_byte(bus, (dev_addr << 1) | 1);
  if((ret == 1) || (ret == -1))
  {
    ret = i2c_stop(bus);
    return -1;
  }

  uint8_t byte = 0;
  ret = i2c_read_byte(bus, &byte, false);
  if(ret != 0)
  {
    ret = i2c_stop(bus);
    return -1;
  }
  *data = byte;

  ret = i2c_stop(bus);
  if(ret != 0)
  {
    return -1;
  }

  return 0;
}
