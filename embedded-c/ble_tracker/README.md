# BLE Proximity Tracker Firmware Modules

Embedded C firmware modules for a wireless proximity tracking accessory, covering bitfield payload decoding, interrupt-safe lock-free buffers, accelerometer motion debouncing, and concurrent multi-thread radio coordination.

## Module Files

| File | Focus Area | Description / Tests Verified |
| :--- | :--- | :--- |
| `ble_payload_parser.c` | Non-Aligned Bit Packing | Extracts a 224-bit P-224 public key, 2-bit device status, 6-bit battery level, and 16-bit little-endian checksum from a 31-byte raw BLE advertisement packet. |
| `isr_ring_buffer.c` | Lock-Free ISR Buffer | Single-producer / single-consumer circular buffer (`ring_buf_t`, capacity 16) for high-rate UWB ranging frames without mutexes or dynamic allocation. Tests full/empty boundary cases and wrap-around. |
| `motion_alert_debouncer.c` | Sensor Debouncing | Stateful rolling 60-second window accelerometer interrupt debouncer. Discards transient bumps; triggers alert chime only upon 5 motion detections within a 60s sliding window. |
| `secure_broadcast_coordinator.c` | Multi-Thread Synchronization | Coordinates crypto key rotation producer, motion sensor ISR simulator, and BLE broadcast radio consumer using mutexes and condition variables. Verifies 0 torn reads under concurrent execution. |

## How to Build & Run

### From repository root
```bash
make -C embedded-c/ble_tracker                # Build all binaries
make -C embedded-c/ble_tracker test           # Run all 4 test suites
make -C embedded-c/ble_tracker clean          # Clean artifacts
```

### From within this folder
```bash
make                                          # Build all binaries
make test                                     # Run all 4 test suites
make run FILE=ble_payload_parser              # Run specific binary
make run FILE=isr_ring_buffer
make run FILE=motion_alert_debouncer
make run FILE=secure_broadcast_coordinator
make clean                                    # Clean artifacts
```

### Compiling a single file directly
```bash
clang -Wall -Wextra -g -pthread ble_payload_parser.c -o ble_payload_parser
./ble_payload_parser
```
