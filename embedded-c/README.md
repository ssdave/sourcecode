# Embedded C, Firmware & Concurrency Practice (`embedded-c`)

Hands-on embedded C, register bit manipulation, lock-free circular buffers, binary protocol parsing, and POSIX threads (`pthreads`) synchronization exercises with self-contained verification harnesses, drawing from **embedded systems practice problems, university operating systems / concurrency courses, and classic C systems references**.

## Building & Running All Exercises

```bash
# Build every subfolder from embedded-c/
make

# Clean all compiled binaries
make clean
```

---

## Modules, What Is Built & What Is Tested

### 1. BLE Proximity Tracker Firmware ([`./ble_tracker/`](./ble_tracker))
- **[`Problem1 - BLE Payload Bit Extraction.c`](./ble_tracker/Problem1%20-%20BLE%20Payload%20Bit%20Extraction.c)** *(Implemented — Passing)*
  - *Built*: `parse_ble_payload()` unpacks a raw 31-byte BLE advertisement frame into a 28-byte P-224 public key, 2-bit device state (`raw[28] & 0x03`), 6-bit battery percentage (`(raw[28] & 0xFC) >> 2`), and 16-bit little-endian checksum (`raw[29] | (raw[30] << 8)`).
  - *Tested*: Verifies extraction against a synthetic 31-byte packet encoding State `2` (Lost Mode), Battery `50%` (`0xCA`), and Checksum `0xABCD`.
- **[`Problem2 - The Interrupt-Safe Ring Buffer.c`](./ble_tracker/Problem2%20-%20The%20Interrupt-Safe%20Ring%20Buffer.c)** *(Implemented — Passing)*
  - *Built*: Statically allocated, lock-free SPSC circular queue (`BUFFER_SIZE = 16`) for UWB ranging packets (`distance_mm`, `angle_degrees`) using power-of-two bitmask indexing (`BUFFER_SIZE - 1`).
  - *Tested*: 5-stage harness testing full 16-item fill, 17th-item overfill rejection, FIFO drain ordering, empty-buffer underflow protection, and physical array boundary wrap-around.
- **[`Problem 3 - The Anti-Stalking Debouncer.c`](./ble_tracker/Problem%203%20-%20The%20Anti-Stalking%20Debouncer.c)** *(Implemented — Passing)*
  - *Built*: Stateful 60-second rolling-window accelerometer interrupt debouncer (`on_movement_detected`) that triggers `play_chime()` only when 5 motion events occur within 60 seconds.
  - *Tested*: Verifies scattered noise rejection (5 events over 100s), continuous motion detection (5 events within 40s), and sliding-window expiration of stale events after a pause.
- **[`Problem 4 - The Secure Broadcast Coordinator.c`](./ble_tracker/Problem%204%20-%20The%20Secure%20Broadcast%20Coordinator.c)** *(Implemented — Passing)*
  - *Built*: Multi-threaded synchronization between a key-rotating `crypto_thread`, an ISR-simulating `sensor_thread`, and a `ble_radio_thread` using `pthread_mutex_t` and `pthread_cond_t` to prevent torn reads of the 28-byte public key.
  - *Tested*: 1-second stress test with 50us per-byte write delays verifying `torn_read_count == 0` and clean thread shutdown.

### 2. POSIX Threads & Synchronization ([`./concurrency/`](./concurrency/README.md))
*Draws from university Operating Systems and concurrency curricula (including **Rose-Hulman Institute of Technology — CSSE 332: Operating Systems** ([Course Schedule](https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/) | [Lab Archive](https://github.com/rhit-csse332/csse332-labs-archive)), **Jacobs University Bremen**, and **OSTEP**). See [`./concurrency/README.md`](./concurrency/README.md) for the complete file-by-file mapping and academic attribution.*
- **`basicMutex1.c`, `basicMutex2.c`** *(Implemented — Passing)* — Shared counter mutual exclusion (`10 * 10,000` increments) and deadlock-free dual-channel (`redLock`/`blueLock`) lock ordering.
- **`condwait.c`, `barrier.c`, `producerconsumer.c`** *(Implemented — Passing)* — Custom `thread_join` via condition variables, 5-thread rendezvous barrier, and 5-producer/5-consumer circular bounded buffer.
- **`conditionalvariables/max.c` & `conditionalvariables/rooms.c`** *(Implemented — Passing)* — Bounded 3-thread critical section (`max.c`) and two-stage customer service rooms with bounded waiting room and balking (`rooms.c`).
- **`conditionalvariables/inorder.c`, `prodcons_condvar.c`, `tunnel.c`** *(Lab Starter Templates)* — Unimplemented practice templates preserved for additional condition-variable exercises.

### 3. Bit Manipulation & Hardware Registers ([`./bit_manipulation/`](./bit_manipulation))
- **[`bit_basics.c`](./bit_manipulation/bit_basics.c)** *(Implemented — Passing)* — Implements and tests `set_bit`, `clear_bit`, `toggle_bit`, `check_bit`, and `is_power_of_two` (`val & (val - 1) == 0`) with bit-index bounds guards.
- **[`bit_level2.c`](./bit_manipulation/bit_level2.c)** *(Implemented — Passing)* — Implements and tests `count_set_bits`, `extract_bitfield`, and read-modify-write `insert_bitfield`.
- **[`registers.c`](./bit_manipulation/registers.c)** *(Implemented — Passing)* — Demonstrates 32-bit hardware register access via a `union` and 8-bit bitfield `struct`.

### 4. Circular Buffers & Signal Filtering ([`./ring_buffers/`](./ring_buffers))
- **[`ringbuffer.c`](./ring_buffers/ringbuffer.c)** *(Implemented — Passing)* — Power-of-two circular queue (`BUFFER_SIZE = 8`) using `(index + 1) & BUFFER_MASK` wrap-around, tested for FIFO ordering and full/empty boundaries.
- **[`movingaverage.c`](./ring_buffers/movingaverage.c)** *(Implemented — Passing)* — O(1) sliding-window moving average filter (`WINDOW_SIZE = 4`) tested across ramp-up, steady-state wrap-around, and negative/zero inputs.
- **[`ring_buffer.c`](./ring_buffers/ring_buffer.c)** *(Practice Harness / Starter Template)* — Struct-based byte ring buffer unit-test harness kept as a timed practice template.

### 5. Protocol Parsers, Serialization & ISP ([`./parsers_drivers/`](./parsers_drivers))
- **[`tlv_parser.c`](./parsers_drivers/tlv_parser.c)** *(Implemented — Passing)* — Defensive `[Tag][Length][Value]` stream parser tested against multi-TLV frames, zero-length records, truncated headers, oversized lengths, and `NULL` buffers.
- **[`serialize.c`](./parsers_drivers/serialize.c)** *(Implemented — Passing)* — Round-trip `uint32_t` <-> `uint8_t[4]` byte serialization.
- **[`camera_isp.c`](./parsers_drivers/camera_isp.c)** *(Part 1 Implemented; Parts 2–5 Practice Stubs)* — Implements and passes Part 1 (`update_camera_register` read-modify-write on `bits [15:12]`); Parts 2–5 (lock-free VSYNC frame queue, overlapping `custom_memmove`, and non-blocking PMU rail state machine) are practice stubs.

### 6. Pointers, Strings & Algorithms ([`./pointers_algorithms/`](./pointers_algorithms))
- **[`double_pointer.c`](./pointers_algorithms/double_pointer.c)** *(Implemented — Passing)* — Tests `allocate_buffer(uint8_t **)`, `list_push(Node **)`, and `next_token(char **, char)` delimiter parsing.
- **[`remove_char.c`](./pointers_algorithms/remove_char.c)** *(Implemented — Passing)* — Two-pointer O(N) in-place character removal tested on interior matches, all-matching strings, empty strings, and `NULL`.
- **[`Hello.c`](./pointers_algorithms/Hello.c)** *(Implemented — Passing)* — Heap-allocated singly linked list construction and traversal.
- **[`binarysearch.c`](./pointers_algorithms/binarysearch.c)** *(Practice Harness / Starter Template)* — 15-case boundary test suite for iterative binary search.

### 7. Standalone Libraries with Unit Test Suites
- **[`./endian_practice/`](./endian_practice)** *(Implemented — Passing)* — `swap_endian_16`, `swap_endian_32`, `swap_endian_64`, `is_system_little_endian`, and `swap_endian_buffer_32`.
- **[`./atoi_itoa_practice/`](./atoi_itoa_practice)** *(Implemented — Passing)* — Custom `my_atoi` and `my_itoa` with assertion test runner.

### 8. Hardware Interfacing, MMIO & Device Drivers ([`./device_drivers/`](./device_drivers))
*Draws from computer architecture, bus specifications, and kernel device driver principles (CS:APP Chapter 6, OSTEP Chapter 36, PCI Express Base Specification, Linux kernel driver architecture).*
- **[`mmio_uart_driver.c`](./device_drivers/mmio_uart_driver.c)** *(Scaffolded — Ready for Implementation)* — Memory-Mapped I/O hardware peripheral controller (`SR`, `DR`, `CR`, `BSRR`), polling TX/RX, and atomic bit-set-reset operations.
- **[`pcie_config_driver.c`](./device_drivers/pcie_config_driver.c)** *(Scaffolded — Ready for Implementation)* — PCIe Type 0 Configuration Space probe and Base Address Register (BAR) sizing engine (`0xFFFFFFFF` aperture probe, 32-bit vs 64-bit BARs).
- **[`dma_ring_buffer_driver.c`](./device_drivers/dma_ring_buffer_driver.c)** *(Scaffolded — Ready for Implementation)* — Coherent DMA circular descriptor ring controller with ownership handoffs (`OWN` bit), hardware doorbell notification, and completion reaping.

