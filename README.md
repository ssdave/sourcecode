# Systems Programming, Embedded C & iOS Practice

Personal repository for hands-on systems programming, embedded C/firmware development, POSIX threads (`pthreads`) concurrency, operating systems internals, and Swift/UIKit engineering practice.

> **Note on Workflow & AI Usage**: All core problem solutions, bit-level algorithms, concurrency synchronization logic, and operating systems exercises in this repository are written hands-on for systems learning and technical interview preparation. AI tools are used where helpful to standardize code formatting (`.clang-format`), structure documentation and explanatory comments, and scaffold unit test harnesses.

---

## Repository Overview

```text
sourcecode/
├── embedded-c/                # Embedded C, firmware & POSIX concurrency modules
│   ├── ble_tracker/           # BLE packet bit-extraction, ISR ring buffer, debouncer, thread coordinator
│   ├── concurrency/           # POSIX threads mutexes, barriers, condition variables & state machines
│   ├── bit_manipulation/      # Bitwise macros, bitfields & hardware status register unions
│   ├── ring_buffers/          # Power-of-two circular buffers & moving average filters
│   ├── parsers_drivers/       # Stateful TLV packet parser, byte serialization & camera ISP pipeline
│   ├── pointers_algorithms/   # In-place double-pointer list ops, string manipulation & binary search
│   ├── endian_practice/       # 16/32/64-bit endianness swap & detection library + unit tests
│   └── atoi_itoa_practice/    # Custom my_atoi / my_itoa implementation + unit tests
├── os-systems/                # Operating Systems & systems programming labs (university self-study)
│   └── simpleshell/           # UNIX shell with foreground/background job execution & process control
├── swift-ios/                 # Swift & iOS UIKit projects
│   ├── SwiftPractice/         # Swift OOP & class inheritance exercises
│   └── SwiftUiKitPractice/    # iOS UIKit & Storyboard practice
└── leetcode/                  # 26 accepted LeetCode C solutions with standalone main() test harnesses
```

---

## What Is Built & Tested Across the Repository

### 1. Embedded C & Firmware Modules ([`./embedded-c/`](./embedded-c/README.md))
- **BLE Proximity Tracker Firmware ([`./embedded-c/ble_tracker/`](./embedded-c/ble_tracker))**:
  - `Problem1 - BLE Payload Bit Extraction.c` *(Implemented — Passing)* — Unpacks unaligned fields (224-bit P-224 public key, 2-bit device state, 6-bit battery level, 16-bit little-endian checksum) from a raw 31-byte BLE advertisement packet.
  - `Problem2 - The Interrupt-Safe Ring Buffer.c` *(Implemented — Passing)* — Statically allocated, lock-free power-of-two circular buffer (`BUFFER_SIZE = 16`) queuing UWB ranging packets from an ISR to an RTOS thread; tested across fill, overfill, drain, underflow, and wrap-around.
  - `Problem 3 - The Anti-Stalking Debouncer.c` *(Implemented — Passing)* — Stateful rolling-window accelerometer debouncer triggering a chime only when 5 motion interrupts arrive within a 60-second window.
  - `Problem 4 - The Secure Broadcast Coordinator.c` *(Implemented — Passing)* — Multi-threaded crypto key rotator, motion sensor signaler, and BLE broadcaster using `pthread_mutex_t` and `pthread_cond_t` to eliminate torn reads.
- **POSIX Threads Concurrency ([`./embedded-c/concurrency/`](./embedded-c/concurrency/README.md))**:
  - `basicMutex1.c`, `basicMutex2.c` *(Implemented — Passing)* — Shared accumulator mutual exclusion and deadlock-free dual-channel (`redLock`/`blueLock`) lock ordering.
  - `condwait.c`, `barrier.c`, `producerconsumer.c` *(Implemented — Passing)* — Condition variable `thread_join` simulation, N-thread rendezvous barrier, and 5-producer/5-consumer circular bounded buffer.
  - `conditionalvariables/max.c`, `conditionalvariables/rooms.c` *(Implemented — Passing)* — Bounded critical sections (`max = 3`) and two-stage customer rooms with bounded waiting room and balking.
  - `conditionalvariables/inorder.c`, `prodcons_condvar.c`, `tunnel.c` *(Lab Starter Templates)* — Unimplemented starter templates preserved for additional practice.
- **Bit Manipulation & Hardware Registers ([`./embedded-c/bit_manipulation/`](./embedded-c/bit_manipulation))**:
  - `bit_basics.c` *(Implemented — Passing)* — `set_bit`, `clear_bit`, `toggle_bit`, `check_bit`, and `is_power_of_two` with MSB/LSB and out-of-range guards.
  - `bit_level2.c` *(Implemented — Passing)* — Hamming weight (`count_set_bits`), `extract_bitfield`, and read-modify-write `insert_bitfield`.
  - `registers.c` *(Implemented — Passing)* — 32-bit hardware register overlay using a C `union` and 8-bit bitfield `struct`.
- **Circular Buffers & Filtering ([`./embedded-c/ring_buffers/`](./embedded-c/ring_buffers))**:
  - `ringbuffer.c` *(Implemented — Passing)* — Power-of-two circular queue (`BUFFER_SIZE = 8`) using bitmask index wrap-around (`(index + 1) & BUFFER_MASK`).
  - `movingaverage.c` *(Implemented — Passing)* — O(1) running-sum sliding-window moving average filter (`WINDOW_SIZE = 4`).
  - `ring_buffer.c` *(Practice Harness / Starter Template)* — Struct-encapsulated byte ring buffer unit-test harness kept as a starter template.
- **Protocol Parsers & Drivers ([`./embedded-c/parsers_drivers/`](./embedded-c/parsers_drivers))**:
  - `tlv_parser.c` *(Implemented — Passing)* — Defensive `[Tag][Length][Value]` binary stream parser rejecting truncated headers and buffer-overflow lengths.
  - `serialize.c` *(Implemented — Passing)* — `uint32_t` to `uint8_t[4]` byte-stream serialization and deserialization.
  - `camera_isp.c` *(Part 1 Implemented; Parts 2–5 Practice Stubs)* — Implements and passes Part 1 (`update_camera_register` read-modify-write on `bits [15:12]`); Parts 2–5 (lock-free VSYNC frame queue, overlapping `custom_memmove`, and non-blocking PMU rail state machine) are practice stubs.
- **Pointers, Strings & Algorithms ([`./embedded-c/pointers_algorithms/`](./embedded-c/pointers_algorithms))**:
  - `double_pointer.c` *(Implemented — Passing)* — Heap buffer output parameters (`uint8_t **`), linked-list head insertion (`Node **`), and delimiter tokenizer (`char **`).
  - `remove_char.c` *(Implemented — Passing)* — Two-pointer O(N) in-place string character removal.
  - `Hello.c` *(Implemented — Passing)* — Heap-allocated singly linked list construction and traversal.
  - `binarysearch.c` *(Practice Harness / Starter Template)* — 15-case boundary test suite for iterative binary search.
- **Standalone C Libraries with Unit Test Suites**:
  - [`./embedded-c/endian_practice/`](./embedded-c/endian_practice) *(Implemented — Passing)* — 16/32/64-bit endianness byte swapping, runtime architecture endianness detection, and 32-bit stream conversion.
  - [`./embedded-c/atoi_itoa_practice/`](./embedded-c/atoi_itoa_practice) *(Implemented — Passing)* — Custom `my_atoi` and `my_itoa` implementations with assertion suite.

### 2. Operating Systems Labs ([`./os-systems/`](./os-systems/README.md))
- **`simpleshell/`** *(Implemented — Passing)* — UNIX command shell implementing command parsing, foreground command execution (`fork`, `execlp`, `wait`, `WEXITSTATUS`), and background command execution (`BG` prefix) with double-`fork` completion watcher.

### 3. Swift & iOS UIKit ([`./swift-ios/`](./swift-ios))
- **`SwiftPractice/`** — Swift object-oriented programming and class inheritance exercises.
- **`SwiftUiKitPractice/`** — iOS UIKit view controllers (`ViewController`, `LibraryViewController`) and Storyboard navigation.

### 4. LeetCode C Solutions ([`./leetcode/`](./leetcode/README.md))
- **26 Accepted Problems in C (2016 – 2026) with Standalone `main()` Test Runners** — Bit manipulation (`#190 Reverse Bits`, `#191 Number of 1 Bits`, `#338 Counting Bits`, `#693 Binary Number with Alternating Bits`), linked lists (`#2 Add Two Numbers`, `#83 Remove Duplicates`, `#92 Reverse Linked List II`, `#206 Reverse Linked List`, `#328 Odd Even Linked List`), trees (`#98 Validate BST`, `#104 Max Depth`, `#112 Path Sum`, `#226 Invert Binary Tree`, `#235 LCA of BST`), arrays/sliding window (`#1 Two Sum`, `#3 Longest Substring`, `#26 Remove Duplicates`, `#28 Find First Occurrence`, `#53 Maximum Subarray`, `#283 Move Zeroes`, `#346 Moving Average from Data Stream`, `#485 Max Consecutive Ones`, `#1762 Buildings With an Ocean View`), and concurrency (`#1114 Print in Order`, `#1115 Print FooBar Alternately`).

---

## Building & Running

- **From Terminal (`make`)**:
  - Build all embedded C modules: `make -C embedded-c` (or run `make` inside any subfolder).
  - Build and run all 26 LeetCode C solutions & basic test cases: `make -C leetcode test` (or run a single problem via `make -C leetcode run ID=0001`).
  - Build `simpleshell`: `make -C os-systems/simpleshell`.
- **From VS Code**:
  - Press **`Cmd+Shift+B`** to compile the active `.c` file with `clang -Wall -g -lpthread`, or press **`F5`** to build and debug under `lldb`.

## Code Style & Formatting

All C and header files follow a consistent `.clang-format` configuration:
- **Brace Style**: Allman (`BreakBeforeBraces: Allman` — opening `{` on a new line, mandatory braces on all control blocks).
- **Control Statements**: No space before opening parentheses or inside outer parentheses (`SpaceBeforeParens: Never` — `if(x)`, `for(...)`, `while(x)`).

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
