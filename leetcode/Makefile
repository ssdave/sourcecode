# ============================================================================
# Makefile for /Users/sanketdave/sourcecode/leetcode
#
# Usage:
#   make               - Compile all 26 LeetCode C solutions
#   make test          - Compile and run all 26 LeetCode C test suites
#   make run ID=0001   - Compile and run a specific problem by ID (e.g., ID=1 or ID=0001)
#   make 0001-two-sum  - Compile and run a specific problem directory
#   make sync          - Pull & format newly accepted LeetCode submissions
#   make clean         - Remove all compiled solution binaries and .dSYM bundles
# ============================================================================

CC       = clang
CFLAGS   = -Wall -Wextra -g -O2
LDFLAGS  = -lpthread

PROBLEM_DIRS := $(sort $(patsubst %/solution.c,%,$(wildcard *-*/solution.c)))
BINARIES     := $(addsuffix /solution,$(PROBLEM_DIRS))

.PHONY: all test run sync clean $(PROBLEM_DIRS)

all: $(BINARIES)

%/solution: %/solution.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(PROBLEM_DIRS): %: %/solution
	@./$</solution

run:
	@if [ -z "$(ID)" ]; then \
		echo "Usage: make run ID=0001 (or make 0001-two-sum)"; \
		exit 1; \
	fi; \
	PADDED=$$(printf "%04d" $(ID) 2>/dev/null || echo "$(ID)"); \
	DIR=$$(ls -d $${PADDED}-* 2>/dev/null | head -n 1); \
	if [ -z "$$DIR" ]; then \
		echo "No LeetCode directory found for ID=$(ID)"; \
		exit 1; \
	fi; \
	$(CC) $(CFLAGS) "$$DIR/solution.c" -o "$$DIR/solution" $(LDFLAGS) && "./$$DIR/solution"

test: $(BINARIES)
	@for bin in $(BINARIES); do \
		./$$bin || exit 1; \
		echo ""; \
	done
	@echo "============================================================"
	@echo "All $(words $(BINARIES)) LeetCode solutions built and passed basic test cases!"
	@echo "============================================================"

sync:
	../.venv/bin/python sync_leetcode.py

clean:
	rm -rf *-*/solution *-*/solution.dSYM
