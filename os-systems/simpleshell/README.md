# Simple Unix Shell (`simpleshell`)

A lightweight POSIX command-line interpreter demonstrating Unix process control, command tokenization, foreground process execution (`fork` / `execvp` / `waitpid`), background task management (`BG`), and asynchronous signal-driven zombie reaping (`SIGCHLD`).

## Academic Attribution & Copyright

- **Course**: **Rose-Hulman Institute of Technology — CSSE 332: Operating Systems**
  - Course Schedule: [https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/](https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/)
  - Original Lab Name: `simpleshell` ([Lab Specification](./lab_handout.md))
- **Authors**: RHIT CSSE 332 Faculty (M. Noureddine, M. Hewner, S. Stamm, R. Williamson, S. Mohan).
- **Purpose**: Completed independently for Operating Systems self-study.

## Module Files

| File | Focus Area | Description / Implementation Status |
| :--- | :--- | :--- |
| `simpleshell.c` | Core Shell Implementation | Implements command prompt, string parsing, foreground command execution with `fork()` and `execvp()`, background process spawning (`BG <command>`), double-fork background process watcher, and `exit` builtin. |
| `donothing.c` | Test Utility Binary | Auxiliary test executable that sleeps for 5 seconds and exits, used to verify background process execution without blocking the main shell prompt. |
| `lab_handout.md` | Lab Specification | Course assignment specification outlining steps 1 through 5. |
| `lab_grading_rubric.md` | Testing Checklist | Step-by-step grading rubric and process state transition checks. |

## Progress Checklist

- [x] **Step 1**: Basic foreground command execution with `fork()` and `execvp()`.
- [x] **Step 2**: Background execution (`BG <command>`) without blocking the prompt.
- [x] **Step 3**: Double-fork background watcher process printing completion status.
- [ ] **Step 4**: Signal handler (`signal(SIGCHLD, ...)` / `waitpid(-1, ..., WNOHANG)`) to prevent zombie accumulation.
- [ ] **Step 5**: Command line argument parsing and built-in commands (`cd`).

## How to Build & Run

### From repository root
```bash
make -C os-systems/simpleshell           # Build simpleshell and donothing
make -C os-systems/simpleshell test      # Run smoke test
make -C os-systems/simpleshell clean     # Clean artifacts
```

### From within this folder
```bash
make                                     # Build simpleshell and donothing
make test                                # Run smoke test
make run                                 # Launch interactive shell
make clean                               # Clean artifacts
```

### Running the shell interactively
```bash
./simpleshell
simpleshell> ls -la
simpleshell> BG ./donothing
simpleshell> exit
```
