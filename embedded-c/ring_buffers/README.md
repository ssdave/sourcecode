# Circular Ring Buffers & DSP Filters

Power-of-two bitmask indexing, struct-encapsulated circular FIFO queues, and sliding-window sensor moving average filtering.

## Module Files

| File | Status | Focus Area | Description / Tests Verified |
| :--- | :--- | :--- | :--- |
| `power_of_two_ring_buffer.c` | **Implemented** | Power-of-Two Index Masking | Static circular queue (`BUFFER_SIZE = 8`) using bitwise AND masking (`(index + 1) & BUFFER_MASK`) instead of expensive modulo division. Verifies overflow rejection and FIFO dequeue. |
| `moving_average_filter.c` | **Implemented** | O(1) DSP Moving Average | Sliding-window moving average filter (`WINDOW_SIZE = 4`) for incoming sensor streams. Subtracts oldest sample and adds newest sample without re-summing. Tests step-response convergence. |
| `counted_ring_buffer.c` | *Starter Template* | Encapsulated Struct Queue | Struct-encapsulated byte ring buffer (`ring_buffer_t`) with explicit sample counting. Starter harness preserved for timed practice. |

## How to Build & Run

### From repository root
```bash
make -C embedded-c/ring_buffers           # Build all binaries
make -C embedded-c/ring_buffers test      # Run all implemented tests
make -C embedded-c/ring_buffers clean     # Clean artifacts
```

### From within this folder
```bash
make                                      # Build all binaries
make test                                 # Run all implemented tests
make run FILE=power_of_two_ring_buffer    # Run specific binary
make run FILE=moving_average_filter
make run FILE=counted_ring_buffer
make clean                                # Clean artifacts
```

### Compiling a single file directly
```bash
clang -Wall -Wextra -g power_of_two_ring_buffer.c -o power_of_two_ring_buffer
./power_of_two_ring_buffer
```
