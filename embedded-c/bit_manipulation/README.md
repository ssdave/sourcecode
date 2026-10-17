# Bit Manipulation & Hardware Registers

Bitwise operators, bitfield packing, popcounts, and memory-mapped hardware register unions.

## Module Files

| File | Focus Area | Description / Tests Verified |
| :--- | :--- | :--- |
| `bit_basics.c` | Core Bitwise Operations | Set, clear, toggle, and test bits at specific bit offsets (`1 << n`). Verifies power-of-two tests and boundary masking. |
| `bitfield_operations.c` | Register Field Manipulation | Fast-path Hamming weight / popcount (`count_set_bits`), multi-bit field extraction, and read-modify-write field insertion without altering neighboring bits. |
| `register_union_overlay.c` | Memory-Mapped Register Overlays | C `union` mapping a 32-bit hardware register word (`value`) across four 8-bit slice bitfields (`val1`..`val4`). Tests sub-byte overlay updates. |

## How to Build & Run

### From repository root
```bash
make -C embedded-c/bit_manipulation           # Build all binaries
make -C embedded-c/bit_manipulation test      # Run all tests
make -C embedded-c/bit_manipulation clean     # Clean artifacts
```

### From within this folder
```bash
make                                          # Build all binaries
make test                                     # Run all tests
make run FILE=bit_basics                      # Run specific binary
make run FILE=bitfield_operations
make run FILE=register_union_overlay
make clean                                    # Clean artifacts
```

### Compiling a single file directly
```bash
clang -Wall -Wextra -g bit_basics.c -o bit_basics
./bit_basics
```
