CC      = clang
CFLAGS  = -Wall -Wextra -g
TARGETS = power_of_two_ring_buffer counted_ring_buffer moving_average_filter

.PHONY: all test clean run

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

# Run fully implemented solutions for test verification
test: power_of_two_ring_buffer moving_average_filter
	@echo "=== Running ring_buffers/power_of_two_ring_buffer ==="
	@./power_of_two_ring_buffer || exit 1
	@echo "=== Running ring_buffers/moving_average_filter ==="
	@./moving_average_filter || exit 1
	@echo "All ring_buffers test targets passed!"

run:
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run FILE=<target_name> (e.g., make run FILE=power_of_two_ring_buffer)"; \
		exit 1; \
	fi; \
	./$(FILE)

clean:
	rm -rf $(TARGETS) *.o *.bin *.dSYM ringbuffer ring_buffer movingaverage
