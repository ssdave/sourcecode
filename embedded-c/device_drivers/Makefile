CC      = clang
CFLAGS  = -Wall -Wextra -g -pthread
INCLUDES = -I. -Ii2c_cdev -Ii2c_bitbang -Ispi_bitbang -Irtos_pipeline
TARGETS = mmio_uart_driver \
          pcie_config_driver \
          dma_ring_buffer_driver \
          i2c_bitbang_driver \
          spi_bitbang_driver \
          i2s_bitbang_driver \
          rtos_sensor_pipeline \
          i2c_cdev_subsystem

I2C_CDEV_SRCS = i2c_cdev/i2c_mock_hw.c \
                i2c_cdev/cdev_vfs.c \
                i2c_cdev/i2c_cdev_driver.c \
                i2c_cdev/main.c

I2C_BITBANG_SRCS = i2c_bitbang/i2c_hal.c \
                   i2c_bitbang/i2c_driver.c \
                   i2c_bitbang/main.c

SPI_BITBANG_SRCS = spi_bitbang/spi_hal.c \
                   spi_bitbang/spi_driver.c \
                   spi_bitbang/main.c

RTOS_PIPELINE_SRCS = rtos_pipeline/spsc_ring.c \
                     rtos_pipeline/sensor_pipeline.c \
                     rtos_pipeline/main.c

.PHONY: all test clean run

all: $(TARGETS)

i2c_cdev_subsystem: $(I2C_CDEV_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

i2c_bitbang_driver: $(I2C_BITBANG_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

spi_bitbang_driver: $(SPI_BITBANG_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

rtos_sensor_pipeline: $(RTOS_PIPELINE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

%: %.c
	$(CC) $(CFLAGS) $< -o $@

test: all
	@for target in $(TARGETS); do \
		echo "=== Running device_drivers/$$target ==="; \
		./$$target || exit 1; \
	done
	@echo "All device driver tests passed!"

run:
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run FILE=<target_name> (e.g., make run FILE=mmio_uart_driver)"; \
		exit 1; \
	fi; \
	$(MAKE) $(FILE); \
	./$(FILE)

clean:
	rm -rf $(TARGETS) *.o *.bin *.dSYM \
	       i2c_cdev/*.o i2c_cdev/*.dSYM \
	       i2c_bitbang/*.o i2c_bitbang/*.dSYM \
	       spi_bitbang/*.o spi_bitbang/*.dSYM \
	       rtos_pipeline/*.o rtos_pipeline/*.dSYM
