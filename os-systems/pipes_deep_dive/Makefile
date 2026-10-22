CC      = clang
CFLAGS  = -Wall -Wextra -g

BINARIES = \
	prime_sieve_pipeline \
	phantom_writer \
	broken_pipe \
	multi_writer_collector \
	stream_null_terminator

.PHONY: all test clean run-primes run-phantom run-broken-pipe run-multi-writer run-stream

all: $(BINARIES)

prime_sieve_pipeline: prime_sieve_pipeline.c
	$(CC) $(CFLAGS) prime_sieve_pipeline.c -o prime_sieve_pipeline

phantom_writer: phantom_writer.c
	$(CC) $(CFLAGS) phantom_writer.c -o phantom_writer

broken_pipe: broken_pipe.c
	$(CC) $(CFLAGS) broken_pipe.c -o broken_pipe

multi_writer_collector: multi_writer_collector.c
	$(CC) $(CFLAGS) multi_writer_collector.c -o multi_writer_collector

stream_null_terminator: stream_null_terminator.c
	$(CC) $(CFLAGS) stream_null_terminator.c -o stream_null_terminator

test: all
	@echo "=== Running prime_sieve_pipeline ==="
	@./prime_sieve_pipeline
	@echo "=== Running phantom_writer ==="
	@./phantom_writer
	@echo "=== Running broken_pipe ==="
	@./broken_pipe
	@echo "=== Running multi_writer_collector ==="
	@./multi_writer_collector
	@echo "=== Running stream_null_terminator ==="
	@./stream_null_terminator
	@echo "============================================================"
	@echo "All pipes_deep_dive exercises executed successfully!"
	@echo "============================================================"

run-primes: prime_sieve_pipeline
	./prime_sieve_pipeline

run-phantom: phantom_writer
	./phantom_writer

run-broken-pipe: broken_pipe
	./broken_pipe

run-multi-writer: multi_writer_collector
	./multi_writer_collector

run-stream: stream_null_terminator
	./stream_null_terminator

clean:
	rm -rf $(BINARIES) *.bin *.o *.dSYM
