# Systems Programming, Embedded C & iOS Practice

Personal repository for hands-on systems programming, embedded C/firmware development, POSIX threads (`pthreads`) concurrency, operating systems internals, and Swift/UIKit engineering practice.

> **Note on Workflow, Authentic Code & AI Code Review**:
> - **Authentic Author Code**: All core problem solutions, data structures, concurrency synchronization schemes, and operating systems exercises in this repository are written hands-on by the author to showcase genuine progression, deep conceptual understanding, and systems practice.
> - **AI Agents & Automated Review**: Collaborative AI subagents (`builder` and `reviewer`) format code style (`.clang-format`), maintain build infrastructure, and perform adversarial code and security reviews.
> - **Transparent Review Feedback**: Rather than silently rewriting or artificially "perfecting" author code, review observations regarding memory management, edge cases, and algorithmic nuances are tagged directly in the code with **`// AI Review Feedback:`** comments so learning opportunities and iterative improvements are completely transparent to external readers, code reviewers, and engineering teams.
>
> ⚡ **Systems & Embedded 15-Minute Quick Reference**: See [`QUICK_REFERENCE.md`](./QUICK_REFERENCE.md) for an ultra-dense, 180-line revision card designed for a 15–20 minute review of core invariants, bit formulas, lock rules, and memory models.  
> 📖 **Systems Architecture & Reference Treatise**: See [`SYSTEMS_REFERENCE.md`](./SYSTEMS_REFERENCE.md) for the comprehensive textbook treatise covering first-principles theory, hardware trade-offs, and native Mermaid architecture diagrams.

---

## Quick Start: Building & Running

A root [`Makefile`](./Makefile) orchestrates building and testing across all engineering modules:

```bash
# Build all modules and binaries across the entire repository
make

# Run all automated test suites and harnesses
make test

# Format all C source and header files using repository .clang-format
make format

# Clean all build artifacts
make clean
```

---

## Running a Specific Module

Every module subfolder contains its own standalone [`Makefile`](./Makefile) and detailed `README.md` with file tables and test instructions. You can build, test, and run any individual module from the repository root or by navigating into its directory:

| Module | Description | Run Tests | Run a Single File | Module Documentation |
| :--- | :--- | :--- | :--- | :--- |
| **Bit Manipulation** | Bitfield extraction, Hamming weight, register overlays | `make -C embedded-c/bit_manipulation test` | `make -C embedded-c/bit_manipulation run FILE=bit_basics` | [`embedded-c/bit_manipulation/README.md`](./embedded-c/bit_manipulation/README.md) |
| **BLE Tracker Firmware** | Unaligned bit unpacking, lock-free ISR queue, debouncer | `make -C embedded-c/ble_tracker test` | `make -C embedded-c/ble_tracker run FILE=ble_payload_parser` | [`embedded-c/ble_tracker/README.md`](./embedded-c/ble_tracker/README.md) |
| **POSIX Concurrency** | Mutexes, deadlocks, barriers, producer-consumer queues | `make -C embedded-c/concurrency test` | `make -C embedded-c/concurrency run FILE=mutex_shared_counter` | [`embedded-c/concurrency/README.md`](./embedded-c/concurrency/README.md) |
| **Condition Variables** | Counting semaphores, multi-stage pipelines, traffic control | `make -C embedded-c/concurrency/condition_variables test` | `make -C embedded-c/concurrency/condition_variables run FILE=bounded_critical_section` | [`embedded-c/concurrency/condition_variables/README.md`](./embedded-c/concurrency/condition_variables/README.md) |
| **Device Drivers & MMIO** | MMIO UART registers, PCIe BAR probe, coherent DMA descriptor ring, I2C/SPI/I2S drivers | `make -C embedded-c/device_drivers test` | `make -C embedded-c/device_drivers run FILE=mmio_uart_driver` | [`embedded-c/device_drivers/README.md`](./embedded-c/device_drivers/README.md) |
| **Parsers & Drivers** | TLV stream decoder, serializer, camera ISP bitfields | `make -C embedded-c/parsers_drivers test` | `make -C embedded-c/parsers_drivers run FILE=tlv_parser` | [`embedded-c/parsers_drivers/README.md`](./embedded-c/parsers_drivers/README.md) |
| **Pointers & Algorithms** | Double-pointers, singly linked list, in-place string strip | `make -C embedded-c/pointers_algorithms test` | `make -C embedded-c/pointers_algorithms run FILE=singly_linked_list` | [`embedded-c/pointers_algorithms/README.md`](./embedded-c/pointers_algorithms/README.md) |
| **Ring Buffers & DSP** | Power-of-two circular queue, moving average filter | `make -C embedded-c/ring_buffers test` | `make -C embedded-c/ring_buffers run FILE=power_of_two_ring_buffer` | [`embedded-c/ring_buffers/README.md`](./embedded-c/ring_buffers/README.md) |
| **Operating Systems Shell** | Foreground/background jobs, double-fork watcher | `make -C os-systems/simpleshell test` | `make -C os-systems/simpleshell run` | [`os-systems/simpleshell/README.md`](./os-systems/simpleshell/README.md) |
| **Process Lab** | Multi-process unit testing harness, fault isolation | `make -C os-systems/process_lab test` | `make -C os-systems/process_lab run` | [`os-systems/process_lab/README.md`](./os-systems/process_lab/README.md) |
| **LeetCode C Solutions** | 26 solved problems with standalone test runners | `make -C leetcode test` | `make -C leetcode run ID=0001` *(or `make -C leetcode 0001-two-sum`)* | [`leetcode/README.md`](./leetcode/README.md) |

---

## Repository Overview

```text
sourcecode/
├── embedded-c/                # Embedded C, firmware & POSIX concurrency modules
│   ├── bit_manipulation/      # Bitwise macros, bitfield popcounts & register overlays
│   ├── ble_tracker/           # BLE packet bit extraction, ISR ring buffer, debouncer, thread coordinator
│   ├── concurrency/           # POSIX threads mutexes, barriers, bounded queues & condition variables
│   │   └── condition_variables/ # Counting semaphore patterns & multi-stage service queues
│   ├── device_drivers/        # Memory-Mapped I/O, PCIe configuration, DMA rings, RTOS pipeline, I2C/SPI/I2S bus drivers
│   ├── parsers_drivers/       # Stateful TLV packet parser, byte serialization & camera ISP pipeline
│   ├── pointers_algorithms/   # Double pointers, dynamic singly linked list & string compaction
│   ├── ring_buffers/          # Power-of-two circular buffers & sliding-window average filters
│   ├── endian_practice/       # 16/32/64-bit endianness swap & detection library + unit tests
│   └── atoi_itoa_practice/    # Custom my_atoi / my_itoa implementation + unit tests
├── os-systems/                # Operating Systems & systems programming labs (university self-study)
│   ├── simpleshell/           # UNIX shell with foreground/background job execution & process control
│   └── process_lab/           # Multi-process unit test harness with fault isolation & timeouts
├── swift-ios/                 # Swift & iOS UIKit projects
│   ├── SwiftPractice/         # Swift OOP & class inheritance exercises
│   └── SwiftUiKitPractice/    # iOS UIKit & Storyboard practice
├── leetcode/                  # 26 accepted LeetCode C solutions with standalone main() test harnesses
├── vscode-template/           # Portable installer script to sync VS Code settings & snippets
├── scripts/git-hooks/         # Tracked Git hooks (pre-commit styling/privacy, commit-msg validation)
├── .vscode/                   # Tracked repository IDE configuration, tasks, and snippets
├── .clang-format              # Allman brace style and strict spacing rules
├── .gitmessage                # Standardized Git commit message template
└── Makefile                   # Root orchestration Makefile
```

---

## What Is Built & Tested Across the Repository

### 1. Embedded C & Firmware Modules ([`./embedded-c/`](./embedded-c/concurrency/README.md))
- **BLE Proximity Tracker Firmware ([`./embedded-c/ble_tracker/`](./embedded-c/ble_tracker/README.md))**:
  - `ble_payload_parser.c` *(Implemented — Passing)* — Unpacks unaligned fields (224-bit P-224 public key, 2-bit device state, 6-bit battery level, 16-bit little-endian checksum) from a raw 31-byte BLE advertisement packet.
  - `isr_ring_buffer.c` *(Implemented — Passing)* — Statically allocated, lock-free power-of-two circular buffer (`BUFFER_SIZE = 16`) queuing UWB ranging packets from an ISR to an RTOS thread; tested across fill, overfill, drain, underflow, and wrap-around.
  - `motion_alert_debouncer.c` *(Implemented — Passing)* — Stateful rolling-window accelerometer debouncer triggering a chime only when 5 motion interrupts arrive within a 60-second window.
  - `secure_broadcast_coordinator.c` *(Implemented — Passing)* — Multi-threaded crypto key rotator, motion sensor signaler, and BLE broadcaster using `pthread_mutex_t` and `pthread_cond_t` to eliminate torn reads.
- **POSIX Threads Concurrency ([`./embedded-c/concurrency/`](./embedded-c/concurrency/README.md))**:
  - `mutex_shared_counter.c`, `mutex_channel_lock_ordering.c` *(Implemented — Passing)* — Shared accumulator mutual exclusion and deadlock-free dual-channel (`redLock`/`blueLock`) lock ordering.
  - `condvar_thread_join.c`, `condvar_barrier.c`, `bounded_buffer_producer_consumer.c` *(Implemented — Passing)* — Condition variable `thread_join` simulation, N-thread rendezvous barrier, and 5-producer/5-consumer circular bounded buffer.
  - `custom_semaphore.c`, `reader_writer_lock.c`, `atomic_spinlock.c` *(Implemented — Passing)* — Custom counting semaphore (12 workers), reader-writer lock with writer preference (8 readers, 3 writers), and C11 atomic spinlock (8 threads, 800,000 increments) tested with zero race conditions.
  - `dining_philosophers.c` *(Scaffolded — Optional)* — Dijkstra resource hierarchy dining philosophers deadlock prevention.
  - `condition_variables/bounded_critical_section.c`, `condition_variables/two_stage_service_rooms.c` *(Implemented — Passing)* — Bounded critical sections (`max = 3`) and two-stage customer rooms with bounded waiting room and balking.
  - `condition_variables/ordered_thread_entry.c`, `stack_bounded_buffer.c`, `tunnel_traffic_control.c` *(Lab Starter Templates)* — Unimplemented starter templates preserved for additional practice.
- **Bit Manipulation & Hardware Registers ([`./embedded-c/bit_manipulation/`](./embedded-c/bit_manipulation/README.md))**:
  - `bit_basics.c` *(Implemented — Passing)* — `set_bit`, `clear_bit`, `toggle_bit`, `check_bit`, and `is_power_of_two` with MSB/LSB and out-of-range guards.
  - `bitfield_operations.c` *(Implemented — Passing)* — Fast-path Hamming weight (`count_set_bits`), `extract_bitfield`, and read-modify-write `insert_bitfield`.
  - `register_union_overlay.c` *(Implemented — Passing)* — 32-bit hardware register overlay using a C `union` and 8-bit bitfield slices.
- **Circular Buffers & Filtering ([`./embedded-c/ring_buffers/`](./embedded-c/ring_buffers/README.md))**:
  - `power_of_two_ring_buffer.c` *(Implemented — Passing)* — Power-of-two circular queue (`BUFFER_SIZE = 8`) using bitmask index wrap-around (`(index + 1) & BUFFER_MASK`).
  - `moving_average_filter.c` *(Implemented — Passing)* — O(1) running-sum sliding-window moving average filter (`WINDOW_SIZE = 4`).
  - `counted_ring_buffer.c` *(Practice Harness / Starter Template)* — Struct-encapsulated byte ring buffer unit-test harness kept as a starter template.
- **Protocol Parsers & Drivers ([`./embedded-c/parsers_drivers/`](./embedded-c/parsers_drivers/README.md))**:
  - `tlv_parser.c` *(Implemented — Passing)* — Defensive `[Tag][Length][Value]` binary stream parser rejecting truncated headers and buffer-overflow lengths.
  - `uint32_serializer.c` *(Implemented — Passing)* — `uint32_t` to `uint8_t[4]` byte-stream serialization and deserialization.
  - `camera_isp_driver.c` *(Part 1 Implemented; Parts 2–5 Practice Stubs)* — Implements and passes Part 1 (`update_camera_register` read-modify-write on `bits [15:12]`); Parts 2–5 are practice stubs.
- **Pointers, Strings & Algorithms ([`./embedded-c/pointers_algorithms/`](./embedded-c/pointers_algorithms/README.md))**:
  - `double_pointer.c` *(Implemented — Passing)* — Heap buffer output parameters (`uint8_t **`), linked-list head insertion (`Node **`), and delimiter tokenizer (`char **`).
  - `remove_char.c` *(Implemented — Passing)* — Two-pointer O(N) in-place string character removal.
  - `singly_linked_list.c` *(Implemented — Passing)* — Heap-allocated singly linked list construction and traversal.
  - `binary_search.c` *(Practice Harness / Starter Template)* — 15-case boundary test suite for iterative binary search.
- **Standalone C Libraries with Unit Test Suites**:
  - [`./embedded-c/endian_practice/`](./embedded-c/endian_practice) *(Implemented — Passing)* — 16/32/64-bit endianness byte swapping, runtime architecture endianness detection, and 32-bit stream conversion.
  - [`./embedded-c/atoi_itoa_practice/`](./embedded-c/atoi_itoa_practice) *(Implemented — Passing)* — Custom `my_atoi` and `my_itoa` implementations with assertion suite.

### 2. Operating Systems Labs ([`./os-systems/`](./os-systems/simpleshell/README.md))
- **`simpleshell/`** *(Implemented — Passing)* — UNIX command shell implementing command parsing, foreground command execution (`fork`, `execlp`, `wait`, `WEXITSTATUS`), and background command execution (`BG` prefix) with double-`fork` completion watcher.

### 3. Swift & iOS UIKit ([`./swift-ios/`](./swift-ios))
- **`SwiftPractice/`** — Swift object-oriented programming and class inheritance exercises.
- **`SwiftUiKitPractice/`** — iOS UIKit view controllers (`ViewController`, `LibraryViewController`) and Storyboard navigation.

### 4. LeetCode C Solutions ([`./leetcode/`](./leetcode/README.md))
- **26 Accepted Problems in C (2016 – 2026) with Standalone `main()` Test Runners** — Bit manipulation (`#190 Reverse Bits`, `#191 Number of 1 Bits`, `#338 Counting Bits`, `#693 Binary Number with Alternating Bits`), linked lists (`#2 Add Two Numbers`, `#83 Remove Duplicates`, `#92 Reverse Linked List II`, `#206 Reverse Linked List`, `#328 Odd Even Linked List`), trees (`#98 Validate BST`, `#104 Max Depth`, `#112 Path Sum`, `#226 Invert Binary Tree`, `#235 LCA of BST`), arrays/sliding window (`#1 Two Sum`, `#3 Longest Substring`, `#26 Remove Duplicates`, `#28 Find First Occurrence`, `#53 Maximum Subarray`, `#283 Move Zeroes`, `#346 Moving Average from Data Stream`, `#485 Max Consecutive Ones`, `#1762 Buildings With an Ocean View`), and concurrency (`#1114 Print in Order`, `#1115 Print FooBar Alternately`).

---

## Code Style & Portable VS Code Setup (`.vscode` & `.clang-format`)

All C and header files follow a strict `.clang-format` configuration:
- **Brace Style**: Allman (`BreakBeforeBraces: Allman` — opening `{` on a new line, mandatory braces on all control blocks with `InsertBraces: true`).
- **Control Statements**: No space before opening parentheses or inside outer parentheses (`SpaceBeforeParens: Never` — `if(x)`, `for(...)`, `while(x)`).

### Portable Setup for Any Machine
To import these editor preferences, C snippets, and Git configurations into any fresh VS Code installation on macOS or Linux:

```bash
# Run the automated installer script
./vscode-template/install.sh
```

The script automatically:
1. Detects your OS configuration directory:
   - **macOS**: `~/Library/Application Support/Code/User/`
   - **Linux**: `~/.config/Code/User/`
2. Creates `.bak` backups of any existing settings or snippets.
3. Installs `.vscode/settings.json` and `.vscode/c.code-snippets` (covering `if`, `ifelse`, `elif`, `else`, `for`, `while`, `do`, `switch`, `func`, `struct`, `main` with Allman braces).
4. Configures `git config commit.template .gitmessage` and installs the privacy & styling Git hooks.

---

## Git Commit Workflow & Message Template

This repository follows a clean, professional commit history without noisy prefixes or company references:
- **Template File**: [`.gitmessage`](./.gitmessage)
- **Subject Line**: Imperative mood, <= 72 characters (e.g., `Implement power-of-two circular buffer with bitmask indexing`, `Solve LeetCode 83. Remove Duplicates from Sorted List (C)`).
- **Body Sections**: What & Why technical rationale, automated test verification confirmation, and academic attribution.
- **Git Hooks**:
  - `pre-commit`: Automatically applies `.clang-format` and blocks any accidental credential or company token leaks.
  - `commit-msg`: Verifies subject length <= 72 characters and enforces privacy keyword boundaries.

---

## Credits, Copyright & Academic Attribution

Exercises and lab projects across this repository draw from **multiple university operating systems, embedded systems, and computer science courses, open academic lab archives, and classic systems textbooks** referenced for independent self-study. Where starter templates, Makefiles, or problem specifications originate from a university course or open educational archive, copyright belongs to the respective faculty authors and institutions, and each module credits its specific source:

1. **Rose-Hulman Institute of Technology — CSSE 332: Operating Systems** *(used in [`./os-systems/simpleshell/`](./os-systems/simpleshell) and [`./embedded-c/concurrency/`](./embedded-c/concurrency/README.md))*
   - **Course Schedule & Materials**: [https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/](https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/)
   - **Official Course Lab Archive**: [https://github.com/rhit-csse332/csse332-labs-archive](https://github.com/rhit-csse332/csse332-labs-archive)
   - **Faculty Authors & Contributors**: Mohammad Noureddine, Michael Hewner, Sid Stamm, Robert Williamson, Sriram Mohan, and the Rose-Hulman Department of Computer Science and Software Engineering.
2. **Jacobs University Bremen — Computer Networks and Distributed Systems (CAOS) Lab** *(referenced in [`./os-systems/simpleshell/`](./os-systems/simpleshell) and [`./embedded-c/concurrency/`](./embedded-c/concurrency/README.md))*
   - Original `simpleshell` assignment concept (`http://cnds.eecs.jacobs-university.de/courses/caoslab-2007/`) and POSIX semaphore exercises (Dr. Kinga Lipskoch).
3. **Classic Systems & C References**
   - *Operating Systems: Three Easy Pieces (OSTEP)* by Remzi H. Arpaci-Dusseau and Andrea C. Arpaci-Dusseau (University of Wisconsin-Madison) for condition-variable producer-consumer patterns, and *The C Programming Language (K&R)* for foundational string/number conversion (`atoi`/`itoa`) exercises.
