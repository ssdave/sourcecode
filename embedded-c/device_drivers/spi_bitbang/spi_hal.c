/**
 * @file spi_hal.c
 * @brief GPIO Hardware Simulation & Mock SPI Flash Peripheral Model.
 */

#include "spi_bitbang.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
void mock_slave_on_cs_change(spi_bus_t *bus, uint8_t old_cs, uint8_t new_cs)
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
void mock_slave_on_sclk_edge(spi_bus_t *bus, uint8_t edge_type)
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
