# Condition Variables Coordination Lab

Advanced multi-threaded coordination problems using POSIX condition variables (`pthread_cond_t`) and mutexes (`pthread_mutex_t`), modeling counting semaphores, multi-room service pipelines, sequential scheduling, and traffic flow.

## Academic Attribution & Copyright

Derived from **Rose-Hulman Institute of Technology — CSSE 332: Operating Systems** (`condvar_basics/`).
Authors: RHIT CSSE 332 Faculty. Used for independent self-study.

## Module Files

| File | Status | Focus Area | Description / Tests Verified |
| :--- | :--- | :--- | :--- |
| `bounded_critical_section.c` | **Implemented** | Counting Semaphore Pattern | Bounded concurrency limiting occupancy to at most 3 threads (`active_threads_count < 3`). Spawns 8 threads (`a`–`h`); verifies at most 3 concurrent entries. |
| `two_stage_service_rooms.c` | **Implemented** | Multi-Stage Service Queue | Two-stage customer service pipeline with room 1 (capacity 5), room 2 (capacity 2), and a waiting room (capacity 2). Simulates 10 arriving customers and tracks unhappy departures. |
| `ordered_thread_entry.c` | *Starter Template* | Sequential Thread Ordering | Coordinates threads 1..4 to enter a critical section strictly in ascending numerical order (`1 -> 2 -> 3 -> 4`) regardless of spawn sequence. |
| `stack_bounded_buffer.c` | *Starter Template* | Bounded Stack Buffer | 5-slot LIFO shared buffer with condition-variable producer/consumer synchronization under staggered sleep delays. |
| `tunnel_traffic_control.c` | *Starter Template* | Traffic Flow Exclusion | Directional single-lane tunnel exclusion (East-West vs. West-East) with emergency vehicle (ambulance) priority preemption. |

## How to Build & Run

### From repository root
```bash
make -C embedded-c/concurrency/condition_variables           # Build all binaries
make -C embedded-c/concurrency/condition_variables test      # Run completed tests
make -C embedded-c/concurrency/condition_variables clean     # Clean artifacts
```

### From within this folder
```bash
make                                                         # Build all binaries
make test                                                    # Run completed tests
make run FILE=bounded_critical_section                       # Run specific binary
make run FILE=two_stage_service_rooms
make run FILE=ordered_thread_entry
make run FILE=stack_bounded_buffer
make run FILE=tunnel_traffic_control
make clean                                                   # Clean artifacts
```

### Compiling a single file directly
```bash
clang -Wall -Wextra -g -pthread bounded_critical_section.c -o bounded_critical_section
./bounded_critical_section
```
