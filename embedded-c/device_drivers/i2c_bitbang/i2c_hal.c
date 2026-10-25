/**
 * @file i2c_hal.c
 * @brief Open-Drain GPIO Hardware Emulation & Mock Slave Device for I2C Bus.
 */

#include "i2c_bitbang.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
void mock_slave_on_bus_transition(i2c_bus_t *bus, uint8_t old_scl, uint8_t old_sda, uint8_t new_scl,
                                  uint8_t new_sda)
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
