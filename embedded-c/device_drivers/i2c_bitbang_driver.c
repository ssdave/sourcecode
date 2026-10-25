/**
 * @file i2c_bitbang_driver.c
 * @brief Open-Drain GPIO Bit-Banged I2C Bus Controller and Peripheral Driver.
 *
 * Concepts Demonstrated:
 * 1. Open-drain GPIO modeling with external pull-up resistors for SCL and SDA.
 * 2. I2C START and STOP framing condition generation on open-drain lines.
 * 3. 8-bit MSB-first byte serialization and synchronous clock pulse generation.
 * 4. 9th clock tick ACK/NACK protocol arbitration across master and slave.
 * 5. Multi-byte transaction framing for device register addressing and sensor reads.
 *
 * I2C (Inter-Integrated Circuit) Bus Protocol & Timing Diagram:
 *
 * SCL and SDA are open-drain lines pulled up to VDD via external pull-up resistors.
 * A line is actively driven LOW (0) by pulling it to ground.
 * A line is released to HIGH (1) by setting GPIO to open-drain input/float (pulled up by resistor).
 *
 * 1. START CONDITION:
 *    SDA transitions HIGH to LOW while SCL remains HIGH.
 *
 *         +----------------+
 *    SCL:                  \________
 *         +--------+
 *    SDA:          \________________
 *                  ^ START
 *
 * 2. BIT TRANSMISSION & SAMPLING:
 *    SDA must remain stable while SCL is HIGH.
 *    SDA changes state ONLY when SCL is LOW.
 *
 *               +---+       +---+
 *    SCL: ______/   \_______/   \______
 *            +---------+ +---------+
 *    SDA: XXX|  Bit 7  |X|  Bit 6  |XXX  (MSB first)
 *            +---------+ +---------+
 *                ^           ^
 *              Sample      Sample
 *
 * 3. ACK / NACK CONDITION (9th Clock Tick):
 *    Master releases SDA line (floats HIGH).
 *    Slave pulls SDA LOW to ACK (0), or leaves HIGH to NACK (1).
 *
 *               +---+
 *    SCL: ______/   \______
 *         ----+
 *    SDA:     \____________ (ACK = 0)
 *         ----------------- (NACK = 1)
 *
 * 4. STOP CONDITION:
 *    SDA transitions LOW to HIGH while SCL remains HIGH.
 *
 *         +------------------------+
 *    SCL: |
 *                +-----------------+
 *    SDA: _______/
 *                ^ STOP
 *
 * Academic Attribution:
 * Exercise design based on computer architecture and embedded systems peripheral protocols
 * (CS:APP Chapter 6, OSTEP Chapter 36, and NXP/Philips I2C Bus Specification UM10204).
 * Retained strictly for independent educational self-study.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==============================================================================
 * Hardware Abstraction Layer (HAL) & Mock Bus Architecture
 * ============================================================================== */

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

/* Forward declaration of internal transition handler */
static void mock_slave_on_bus_transition(i2c_bus_t *bus, uint8_t old_scl, uint8_t old_sda,
                                         uint8_t new_scl, uint8_t new_sda);

/**
 * Samples the physical logic level of the SDA data line.
 *
 * @param bus Pointer to the I2C bus structure.
 * @return 1 if line is high (both released), 0 if pulled low by either master or slave.
 */
uint8_t i2c_hal_get_sda(const i2c_bus_t *bus)
{
  if(bus == NULL)
  {
    return 1;
  }
  return (bus->sda_pin.master != 0 && bus->sda_pin.slave != 0) ? 1 : 0;
}

/**
 * Samples the physical logic level of the SCL clock line.
 *
 * @param bus Pointer to the I2C bus structure.
 * @return 1 if line is high (both released), 0 if pulled low.
 */
uint8_t i2c_hal_get_scl(const i2c_bus_t *bus)
{
  if(bus == NULL)
  {
    return 1;
  }
  return (bus->scl_pin.master != 0 && bus->scl_pin.slave != 0) ? 1 : 0;
}

/**
 * Simulated bus timing delay for half-clock cycles.
 */
void i2c_hal_delay(void)
{
  /* Simulated hardware propagation and clock period delay */
}

/**
 * Sets the master driver level for the SCL clock line.
 *
 * @param bus Pointer to the I2C bus structure.
 * @param level 0 to actively pull LOW, 1 to release (float HIGH via pull-up).
 */
void i2c_hal_set_scl(i2c_bus_t *bus, uint8_t level)
{
  if(bus == NULL)
  {
    return;
  }

  uint8_t old_scl = i2c_hal_get_scl(bus);
  uint8_t old_sda = i2c_hal_get_sda(bus);

  bus->scl_pin.master = (level != 0) ? 1 : 0;

  uint8_t new_scl = i2c_hal_get_scl(bus);
  uint8_t new_sda = i2c_hal_get_sda(bus);

  mock_slave_on_bus_transition(bus, old_scl, old_sda, new_scl, new_sda);
}

/**
 * Sets the master driver level for the SDA data line.
 *
 * @param bus Pointer to the I2C bus structure.
 * @param level 0 to actively pull LOW, 1 to release (float HIGH via pull-up).
 */
void i2c_hal_set_sda(i2c_bus_t *bus, uint8_t level)
{
  if(bus == NULL)
  {
    return;
  }

  uint8_t old_scl = i2c_hal_get_scl(bus);
  uint8_t old_sda = i2c_hal_get_sda(bus);

  bus->sda_pin.master = (level != 0) ? 1 : 0;

  uint8_t new_scl = i2c_hal_get_scl(bus);
  uint8_t new_sda = i2c_hal_get_sda(bus);

  mock_slave_on_bus_transition(bus, old_scl, old_sda, new_scl, new_sda);
}

/**
 * State machine handler for mock I2C slave responding to bus transitions.
 */
static void mock_slave_on_bus_transition(i2c_bus_t *bus, uint8_t old_scl, uint8_t old_sda,
                                         uint8_t new_scl, uint8_t new_sda)
{
  mock_i2c_slave_t *slv = &bus->slave;

  /* Detect START condition: SDA falls while SCL remains HIGH */
  if(old_scl == 1 && new_scl == 1 && old_sda == 1 && new_sda == 0)
  {
    slv->in_transaction = true;
    slv->state = SLAVE_STATE_ADDR;
    slv->bit_index = 0;
    slv->shift_reg = 0;
    bus->sda_pin.slave = 1;
    return;
  }

  /* Detect STOP condition: SDA rises while SCL remains HIGH */
  if(old_scl == 1 && new_scl == 1 && old_sda == 0 && new_sda == 1)
  {
    slv->in_transaction = false;
    slv->state = SLAVE_STATE_IDLE;
    slv->bit_index = 0;
    slv->shift_reg = 0;
    bus->sda_pin.slave = 1;
    return;
  }

  if(!slv->in_transaction)
  {
    return;
  }

  /* Rising edge of SCL: Sample data line */
  if(old_scl == 0 && new_scl == 1)
  {
    bus->clock_cycles++;

    if(slv->state == SLAVE_STATE_ADDR || slv->state == SLAVE_STATE_REG_PTR ||
       slv->state == SLAVE_STATE_WRITE_DATA)
    {
      if(slv->bit_index < 8)
      {
        slv->shift_reg = (uint8_t)((slv->shift_reg << 1) | (new_sda & 0x01));
        slv->bit_index++;
      }
    }
    else if(slv->state == SLAVE_STATE_READ_DATA)
    {
      slv->bit_index++;
    }
    else if(slv->state == SLAVE_STATE_READ_ACK)
    {
      /* Master sends ACK/NACK bit on 9th clock during read transaction */
      (void)new_sda;
    }
    return;
  }

  /* Falling edge of SCL: Setup next data bit or ACK */
  if(old_scl == 1 && new_scl == 0)
  {
    if(slv->state == SLAVE_STATE_ADDR)
    {
      if(slv->bit_index == 8)
      {
        uint8_t addr = (slv->shift_reg >> 1) & 0x7F;
        uint8_t rw = slv->shift_reg & 0x01;

        if(addr == slv->address)
        {
          /* Valid address: Assert ACK on 9th clock */
          bus->sda_pin.slave = 0;
          slv->bit_index = 0;
          if(rw == 0)
          {
            slv->state = SLAVE_STATE_ADDR_ACK_WRITE;
          }
          else
          {
            slv->state = SLAVE_STATE_ADDR_ACK_READ;
            slv->tx_byte = slv->registers[slv->selected_reg & 0x07];
          }
        }
        else
        {
          /* Address mismatch: NACK and abort */
          bus->sda_pin.slave = 1;
          slv->state = SLAVE_STATE_IDLE;
          slv->in_transaction = false;
        }
      }
    }
    else if(slv->state == SLAVE_STATE_ADDR_ACK_WRITE)
    {
      /* Release ACK after 9th clock */
      bus->sda_pin.slave = 1;
      slv->state = SLAVE_STATE_REG_PTR;
      slv->bit_index = 0;
      slv->shift_reg = 0;
    }
    else if(slv->state == SLAVE_STATE_ADDR_ACK_READ)
    {
      /* Release ACK and present bit 7 of read byte */
      slv->state = SLAVE_STATE_READ_DATA;
      slv->bit_index = 0;
      bus->sda_pin.slave = (slv->tx_byte >> 7) & 0x01;
    }
    else if(slv->state == SLAVE_STATE_REG_PTR)
    {
      if(slv->bit_index == 8)
      {
        /* Register address received: store pointer and assert ACK */
        slv->selected_reg = slv->shift_reg & 0x07;
        bus->sda_pin.slave = 0;
        slv->state = SLAVE_STATE_REG_ACK;
        slv->bit_index = 0;
      }
    }
    else if(slv->state == SLAVE_STATE_REG_ACK)
    {
      /* Release ACK after 9th clock */
      bus->sda_pin.slave = 1;
      slv->state = SLAVE_STATE_WRITE_DATA;
      slv->bit_index = 0;
      slv->shift_reg = 0;
    }
    else if(slv->state == SLAVE_STATE_WRITE_DATA)
    {
      if(slv->bit_index == 8)
      {
        /* Data byte received: store into register and assert ACK */
        slv->registers[slv->selected_reg & 0x07] = slv->shift_reg;
        bus->sda_pin.slave = 0;
        slv->state = SLAVE_STATE_WRITE_ACK;
        slv->bit_index = 0;
      }
    }
    else if(slv->state == SLAVE_STATE_WRITE_ACK)
    {
      /* Release ACK after 9th clock */
      bus->sda_pin.slave = 1;
      slv->state = SLAVE_STATE_WRITE_DATA;
      slv->bit_index = 0;
      slv->shift_reg = 0;
    }
    else if(slv->state == SLAVE_STATE_READ_DATA)
    {
      if(slv->bit_index < 8)
      {
        /* Present next bit (MSB first) */
        bus->sda_pin.slave = (slv->tx_byte >> (7 - slv->bit_index)) & 0x01;
      }
      else
      {
        /* All 8 bits transmitted: release SDA for master ACK/NACK */
        bus->sda_pin.slave = 1;
        slv->state = SLAVE_STATE_READ_ACK;
        slv->bit_index = 0;
      }
    }
    else if(slv->state == SLAVE_STATE_READ_ACK)
    {
      /* Release SDA after 9th clock */
      bus->sda_pin.slave = 1;
      slv->state = SLAVE_STATE_IDLE;
    }
  }
}

/**
 * Initializes the mock slave device with default register contents.
 *
 * @param slv Pointer to the mock slave structure.
 * @param address 7-bit I2C device address.
 */
void mock_slave_init(mock_i2c_slave_t *slv, uint8_t address)
{
  if(slv == NULL)
  {
    return;
  }

  memset(slv, 0, sizeof(*slv));
  slv->address = address;
  slv->state = SLAVE_STATE_IDLE;
  slv->in_transaction = false;

  /* Default sensor register values (LM75 / TMP102 compatible) */
  slv->registers[0] = 0x19; /* Reg 0: Temperature (25°C) */
  slv->registers[1] = 0x00; /* Reg 1: Configuration */
  slv->registers[2] = 0x4B; /* Reg 2: T_LOW (75°C) */
  slv->registers[3] = 0x50; /* Reg 3: T_HIGH (80°C) */
  slv->selected_reg = 0x00;
}

/* ==============================================================================
 * Driver Function Declarations & Detailed Protocol Contracts
 * ============================================================================== */

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

/* ==============================================================================
 * Verification Test Harness
 * ============================================================================== */

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
