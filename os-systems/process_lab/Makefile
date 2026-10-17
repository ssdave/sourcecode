CC      = clang
CFLAGS  = -Wall -Wextra -g

.PHONY: all test clean run

all: process_test_runner

process_test_runner: process_test_runner.c
	$(CC) $(CFLAGS) process_test_runner.c -o process_test_runner

test: all
	@echo "=== Running process_lab parallel test runner ==="
	@./process_test_runner
	@echo "process_lab built and executed successfully!"

run: process_test_runner
	./process_test_runner

clean:
	rm -rf process_test_runner simple_test *.bin *.o *.dSYM
