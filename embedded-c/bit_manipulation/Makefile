CC      = clang
CFLAGS  = -Wall -Wextra -g
TARGETS = bit_basics bitfield_operations register_union_overlay

.PHONY: all test clean run

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

test: all
	@for target in $(TARGETS); do \
		echo "=== Running bit_manipulation/$$target ==="; \
		./$$target || exit 1; \
	done
	@echo "All bit_manipulation tests passed!"

run:
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run FILE=<target_name> (e.g., make run FILE=bit_basics)"; \
		exit 1; \
	fi; \
	./$(FILE)

clean:
	rm -rf $(TARGETS) *.o *.bin *.dSYM
