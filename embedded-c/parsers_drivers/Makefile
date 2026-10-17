CC      = clang
CFLAGS  = -Wall -Wextra -g
TARGETS = tlv_parser uint32_serializer camera_isp_driver

.PHONY: all test clean run

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

test: all
	@for target in $(TARGETS); do \
		echo "=== Running parsers_drivers/$$target ==="; \
		./$$target || exit 1; \
	done
	@echo "All parsers_drivers tests passed!"

run:
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run FILE=<target_name> (e.g., make run FILE=tlv_parser)"; \
		exit 1; \
	fi; \
	./$(FILE)

clean:
	rm -rf $(TARGETS) *.o *.bin *.dSYM serialize camera_isp
