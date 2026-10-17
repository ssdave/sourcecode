CC      = clang
CFLAGS  = -Wall -Wextra -g -pthread
TARGETS = bounded_critical_section ordered_thread_entry stack_bounded_buffer two_stage_service_rooms tunnel_traffic_control

.PHONY: all test clean run

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

# Run the completed solutions for test verification
test: bounded_critical_section two_stage_service_rooms
	@echo "=== Running condition_variables/bounded_critical_section ==="
	@./bounded_critical_section || exit 1
	@echo "=== Running condition_variables/two_stage_service_rooms ==="
	@./two_stage_service_rooms || exit 1
	@echo "All condition_variables test targets passed!"

run:
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run FILE=<target_name> (e.g., make run FILE=bounded_critical_section)"; \
		exit 1; \
	fi; \
	./$(FILE)

clean:
	rm -rf $(TARGETS) *.o *.bin *.dSYM max inorder prodcons_condvar rooms tunnel