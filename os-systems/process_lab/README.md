# Multi-Process Unit Testing Framework (`process_lab`)

A parallelized C unit testing harness ([`process_test_runner.c`](./process_test_runner.c)) using POSIX process management primitives (`fork`, `waitpid`, `pipe`, `kill`), where test cases execute concurrently in isolated child processes to detect crashes and enforce execution timeouts.

---

## Academic Attribution & Copyright

Derived from **Rose-Hulman Institute of Technology — CSSE 332: Operating Systems** (Lab 03: Process Lab).
- **Course URL**: [https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/processlab/](https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/processlab/)
- **Official Lab Repository**: [https://github.com/rhit-csse332/csse332-labs-archive](https://github.com/rhit-csse332/csse332-labs-archive)
- **Faculty Authors**: Mohammad Noureddine, Michael Hewner, Sid Stamm, and contributors at Rose-Hulman Institute of Technology.
- **Usage**: Retained strictly for independent educational self-study.

---

## Lab Architecture & Steps

| Step | Objective | Implementation Focus |
| :--- | :--- | :--- |
| **Step 1** | **Parallel Test Execution** | Concurrent `fork()` loop for all test cases, with `waitpid(-1, &status, 0)` reaping in parent. |
| **Step 2** | **Exit Status Translation** | Child translates test result string into exit status (`0` for pass, `1` for fail); parent inspects `WEXITSTATUS`. |
| **Step 3** | **Shared Memory Setup** | Run expensive `setup()` once in parent before `fork()` to leverage copy-on-write memory semantics. |
| **Step 4** | **Fault Isolation (Crashes)** | Detect segfaults / aborts in child via `WIFSIGNALED` without crashing the main test runner. |
| **Step 5** | **Execution Timeouts** | Enforce a 3-second hard timeout using `alarm()` or parent timer signals (`SIGKILL`/`SIGALRM`) for hung tests. |
| **Step 6** | **IPC via Pipes** | Inter-process pipes (`pipe()`) to stream detailed error strings back to the parent runner. |

---

## Quick Start: Building & Running

```bash
# Build the test harness
make

# Run the test suite
make test

# Run directly
make run

# Clean build artifacts
make clean
```
