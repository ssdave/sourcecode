/**
 * @file i2c_mock_hw.c
 * @brief Mock I2C physical bus controller and peripheral sensor simulation.
 *
 * Academic Attribution:
 * Exercise design based on UNIX device driver architectures and the Linux kernel VFS subsystem
 * (OSTEP Chapters 36 & 39, CS:APP Chapter 10, Corbet/Rubini/Kroah-Hartman "Linux Device Drivers",
 * and Linux kernel drivers/i2c/i2c-dev.c).
 * Retained strictly for independent educational self-study.
 */

#include "i2c_cdev.h"

i2c_cdev_device_t g_i2c_controller;

/**
 * Initializes the mock peripheral registers (LM75 temperature sensor defaults).
 *
 * @param slv Pointer to mock slave peripheral structure.
 * @param address 7-bit slave address.
 */
void mock_slave_init(mock_i2c_slave_t *slv, uint8_t address)
{
  if(slv == NULL)
  {
    return;
  }

  memset(slv, 0, sizeof(*slv));
  slv->address = address;
  slv->selected_reg = 0;

  /* Default sensor register map */
  slv->registers[0] = 0x19; /* Reg 0: Temperature (25°C) */
  slv->registers[1] = 0x00; /* Reg 1: Configuration */
  slv->registers[2] = 0x4B; /* Reg 2: T_LOW (75°C) */
  slv->registers[3] = 0x50; /* Reg 3: T_HIGH (80°C) */
}

/**
 * Executes a simulated physical I2C hardware bus transaction.
 *
 * Protocol Invariant:
 * Bus access must be serialized by the driver using dev->bus_lock before calling.
 *
 * @param dev Pointer to I2C controller device.
 * @param addr 7-bit target slave address.
 * @param buf Data buffer (TX payload for write; RX destination for read).
 * @param count Number of bytes to transfer.
 * @param is_read True for read transaction; False for write transaction.
 * @return 0 on success, or -EIO if device address NACKs.
 */
int i2c_hardware_transfer(i2c_cdev_device_t *dev, uint8_t addr, uint8_t *buf, size_t count,
                          bool is_read)
{
  if(dev == NULL || buf == NULL)
  {
    return -EINVAL;
  }

  mock_i2c_slave_t *slv = &dev->slave;

  /* Check slave address match */
  if(slv->address != addr)
  {
    dev->stats.nack_count++;
    return -EIO; /* Device NACK (address not found on bus) */
  }

  if(is_read)
  {
    /* Read transaction: Stream bytes from selected register */
    for(size_t i = 0; i < count; i++)
    {
      uint8_t reg_idx = (slv->selected_reg + (uint8_t)i) % 8;
      buf[i] = slv->registers[reg_idx];
    }
  }
  else
  {
    /* Write transaction: First byte sets register pointer; subsequent bytes write data */
    if(count > 0)
    {
      slv->selected_reg = buf[0] % 8;
      for(size_t i = 1; i < count; i++)
      {
        uint8_t reg_idx = (slv->selected_reg + (uint8_t)(i - 1)) % 8;
        slv->registers[reg_idx] = buf[i];
      }
    }
  }

  return 0;
}

/**
 * Resets the physical I2C bus state.
 *
 * @param dev Pointer to I2C controller device.
 */
void i2c_bus_reset(i2c_cdev_device_t *dev)
{
  if(dev == NULL)
  {
    return;
  }

  dev->slave.selected_reg = 0;
}
