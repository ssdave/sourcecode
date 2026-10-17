# ==============================================================================
# Root Makefile for /Users/sanketdave/sourcecode
#
# Delegates build, test, and clean actions across all engineering modules.
#
# Usage:
#   make         - Build all modules and binaries
#   make test    - Run automated test suites across all modules
#   make clean   - Clean build artifacts across all modules
#   make format  - Format all C/C++ source files using .clang-format
# ==============================================================================

MODULES = \
	embedded-c/bit_manipulation \
	embedded-c/ble_tracker \
	embedded-c/concurrency \
	embedded-c/parsers_drivers \
	embedded-c/pointers_algorithms \
	embedded-c/ring_buffers \
	os-systems/simpleshell \
	os-systems/process_lab \
	leetcode

.PHONY: all test clean format $(MODULES)

all:
	@for mod in $(MODULES); do \
		echo "=== Building $$mod ==="; \
		$(MAKE) -C $$mod all || exit 1; \
	done
	@echo "All modules built successfully."

test:
	@for mod in $(MODULES); do \
		echo "=== Testing $$mod ==="; \
		$(MAKE) -C $$mod test || exit 1; \
	done
	@echo "============================================================"
	@echo "All repository test suites passed successfully!"
	@echo "============================================================"

clean:
	@for mod in $(MODULES); do \
		echo "=== Cleaning $$mod ==="; \
		$(MAKE) -C $$mod clean || exit 1; \
	done
	@echo "Clean complete."

CLANG_FORMAT ?= $(shell which clang-format 2>/dev/null || echo "xcrun clang-format")

format:
	@find embedded-c os-systems leetcode -type f \( -name "*.c" -o -name "*.h" \) -exec $(CLANG_FORMAT) -i -style=file {} +
	@echo "clang-format applied to all C source and header files."
