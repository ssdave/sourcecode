/**
 * @file spi_bitbang.h
 * @brief Full-Duplex GPIO Bit-Banged SPI Bus Controller & Peripheral Interface.
 *
 * Concepts Demonstrated:
 * 1. Synchronous 4-wire serial bus protocol: SCLK, MOSI, MISO, and CS (active-low).
 * 2. Complete SPI Mode support: Mode 0, Mode 1, Mode 2, and Mode 3 (CPOL and CPHA matrix).
 * 3. Simultaneous full-duplex 8-bit MSB-first byte transmission and reception.
 * 4. Multi-byte buffer streaming and chip-select framing invariants.
 * 5. High-level peripheral command orchestration against a mock SPI Flash/EEPROM device
 *    (JEDEC ID readout, status register polling, write latching, and memory read/write).
 *
 * Academic Attribution:
 * Exercise design based on computer architecture and embedded peripheral bus standards
 * (CS:APP Chapter 6, OSTEP Chapter 36, and Motorola/NXP SPI Block Guide V04.01).
 * Retained strictly for independent educational self-study.
 */

#ifndef SPI_BITBANG_H
#define SPI_BITBANG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

/* ==============================================================================
 * Hardware Abstraction Layer (HAL) & Mock Slave Architecture
 * ============================================================================== */

uint8_t spi_hal_get_miso(const spi_bus_t *bus);
void spi_hal_set_sclk(spi_bus_t *bus, uint8_t level);
void spi_hal_set_mosi(spi_bus_t *bus, uint8_t level);
void spi_hal_set_cs(spi_bus_t *bus, uint8_t level);
void spi_hal_delay(void);
void mock_spi_slave_init(mock_spi_slave_t *slv, spi_mode_t mode);
void mock_slave_on_sclk_edge(spi_bus_t *bus, uint8_t edge_type);
void mock_slave_on_cs_change(spi_bus_t *bus, uint8_t old_cs, uint8_t new_cs);

/* ==============================================================================
 * Driver Function Declarations
 * ============================================================================== */

int spi_init(spi_bus_t *bus, spi_mode_t mode);
int spi_chip_select(spi_bus_t *bus);
int spi_chip_deselect(spi_bus_t *bus);
uint8_t spi_transfer_byte(spi_bus_t *bus, uint8_t tx_byte);
int spi_transfer(spi_bus_t *bus, const uint8_t *tx_buf, uint8_t *rx_buf, size_t len);
int spi_read_device_id(spi_bus_t *bus, uint8_t *id_buf, size_t id_len);
int spi_read_memory(spi_bus_t *bus, uint8_t addr, uint8_t *data_buf, size_t len);
int spi_write_memory(spi_bus_t *bus, uint8_t addr, const uint8_t *data_buf, size_t len);

#endif /* SPI_BITBANG_H */
