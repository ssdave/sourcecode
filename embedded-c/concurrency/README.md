# POSIX Threads (`pthreads`) Synchronization & Concurrency

Hands-on POSIX threads (`pthread_mutex_t`, `pthread_cond_t`), atomic spinlocks (`stdatomic.h`), and synchronization exercises covering mutual exclusion, multi-resource lock ordering, reusable barriers, bounded producer-consumer queues, custom semaphores, reader-writer locks, dining philosophers, and multi-stage condition variable state machines.

## Academic Attribution & Literature Provenance

All problem specifications, synchronization scenarios, and starter test harnesses in `concurrency/` and `concurrency/condition_variables/` originate from classic systems literature and university curricula:

- **Rose-Hulman Institute of Technology — CSSE 332: Operating Systems**
  - Course Schedule & Notes: [https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/](https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/)
  - Official Lab Archive: [https://github.com/rhit-csse332/csse332-labs-archive](https://github.com/rhit-csse332/csse332-labs-archive)
  - Faculty Authors & Contributors: Mohammad Noureddine, Michael Hewner, Sid Stamm, Robert Williamson, Sriram Mohan, and the RHIT CSSE department.
- **Operating Systems: Three Easy Pieces (OSTEP)** — Remzi H. Arpaci-Dusseau & Andrea C. Arpaci-Dusseau
  - Chapter 28: Locks (Spinlocks and Hardware Primitives)
  - Chapter 31: Semaphores (Custom Semaphores, Reader-Writer Locks)
  - Chapter 32: Concurrency Bugs (Deadlock Prevention, Resource Hierarchy)
- **Classic Literature Foundations**:
  - Edsger W. Dijkstra (1965, 1971): *Cooperating Sequential Processes* & *Hierarchical Ordering of Sequential Processes* (EWD310).
  - P.J. Courtois, F. Heymans, D.L. Parnas (1971): *Concurrent Control with "Readers" and "Writers"*, Communications of the ACM.
  - Maurice Herlihy & Nir Shavit (2008): *The Art of Multiprocessor Programming* (Spinlocks & Atomic Primitives).
- **Purpose**: Completed independently for operating systems self-study and embedded/systems concurrency practice.

---

## Concurrency Curriculum & Status

| File | Status | Provenance & Source | What is Built | What is Tested |
| :--- | :--- | :--- | :--- | :--- |
| [`mutex_shared_counter.c`](./mutex_shared_counter.c) | **Implemented** | RHIT CSSE 332 (`threads_basics/basic_mutex/add_a_lot.c`) | Mutex-protected shared accumulator (`pthread_mutex_lock` / `unlock`). | Spawns 10 threads each incrementing `counter_total` 10,000 times; verifies zero lost updates (`counter_total == 100000`). |
| [`mutex_channel_lock_ordering.c`](./mutex_channel_lock_ordering.c) | **Implemented** | RHIT CSSE 332 (`threads_basics/basic_mutex/red_blue_purple.c`) | Fine-grained dual-channel mutex locking (`redLock`, `blueLock`) with consistent lock ordering in `purpleCommand` to prevent deadlock. | Runs 3 concurrent worker threads issuing `Red`, `Blue`, `White`, and `Purple` commands; verifies non-overlapping `Start`/`End` channel intervals without deadlock. |
| [`condvar_thread_join.c`](./condvar_thread_join.c) | **Implemented** | RHIT CSSE 332 (`activities/condvar_intro/condwait.c`) | Custom `thread_join()` and `thread_exit()` implementation using `pthread_cond_t` and a boolean state predicate (`child_done`). | Verifies parent-child join synchronization even if the child finishes before `thread_join()` is called. |
| [`condvar_barrier.c`](./condvar_barrier.c) | **Implemented** | RHIT CSSE 332 (`activities/condvar_intro/barrier.c`) | N-thread rendezvous barrier (`barrier_wait`) using `pthread_cond_broadcast` when `num_threads_at_barrier == NUM_THREADS`. | Launches 5 threads with randomized sleep delays; verifies no thread proceeds past the barrier until all 5 arrive. |
| [`bounded_buffer_producer_consumer.c`](./bounded_buffer_producer_consumer.c) | **Implemented** | RHIT CSSE 332 (`activities/prodcons/03_prodcons.c`) | Multi-producer, multi-consumer circular bounded buffer (`MAX = 10`) using two condition variables (`empty` and `full`) and `while` predicate loops. | Runs 5 producer threads and 5 consumer threads for 100 iterations each without buffer overflow, underflow, or Mesa-semantics race conditions. |
| [`custom_semaphore.c`](./custom_semaphore.c) | **Implemented** | Dijkstra (1965) / POSIX / OSTEP Ch 31 | Custom user-space counting semaphore (`custom_sem_t`) using mutex and condition variable. | Spawns 12 worker threads competing for an execution pool of capacity 3 across 20 iterations; verifies at most 3 threads active simultaneously with zero invariant violations. |
| [`reader_writer_lock.c`](./reader_writer_lock.c) | **Implemented** | Courtois et al. (1971) / OSTEP Ch 31 | Reader-Writer Lock with writer preference to prevent writer starvation. | Adversarial stress harness with 8 readers and 3 writers; verifies exclusive writer access, writer preference preemption, and zero starvation. |
| [`dining_philosophers.c`](./dining_philosophers.c) | **Scaffolded (Awaiting Author)** | Dijkstra (1971) / OSTEP Ch 32 | Deadlock-free dining philosophers using resource hierarchy (total lock ordering) on 5 fork mutexes. | Spawns 5 philosophers completing 20 meals each (100 total) with a 5-second watchdog timer catching deadlocks and verifying zero starvation. |
| [`atomic_spinlock.c`](./atomic_spinlock.c) | **Implemented** | Herlihy & Shavit (2008) / C11 `stdatomic.h` | User-space spinlock (`spinlock_t`) using C11 atomic test-and-set operations (`atomic_flag`). | Spawns 8 threads each performing 100,000 increments; verifies exactly 800,000 total with zero lost updates. |
| [`condition_variables/`](./condition_variables/) | **Sub-Module** | RHIT CSSE 332 (`condvar_basics/`) | Advanced multi-stage condition variable coordination problems. | See [condition_variables/README.md](./condition_variables/README.md) for details. |

---

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
make run FILE=custom_semaphore
make run FILE=reader_writer_lock
make run FILE=dining_philosophers
make run FILE=atomic_spinlock
make clean                                   # Clean artifacts
```

### Compiling a single file directly
```bash
clang -Wall -Wextra -g -pthread custom_semaphore.c -o custom_semaphore
./custom_semaphore
```
