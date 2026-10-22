# UNIX Pipes Deep Dive (`pipes_deep_dive`)

Advanced Inter-Process Communication (IPC), concurrency mechanics, and stream-processing architectures using POSIX UNIX pipes (`pipe()`), process duplication (`fork()`), descriptor lifecycle management (`close()`, `dup2()`), signal resilience (`SIGPIPE`, `sigaction()`), and child coordination (`waitpid()`).

---

## Academic Attribution & Historical Context

- **Foundational Concepts & Architecture**: Conceived by **M. Douglas McIlroy** (inventor of UNIX pipes, Bell Labs / Dartmouth College) as an elegant demonstration of concurrent stream processing, pipeline filters, and coroutines.
- **Academic Context & Literature**:
  - **MIT 6.S081 / 6.828: Operating System Engineering** (Lab 1: `primes`)
  - **Rose-Hulman Institute of Technology: CSSE 332** (Operating Systems — IPC & Pipes Exercises)
  - **W. Richard Stevens & Stephen A. Rago**: *Advanced Programming in the UNIX Environment (APUE)*, Chapter 15: Interprocess Communication
  - **Michael Kerrisk**: *The Linux Programming Interface (TLPI)*, Chapter 44: Pipes and FIFOs
- **Usage**: Retained strictly for independent educational self-study. All solution implementations are authored authentically by the student for learning and systems engineering interview preparation.

---

## Pedagogical Goals & Core Concepts

1. **Kernel Descriptor Reference Counting**:
   Pipes are kernel-allocated circular ring buffers referenced by file descriptors. An anonymous pipe stays open as long as at least one write descriptor remains open across all running processes. Closing write ends in every holding process is required to deliver `EOF` (`read() == 0`) to downstream readers.
2. **The "Phantom Writer" Deadlock**:
   When a parent forks a child writer, both parent and child inherit open copies of `WRITE_FD`. If the parent fails to close its inherited `WRITE_FD`, the kernel writer reference count never reaches zero when the child terminates. Readers block forever waiting for EOF, causing a silent system hang.
3. **Broken Pipe Mechanics (`SIGPIPE` & `EPIPE`)**:
   Writing to a pipe with zero active readers triggers kernel delivery of `SIGPIPE`. By default, `SIGPIPE` abruptly terminates the writer without producing a core dump. Defensive systems programming requires either handling `SIGPIPE` or configuring `SIG_IGN` so that `write()` safely returns `-1` with `errno == EPIPE`.
4. **`PIPE_BUF` Atomicity & Multi-Writer Concurrency**:
   POSIX guarantees that `write()` payloads of size $\le \text{PIPE\_BUF}$ (at least 512 bytes in POSIX, typically 512 or 4096 bytes depending on the OS) are executed atomically and will never be interleaved with concurrent writes from other processes.
5. **Byte Stream Semantics vs. C Strings**:
   Pipes carry raw, unstructured byte streams with no notion of string boundaries, message packets, or null terminators (`'\0'`). Readers must explicitly null-terminate buffers to prevent out-of-bounds memory over-reads.

---

## Curriculum Exercise Index

| File | Challenge Topic | Description / Architectural Focus |
| :--- | :--- | :--- |
| [`prime_sieve_pipeline.c`](./prime_sieve_pipeline.c) | Dynamic Pipeline IPC | Implements the Doug McIlroy Prime Sieve of Eratosthenes pipeline across dynamically chained processes using `fork()`, `pipe()`, `read()`, `write()`, and `waitpid()`. |
| [`phantom_writer.c`](./phantom_writer.c) | Deadlock Prevention | Investigates kernel descriptor reference counting and eliminates the "Phantom Writer" deadlock by enforcing strict immediate closure of inherited descriptors post-`fork()`. Protected by an `alarm()` watchdog. |
| [`broken_pipe.c`](./broken_pipe.c) | Signal & Error Recovery | Implements defensive signal handling for `SIGPIPE` using `sigaction()` and ensures robust recovery from `errno == EPIPE` when downstream readers close prematurely. |
| [`multi_writer_collector.c`](./multi_writer_collector.c) | Atomic Multi-Writer IPC | Coordinates multiple concurrent producer processes writing structured telemetry into a single collector pipe, leveraging the POSIX `PIPE_BUF` atomicity guarantee and stream framing. |
| [`stream_null_terminator.c`](./stream_null_terminator.c) | Stream Framing & Safety | Demystifies pipe byte stream mechanics, contrasting raw octet streams against C null-terminated strings, and implements safe buffer termination and delimiter-based line parsing. |

---

## Coding Standards & Conventions

All exercises in this module follow strict professional systems programming standards:
- **Descriptive Descriptor Macros**: Raw magic numbers `0` and `1` are forbidden. Every file defines and uses:
  ```c
  #define READ_FD  0
  #define WRITE_FD 1
  ```
- **Brace Style**: Strict Allman style (`{` on a new line for functions, conditionals, and loops).
- **Conditional Formatting**: Compact parentheses with zero outer space (`if(x)`, `for(init; cond; inc)`, `while(x)`).
- **Mandatory Braces**: All loop and conditional blocks are wrapped in braces, even single-line statements.
- **Strict Skeleton-Only Rule**: Starter templates provide empty function signatures with `(void)param;` and `/* TODO: Author Implementation */` pseudocode hints. 100% of authentic logic is implemented by the author.

---

## Quick Start: Building & Running

### Build all exercises
```bash
make -C os-systems/pipes_deep_dive all
```

### Run automated test suite
```bash
make -C os-systems/pipes_deep_dive test
```

### Run individual exercises
```bash
make -C os-systems/pipes_deep_dive run-primes       # Prime Sieve Pipeline
make -C os-systems/pipes_deep_dive run-phantom      # Phantom Writer Deadlock Demo
make -C os-systems/pipes_deep_dive run-broken-pipe  # Broken Pipe (SIGPIPE / EPIPE)
make -C os-systems/pipes_deep_dive run-multi-writer # Multi-Writer Atomic Collector
make -C os-systems/pipes_deep_dive run-stream       # Stream Framing & Null-Terminator
```

### Clean build artifacts
```bash
make -C os-systems/pipes_deep_dive clean
```

### Run entire workspace test suite from root
```bash
make test
```
