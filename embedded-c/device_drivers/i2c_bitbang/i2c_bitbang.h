/**
 * @file i2c_bitbang.h
 * @brief Open-Drain GPIO Bit-Banged I2C Bus Controller & Peripheral Interface.
 *
 * Concepts Demonstrated:
 * 1. Open-drain GPIO modeling with external pull-up resistors for SCL and SDA.
 * 2. I2C START and STOP framing condition generation on open-drain lines.
 * 3. 8-bit MSB-first byte serialization and synchronous clock pulse generation.
 * 4. 9th clock tick ACK/NACK protocol arbitration across master and slave.
 * 5. Multi-byte transaction framing for device register addressing and sensor reads.
 *
 * Academic Attribution:
 * Exercise design based on computer architecture and embedded systems peripheral protocols
 * (CS:APP Chapter 6, OSTEP Chapter 36, and NXP/Philips I2C Bus Specification UM10204).
 * Retained strictly for independent educational self-study.
 */

#ifndef I2C_BITBANG_H
#define I2C_BITBANG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define I2C_ACK 0
#define I2C_NACK 1

/**
 * Open-Drain GPIO Pin State.
 * Both master and slave can pull the line LOW (0).
 * The line only floats HIGH (1) if BOTH master and slave release it.
 */
typedef struct
{
  uint8_t master; /* 0 = driving LOW, 1 = released (pull-up HIGH) */
  uint8_t slave;  /* 0 = driving LOW, 1 = released (pull-up HIGH) */
} i2c_pin_t;

/**
 * Mock I2C Slave Protocol States (e.g., LM75/TMP102 Temperature Sensor)
 */
typedef enum
{
  SLAVE_STATE_IDLE = 0,
  SLAVE_STATE_ADDR,
  SLAVE_STATE_ADDR_ACK_WRITE,
  SLAVE_STATE_ADDR_ACK_READ,
  SLAVE_STATE_REG_PTR,
  SLAVE_STATE_REG_ACK,
  SLAVE_STATE_WRITE_DATA,
  SLAVE_STATE_WRITE_ACK,
  SLAVE_STATE_READ_DATA,
  SLAVE_STATE_READ_ACK
} mock_slave_state_t;

/**
 * Mock I2C Slave Device Model
 */
typedef struct
{
  uint8_t address;          /* 7-bit device address (e.g. 0x48) */
  mock_slave_state_t state; /* Internal protocol state */
  uint8_t selected_reg;     /* Current register pointer */
  uint8_t registers[8];     /* Device register file */
  uint8_t shift_reg;        /* Shift register for RX */
  uint8_t bit_index;        /* Bit index counter */
  uint8_t tx_byte;          /* Byte currently being transmitted */
  bool in_transaction;      /* Bus transaction active flag */
} mock_i2c_slave_t;

/**
 * I2C Bus Controller Context
 */
typedef struct
{
  i2c_pin_t scl_pin;      /* SCL clock line */
  i2c_pin_t sda_pin;      /* SDA data line */
  mock_i2c_slave_t slave; /* Connected mock slave peripheral */
  uint32_t clock_cycles;  /* Total clock cycles executed */
} i2c_bus_t;

/* ==============================================================================
 * Hardware Abstraction Layer (HAL) & Mock Bus Architecture
 * ============================================================================== */

uint8_t i2c_hal_get_sda(const i2c_bus_t *bus);
uint8_t i2c_hal_get_scl(const i2c_bus_t *bus);
void i2c_hal_delay(void);
void i2c_hal_set_scl(i2c_bus_t *bus, uint8_t level);
void i2c_hal_set_sda(i2c_bus_t *bus, uint8_t level);
void mock_slave_init(mock_i2c_slave_t *slv, uint8_t address);
void mock_slave_on_bus_transition(i2c_bus_t *bus, uint8_t old_scl, uint8_t old_sda, uint8_t new_scl,
                                  uint8_t new_sda);

/* ==============================================================================
 * Driver Function Declarations
 * ============================================================================== */

int i2c_init(i2c_bus_t *bus);
int i2c_start(i2c_bus_t *bus);
int i2c_stop(i2c_bus_t *bus);
int i2c_write_byte(i2c_bus_t *bus, uint8_t byte);
int i2c_read_byte(i2c_bus_t *bus, uint8_t *byte, bool send_ack);
int i2c_write_register(i2c_bus_t *bus, uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
int i2c_read_register(i2c_bus_t *bus, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data);

#endif /* I2C_BITBANG_H */
