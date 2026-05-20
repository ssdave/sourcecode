# POSIX Threads (`pthreads`) Synchronization & Concurrency

Hands-on POSIX threads (`pthread_mutex_t`, `pthread_cond_t`) synchronization exercises covering mutual exclusion, multi-resource lock ordering, reusable barriers, bounded producer-consumer queues, and multi-stage condition variable state machines.

## Course Attribution & Copyright

All problem specifications, synchronization scenarios, and starter test harnesses in `concurrency/` and `concurrency/conditionalvariables/` originate from:

- **Course**: **Rose-Hulman Institute of Technology — CSSE 332: Operating Systems**
  - Course Schedule & Notes: [https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/](https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/)
  - Official Lab Archive: [https://github.com/rhit-csse332/csse332-labs-archive](https://github.com/rhit-csse332/csse332-labs-archive)
- **Faculty Authors & Contributors**: Mohammad Noureddine, Michael Hewner, Sid Stamm, Robert Williamson, Sriram Mohan, and the Rose-Hulman CSSE department.
- **Purpose**: Completed independently for operating systems self-study and embedded/systems concurrency practice.

---

## Mapping to CSSE 332 Labs & Activities

| File | Status | Original CSSE 332 Source | What is Built | What is Tested |
| :--- | :--- | :--- | :--- | :--- |
| [`basicMutex1.c`](./basicMutex1.c) | **Implemented** | `threads_basics/basic_mutex/add_a_lot.c` ([Threads Lab](https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/threads/)) | Mutex-protected shared accumulator (`pthread_mutex_lock` / `unlock`). | Spawns 10 threads each incrementing `total` 10,000 times; verifies zero lost updates (`total == 100000`). |
| [`basicMutex2.c`](./basicMutex2.c) | **Implemented** | `threads_basics/basic_mutex/red_blue_purple.c` ([Threads Lab](https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/threads/)) | Fine-grained dual-channel mutex locking (`redLock`, `blueLock`) with consistent lock ordering in `purpleCommand` to prevent deadlock. | Runs 3 concurrent worker threads issuing `Red`, `Blue`, `White`, and `Purple` commands; verifies non-overlapping `Start`/`End` channel intervals without deadlock. |
| [`condwait.c`](./condwait.c) | **Implemented** | `activities/condvar_intro/condwait.c` ([Session 19 Notes](https://www.rose-hulman.edu/class/csse/csse332/2324b/notes/session19/)) | Custom `thread_join()` and `thread_exit()` implementation using `pthread_cond_t` and a boolean state predicate (`child_done`). | Verifies parent-child join synchronization even if the child finishes before `thread_join()` is called. |
| [`barrier.c`](./barrier.c) | **Implemented** | `activities/condvar_intro/barrier.c` ([Session 19 Notes](https://www.rose-hulman.edu/class/csse/csse332/2324b/notes/session19/)) | N-thread rendezvous barrier (`barrier_wait`) using `pthread_cond_broadcast` when `num_threads_at_barrier == NUM_THREADS`. | Launches 5 threads with randomized sleep delays; verifies no thread proceeds past the barrier until all 5 arrive. |
| [`producerconsumer.c`](./producerconsumer.c) | **Implemented** | `activities/prodcons/03_prodcons.c` ([Session 20 Notes](https://www.rose-hulman.edu/class/csse/csse332/2324b/notes/session20/)) | Multi-producer, multi-consumer circular bounded buffer (`MAX = 10`) using two condition variables (`empty` and `full`) and `while` predicate loops. | Runs 5 producer threads and 5 consumer threads for 100 iterations each without buffer overflow, underflow, or Mesa-semantics race conditions. |
| [`conditionalvariables/max.c`](./conditionalvariables/max.c) | **Implemented** | `condvar_basics/max.c` ([Condition Variables Lab](https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/condvar/)) | Counting semaphore / bounded critical section limiting concurrent occupancy to at most 3 threads (`numInCriticalSection < 3`). | Spawns 8 threads (`a`–`h`) in staggered waves; verifies at most 3 threads occupy the critical section simultaneously. |
| [`conditionalvariables/rooms.c`](./conditionalvariables/rooms.c) | **Implemented** | `condvar_basics/rooms.c` ([Condition Variables Lab](https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/condvar/)) | Two-stage service queue with bounded first room (`cap = 5`), bounded waiting room (`cap = 2`), bounded second room (`cap = 2`), and early-exit tracking (`num_left_customers`). | Simulates 10 arriving customers; verifies room capacity invariants and counts customers who balk when both the second room and waiting room are full. |
| [`conditionalvariables/inorder.c`](./conditionalvariables/inorder.c) | *Starter Template* | `condvar_basics/inorder.c` ([Condition Variables Lab](https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/condvar/)) | Unimplemented CSSE 332 lab starter template for sequential thread ordering (`1 -> 2 -> 3 -> 4`). | Spawns threads in order `{2, 1, 4, 3}` with a 3-second delay to test ordered critical-section entry. |
| [`conditionalvariables/prodcons_condvar.c`](./conditionalvariables/prodcons_condvar.c) | *Starter Template* | `condvar_basics/` ([Condition Variables Lab](https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/condvar/)) | Unimplemented CSSE 332 lab starter template for a LIFO bounded buffer (`BUFFERSIZE = 5`). | Tests producer throttling against slow consumers (`sleep(1)`) on a shared 5-element array. |
| [`conditionalvariables/tunnel.c`](./conditionalvariables/tunnel.c) | *Starter Template* | `condvar_basics/tunnel.c` ([Condition Variables Lab](https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/condvar/)) | Unimplemented CSSE 332 lab starter template for bidirectional traffic tunnel synchronization with priority ambulances. | Simulates 15 EW cars, 15 WE cars, and 7 randomly arriving ambulances traversing the tunnel. |

## Building & Running

```bash
# Build all top-level concurrency exercises
make

# Build condition variable lab exercises
make -C conditionalvariables
```
