CC      = clang
CFLAGS  = -Wall -Wextra -g -pthread
TARGETS = mutex_shared_counter \
          mutex_channel_lock_ordering \
          condvar_thread_join \
          condvar_barrier \
          bounded_buffer_producer_consumer \
          custom_semaphore \
          reader_writer_lock \
          dining_philosophers \
          atomic_spinlock

.PHONY: all test clean run condition_variables

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

test: all
	@for target in $(TARGETS); do \
		echo "=== Running concurrency/$$target ==="; \
		./$$target || exit 1; \
	done
	@echo "All concurrency tests passed!"
	@$(MAKE) -C condition_variables test

run:
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run FILE=<target_name> (e.g., make run FILE=mutex_shared_counter)"; \
		exit 1; \
	fi; \
	$(MAKE) $(FILE); \
	./$(FILE)

clean:
	rm -rf $(TARGETS) *.o *.bin *.dSYM testout.txt basicMutex1 basicMutex2
	@$(MAKE) -C condition_variables clean