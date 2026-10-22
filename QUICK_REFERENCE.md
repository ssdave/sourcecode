# Systems & Embedded Engineering: 15-Minute Quick Reference

> An ultra-dense, high-yield reference card designed for a 15–20 minute rapid review.  
> *Deep-dive textbook treatise available in [`SYSTEMS_REFERENCE.md`](./SYSTEMS_REFERENCE.md).*

---

## 1. Pointers, Memory & Data Layout

### Double Pointers (`T **`)
- **Core Rule**: In C, all arguments are passed by value. To modify where the caller's pointer points, you must pass the address of that pointer (`T **`).
- **Dynamic Allocation**: `bool alloc(uint8_t **out, size_t sz) { *out = malloc(sz); return *out != NULL; }`
- **Linus Torvalds List Deletion**: Eliminates special-case head branches by modifying the link pointing to the target:
  ```c
  void remove_node(Node **curr, int val) {
      while(*curr && (*curr)->val != val) curr = &((*curr)->next);
      if(*curr) { Node *tmp = *curr; *curr = (*curr)->next; free(tmp); }
  }
  ```

### Struct Padding & Memory Alignment
- **Natural Alignment**: An $N$-byte primitive must be stored at a memory address divisible by $N$ (`sizeof(uint32_t) == 4` $\rightarrow$ address must be a multiple of 4).
- **Rule of Thumb**: Order struct fields from **largest to smallest** to minimize internal padding holes.
- **Pack Attribute**: `__attribute__((packed))` eliminates padding bytes at the cost of unaligned hardware memory access penalties.

---

## 2. Bit Manipulation & Volatile Semantics

| Operation | C Idiom | Machine Context |
| :--- | :--- | :--- |
| **Set Bit $n$** | `REG \|= (1U << n)` | Asserts a control line / enable bit |
| **Clear Bit $n$** | `REG &= ~(1U << n)` | Disables a peripheral interrupt |
| **Toggle Bit $n$** | `REG ^= (1U << n)` | Inverts an output GPIO pin |
| **Test Bit $n$** | `(REG >> n) & 1U` | Checks ready / status flag |
| **Isolate Lowest Set Bit** | `val & (-val)` | Two's-complement sign-inversion property |
| **Clear Lowest Set Bit** | `val & (val - 1)` | Kernighan bit counter; checks if power-of-two (`== 0`) |

### The `volatile` Qualifier Contract
- **What it DOES**: Tells the compiler that the memory location can change outside the program's control (hardware MMIO, ISR). Prevents compiler register-caching and dead-code elimination.
- **What it DOES NOT DO**: Provides **zero atomicity** and **zero hardware memory barriers**. Never use `volatile` for thread synchronization; use C11 `<stdatomic.h>`.

---

## 3. Ring Buffers & Lock-Free SPSC

| Architecture | Full Condition | Empty Condition | Concurrency Model |
| :--- | :--- | :--- | :--- |
| **Power-of-Two** ($2^k$) | `((head + 1) & MASK) == tail` | `head == tail` | **Lock-Free SPSC** (Producer writes `head`, Consumer writes `tail`). |
| **Counted Ring Buffer** | `count == CAPACITY` | `count == 0` | **Requires Locks** (Both sides mutate `count` $\rightarrow$ data race). |
| **Linux `kfifo`** | `(in - out) == CAPACITY` | `in == out` | **Free-running unsigned integers** wrap on overflow; 100% capacity used. |

- **Bitmask Wrap**: `(index + 1) & (CAPACITY - 1)` runs in **1 clock cycle** vs. 15–40 cycles for `% CAPACITY`.
- **False Sharing**: Separate `head` and `tail` across different 64-byte cache lines: `struct alignas(64) SPSCQueue { ... };`.

---

## 4. Processes, Threads & Operating Systems Core

### Process vs. Thread Context Switch
```
Process Switch (Heavy): Save Regs -> Switch Kernel Stack -> Reload CR3/TTBR0 -> Flush TLB -> Cold Caches
Thread Switch  (Light): Save Regs -> Switch Kernel Stack -> Retain CR3/TTBR0 -> Keep TLB   -> Warm Caches
```

### Process Lifecycle & Zombie Reaping
- **Zombie Process**: A terminated child whose parent hasn't read its exit status via `waitpid()`. Holds a slot in the kernel PID table.
- **Non-blocking Reaper**: Must use a `while` loop with `WNOHANG` inside the `SIGCHLD` handler to prevent dropped signals:
  ```c
  void sigchld_handler(int sig) {
      (void)sig;
      while(waitpid(-1, NULL, WNOHANG) > 0);
  }
  ```
- **Exit Status Macros**:
  - `WIFEXITED(status)` $\rightarrow$ true if normal exit; read code with `WEXITSTATUS(status)`.
  - `WIFSIGNALED(status)` $\rightarrow$ true if crashed/terminated by signal; read signal with `WTERMSIG(status)`.

---

## 5. UNIX Pipes & Inter-Process Communication

1. **Kernel Reference Counting**: A pipe's write end stays open as long as at least one descriptor refers to it across any process.
2. **Phantom Writer Deadlock**: If a parent forks a child writer and fails to `close(pipefd[WRITE_FD])`, reader processes will block indefinitely on `read()` waiting for `EOF` that never arrives.
3. **Broken Pipe**: Writing to a pipe with no active reader triggers `SIGPIPE` (default terminates process). Set `signal(SIGPIPE, SIG_IGN)` so `write()` returns `-1` with `errno == EPIPE`.
4. **`PIPE_BUF` Atomicity**: Writes of $\le 512$ bytes (POSIX) or $\le 4096$ bytes (Linux) are guaranteed **atomic** (never interleaved across concurrent writers).
5. **Stream Semantics**: Pipes carry raw bytes, not strings. `read()` **never** appends `\0`. Always null-terminate manually: `buf[bytes_read] = '\0'`.

---

## 6. Concurrency, Synchronization & Memory Models

### Mesa vs. Hoare Condition Variables
- **POSIX uses Mesa semantics**: When a thread signals a condition variable, the woken thread is placed on the ready queue and must re-contend for the mutex.
- **Golden Rule**: Always check condition predicates inside a `while` loop (never `if`) to guard against **spurious wakeups** and intervening threads:
  ```c
  pthread_mutex_lock(&lock);
  while(!condition) pthread_cond_wait(&cond, &lock);
  // Critical Section
  pthread_mutex_unlock(&lock);
  ```

### Mutex vs. Counting Semaphore
| Feature | Mutex (`pthread_mutex_t`) | Counting Semaphore (`sem_t`) |
| :--- | :--- | :--- |
| **Ownership** | Strict thread ownership (locker must unlock). | No ownership (any thread or ISR can signal). |
| **ISR / Signal Safety** | **Unsafe** (Cannot sleep or acquire locks in an ISR). | **Safe** (`sem_post` is async-signal-safe). |
| **Priority Inversion** | Mitigated via Priority Inheritance (`PTHREAD_PRIO_INHERIT`). | Cannot track future signaling thread priority. |

### Deadlock Prevention (Coffman Conditions)
Deadlock occurs if and only if all 4 conditions hold:
1. **Mutual Exclusion** | 2. **Hold and Wait** | 3. **No Preemption** | 4. **Circular Wait**
- **Breaking Circular Wait**: Enforce a **strict total resource hierarchy** ($O(R_a) < O(R_b)$). Always acquire locks in ascending index/address order.

### C11 Memory Orderings (`<stdatomic.h>`)
- **`memory_order_relaxed`**: Atomic update with zero synchronization or ordering constraints.
- **`memory_order_acquire`**: Lock acquisition. Subsequent reads/writes cannot be reordered *before* this operation.
- **`memory_order_release`**: Lock release. Prior reads/writes cannot be reordered *after* this operation.
- **MESI Bus Traffic**: Naive spinloops (`while(atomic_flag_test_and_set)`) cause continuous cache-line invalidation storms across CPU interconnects. Use **Test-and-Test-and-Set (TTAS)** with backoff or CPU pause hints (`__builtin_ia32_pause()`).

---

## 7. Memory Hierarchy, Caches & Alignment

### The Latency Pyramid (10-Second Recall)
- **L1 Cache**: ~1–4 cycles (~1 ns) | Split I-Cache & D-Cache per core (32–64 KB).
- **L2 Cache**: ~10–14 cycles (~3–4 ns) | Unified per core (256–512 KB).
- **L3 (LLC)**: ~40–60 cycles (~15–20 ns) | Shared across all cores (8–32+ MB).
- **DRAM**: ~100–200 cycles (~50–80 ns) | Main memory bus access.

### The 4 C's of Cache Misses
1. **Compulsory (Cold)**: First access to a cache line.
2. **Capacity**: Active working set exceeds physical cache capacity.
3. **Conflict**: Multiple memory addresses hash to the same set in an associative cache.
4. **Coherence**: Multi-core only. A line is invalidated because another core modified it (**False Sharing**).

---

## 8. Virtual Memory, Page Tables & TLB

### 4-Level x86-64 Page Table Walk (48-bit Virtual Address)
$$\text{48-bit VA} = \underbrace{\text{PML4 (9b)}}_{\text{Bit 47:39}} \longrightarrow \underbrace{\text{PDPT (9b)}}_{\text{Bit 38:30}} \longrightarrow \underbrace{\text{PD (9b)}}_{\text{Bit 29:21}} \longrightarrow \underbrace{\text{PT (9b)}}_{\text{Bit 20:12}} \longrightarrow \underbrace{\text{Offset (12b)}}_{\text{Bit 11:0}}$$
- **CR3 Register**: Holds the physical base address of the top-level PML4 table.
- **Huge Pages**: 2MB pages bypass the PT level (21-bit offset); 1GB pages bypass the PD level. Drastically expands **TLB reach** and slashes memory translation walks.

### Page Fault Hierarchy
1. **Minor (Soft) Fault**: Page is in physical RAM (e.g., page cache, CoW unmapped frame). Resolved rapidly without disk I/O.
2. **Major (Hard) Fault**: Page is swapped out or backed by a file on disk. Requires slow storage I/O.
3. **Invalid / Protection Fault**: Invalid virtual address or permission violation (e.g., writing to read-only code) $\rightarrow$ `SIGSEGV`.

---

## 9. Hardware Interfacing, MMIO & PCIe Drivers

### Memory-Mapped I/O (MMIO)
- Registers are mapped into the physical address space. Access via typed volatile pointers:
  ```c
  #define UART_STATUS_REG (*(volatile uint32_t *)0x40001000)
  ```
- **Atomic Set/Clear**: Prefer hardware Set/Clear registers (like ARM BSRR) over non-atomic Read-Modify-Write (`REG |= BIT`) to eliminate race conditions without disabling interrupts.

### PCIe Configuration & BAR Sizing Probe
1. **Host writes all 1s** (`0xFFFFFFFF`) to the target Base Address Register (BAR).
2. **Device returns mask** with read-only zeros in lower bits representing requested address span.
3. **Host calculates size**: $\text{Size} = \sim(\text{ReadValue} \ \& \ \sim 0\text{xF}) + 1$.
4. **Host writes allocated physical Base Address** into the BAR.

### Coherent DMA Ring Buffers
- Host and device share a circular descriptor ring in non-cacheable/coherent physical RAM.
- **Ownership Bit (`OWN`)**: $1 = \text{Device owned}$, $0 = \text{Host owned}$.
- **Doorbell Register**: MMIO register the host writes to alert the hardware that new descriptors are ready for DMA transfer.

---

## 10. The 60-Second Sanity Checklist

- [ ] Every condition variable wait is inside a `while` loop (`while(!pred)`).
- [ ] Every allocated pointer passed by reference is a double pointer (`T **out`).
- [ ] Child process reaping uses non-blocking loops (`while(waitpid(-1, NULL, WNOHANG) > 0)`).
- [ ] All inherited pipe write ends are closed immediately post-`fork()`.
- [ ] Multi-threaded shared data across cores is padded to 64 bytes (`alignas(64)`).
- [ ] Atomics use explicit acquire/release ordering (`memory_order_acquire` / `release`).
- [ ] Mutexes are never locked inside an ISR; use semaphores or bottom-half workqueues.
- [ ] Multi-resource locks are always acquired in strictly increasing total order.
