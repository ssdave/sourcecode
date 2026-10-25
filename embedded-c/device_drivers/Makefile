CC      = clang
CFLAGS  = -Wall -Wextra -g -pthread
TARGETS = mmio_uart_driver \
          pcie_config_driver \
          dma_ring_buffer_driver \
          i2c_bitbang_driver \
          spi_bitbang_driver \
          i2s_bitbang_driver \
          rtos_sensor_pipeline

.PHONY: all test clean run

all: $(TARGETS)

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
	rm -rf $(TARGETS) *.o *.bin *.dSYM
