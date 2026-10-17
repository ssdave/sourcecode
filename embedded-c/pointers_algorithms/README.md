# Pointers, Data Structures & Systems Algorithms

Core C pointer manipulation, double pointers, dynamic linked data structures, in-place string algorithms, and binary search.

## Module Files

| File | Status | Focus Area | Description / Tests Verified |
| :--- | :--- | :--- | :--- |
| `double_pointer.c` | **Implemented** | Double Pointers (`T **`) | Heap buffer allocation via `uint8_t **`, linked-list head mutation in-place (`list_push`), and delimiter-separated string token extraction (`next_token`). |
| `singly_linked_list.c` | **Implemented** | Singly Linked List | Dynamic heap-allocated singly linked list creation, tail node appending (`append_node`), and list traversal (`print_list`). |
| `remove_char.c` | **Implemented** | Two-Pointer In-Place Compaction | O(N) time, O(1) space two-pointer string compaction stripping all occurrences of a target character in-place. Tested on empty strings, repeating characters, and edge cases. |
| `binary_search.c` | *Starter Template* | Iterative Binary Search | 15-case test harness for O(log N) binary search with overflow-safe midpoint calculation (`low + (high - low) / 2`). |

## How to Build & Run

### From repository root
```bash
make -C embedded-c/pointers_algorithms           # Build all binaries
make -C embedded-c/pointers_algorithms test      # Run all implemented tests
make -C embedded-c/pointers_algorithms clean     # Clean artifacts
```

### From within this folder
```bash
make                                             # Build all binaries
make test                                        # Run all implemented tests
make run FILE=double_pointer                     # Run specific binary
make run FILE=singly_linked_list
make run FILE=remove_char
make run FILE=binary_search
make clean                                       # Clean artifacts
```

### Compiling a single file directly
```bash
clang -Wall -Wextra -g singly_linked_list.c -o singly_linked_list
./singly_linked_list
```
