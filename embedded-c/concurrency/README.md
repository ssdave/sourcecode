# POSIX Threads (`pthreads`) Synchronization & Concurrency

Hands-on POSIX threads (`pthread_mutex_t`, `pthread_cond_t`) synchronization exercises covering mutual exclusion, multi-resource lock ordering, reusable barriers, bounded producer-consumer queues, and multi-stage condition variable state machines.

## Academic Attribution & Copyright

All problem specifications, synchronization scenarios, and starter test harnesses in `concurrency/` and `concurrency/condition_variables/` originate from:

- **Course**: **Rose-Hulman Institute of Technology — CSSE 332: Operating Systems**
  - Course Schedule & Notes: [https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/](https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/)
  - Official Lab Archive: [https://github.com/rhit-csse332/csse332-labs-archive](https://github.com/rhit-csse332/csse332-labs-archive)
- **Faculty Authors & Contributors**: Mohammad Noureddine, Michael Hewner, Sid Stamm, Robert Williamson, Sriram Mohan, and the Rose-Hulman CSSE department.
- **Purpose**: Completed independently for operating systems self-study and embedded/systems concurrency practice.

---

## Mapping to CSSE 332 Labs & Activities

| File | Status | Original CSSE 332 Source | What is Built | What is Tested |
| :--- | :--- | :--- | :--- | :--- |
| [`mutex_shared_counter.c`](./mutex_shared_counter.c) | **Implemented** | `threads_basics/basic_mutex/add_a_lot.c` | Mutex-protected shared accumulator (`pthread_mutex_lock` / `unlock`). | Spawns 10 threads each incrementing `counter_total` 10,000 times; verifies zero lost updates (`counter_total == 100000`). |
| [`mutex_channel_lock_ordering.c`](./mutex_channel_lock_ordering.c) | **Implemented** | `threads_basics/basic_mutex/red_blue_purple.c` | Fine-grained dual-channel mutex locking (`redLock`, `blueLock`) with consistent lock ordering in `purpleCommand` to prevent deadlock. | Runs 3 concurrent worker threads issuing `Red`, `Blue`, `White`, and `Purple` commands; verifies non-overlapping `Start`/`End` channel intervals without deadlock. |
| [`condvar_thread_join.c`](./condvar_thread_join.c) | **Implemented** | `activities/condvar_intro/condwait.c` | Custom `thread_join()` and `thread_exit()` implementation using `pthread_cond_t` and a boolean state predicate (`child_done`). | Verifies parent-child join synchronization even if the child finishes before `thread_join()` is called. |
| [`condvar_barrier.c`](./condvar_barrier.c) | **Implemented** | `activities/condvar_intro/barrier.c` | N-thread rendezvous barrier (`barrier_wait`) using `pthread_cond_broadcast` when `num_threads_at_barrier == NUM_THREADS`. | Launches 5 threads with randomized sleep delays; verifies no thread proceeds past the barrier until all 5 arrive. |
| [`bounded_buffer_producer_consumer.c`](./bounded_buffer_producer_consumer.c) | **Implemented** | `activities/prodcons/03_prodcons.c` | Multi-producer, multi-consumer circular bounded buffer (`MAX = 10`) using two condition variables (`empty` and `full`) and `while` predicate loops. | Runs 5 producer threads and 5 consumer threads for 100 iterations each without buffer overflow, underflow, or Mesa-semantics race conditions. |
| [`condition_variables/`](./condition_variables/) | **Sub-Module** | `condvar_basics/` | Advanced multi-stage condition variable coordination problems. | See [condition_variables/README.md](./condition_variables/README.md) for details. |

## How to Build & Run

### From repository root
```bash
make -C embedded-c/concurrency               # Build all binaries (including condition_variables)
make -C embedded-c/concurrency test          # Run all concurrency test suites
make -C embedded-c/concurrency clean         # Clean artifacts
```

### From within this folder
```bash
make                                         # Build all binaries
make test                                    # Run all test suites
make run FILE=mutex_shared_counter           # Run specific binary
make run FILE=mutex_channel_lock_ordering
make run FILE=condvar_thread_join
make run FILE=condvar_barrier
make run FILE=bounded_buffer_producer_consumer
make clean                                   # Clean artifacts
```

### Compiling a single file directly
```bash
clang -Wall -Wextra -g -pthread mutex_shared_counter.c -o mutex_shared_counter
./mutex_shared_counter
```
