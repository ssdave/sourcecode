# Operating Systems & Systems Programming Labs (`os-systems`)

Operating Systems and systems programming labs completed for independent self-study, referencing **university operating systems curricula, open academic lab archives, and systems programming courses**.

## Academic Attribution & Copyright

Lab specifications, starter Makefiles, and problem templates included in this directory are copyright of their respective university faculty authors and institutions, and are used strictly for independent self-study. Specific attributions for the modules currently in this folder:

1. **Rose-Hulman Institute of Technology — CSSE 332: Operating Systems** *(used in [`./simpleshell/`](./simpleshell) and [`./process_lab/`](./process_lab))*
   - **Course Schedule & Lab Handouts**: [https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/](https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/)
   - **Official Lab Repository**: [https://github.com/rhit-csse332/csse332-labs-archive](https://github.com/rhit-csse332/csse332-labs-archive)
   - **Faculty Authors**: Mohammad Noureddine, Michael Hewner, Sid Stamm, Robert Williamson, Sriram Mohan, and contributors at Rose-Hulman Institute of Technology.
2. **Jacobs University Bremen — CAOS Lab**
   - Original `simpleshell` assignment concept (`http://cnds.eecs.jacobs-university.de/courses/caoslab-2007/`), adapted by Rose-Hulman CSSE 332.

---

## Labs

### 1. [`./simpleshell/`](./simpleshell)
- **Original Source**: `simpleshell/` ([Lab Specification](https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/lab01/))
- **What is being built**:
  - A UNIX command-line shell ([`simpleshell.c`](./simpleshell/simpleshell.c)) that parses user commands with optional arguments and executes them via POSIX process management primitives (`fork()`, `execlp()`, `wait()`, `WEXITSTATUS()`).
  - Supports both blocking foreground execution and non-blocking background execution prefixed with `BG` (e.g., `BG./donothing`), including a double-`fork` background watcher that prints `"Background command finished %d\n"` upon completion.
- **What is being tested**:
  - Tested against [`donothing.c`](./simpleshell/donothing.c) (a 5-second sleep helper binary that prints `argv`) and standard UNIX utilities (`ls`, `echo`, `ps`) across foreground execution, exit status reporting, and background process spawning.

### 2. [`./process_lab/`](./process_lab)
- **Original Source**: `process_lab/` ([Lab Specification](https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/processlab/))
- **What is being built**:
  - A parallelized C unit testing harness ([`process_test_runner.c`](./process_lab/process_test_runner.c)) that spawns isolated child processes for each test case via `fork()`.
  - Child tests report outcomes via exit statuses and inter-process pipes (`pipe()`), protecting the main runner against segfault crashes and enforcing a 3-second hard timeout.
- **What is being tested**:
  - Parallel execution of unit tests (`test1` through `test5`), verifying copy-on-write setup state, failure detection, segmentation fault crash handling, and infinite loop timeout handling.
