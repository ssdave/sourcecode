CC      = clang
CFLAGS  = -Wall -Wextra -g
TARGETS = double_pointer remove_char binary_search singly_linked_list

.PHONY: all test clean run

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

# Test targets: run fully implemented exercises
test: double_pointer remove_char singly_linked_list
	@echo "=== Running pointers_algorithms/double_pointer ==="
	@./double_pointer || exit 1
	@echo "=== Running pointers_algorithms/remove_char ==="
	@./remove_char || exit 1
	@echo "=== Running pointers_algorithms/singly_linked_list ==="
	@./singly_linked_list || exit 1
	@echo "All pointers_algorithms test targets passed!"

run:
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run FILE=<target_name> (e.g., make run FILE=singly_linked_list)"; \
		exit 1; \
	fi; \
	./$(FILE)

clean:
	rm -rf $(TARGETS) *.o *.bin *.dSYM Hello binarysearch
