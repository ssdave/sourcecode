CC      = clang
CFLAGS  = -Wall -Wextra -g -pthread
TARGETS = ble_payload_parser isr_ring_buffer motion_alert_debouncer secure_broadcast_coordinator

.PHONY: all test clean run

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

test: all
	@for target in $(TARGETS); do \
		echo "=== Running ble_tracker/$$target ==="; \
		./$$target || exit 1; \
	done
	@echo "All ble_tracker tests passed!"

run:
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run FILE=<target_name> (e.g., make run FILE=ble_payload_parser)"; \
		exit 1; \
	fi; \
	./$(FILE)

clean:
	rm -rf $(TARGETS) *.o *.bin *.dSYM myapp \
		"Problem1 - BLE Payload Bit Extraction" \
		"Problem2 - The Interrupt-Safe Ring Buffer" \
		"Problem 3 - The Anti-Stalking Debouncer" \
		"Problem 4 - The Secure Broadcast Coordinator"
