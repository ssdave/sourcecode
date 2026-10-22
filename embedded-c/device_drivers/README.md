# Hardware Interfacing, MMIO & Device Drivers

Hardware-level peripheral controllers, bus configuration protocols, and direct memory access descriptor rings.

## Module Files

| File | Focus Area | Description / Concepts Verified |
| :--- | :--- | :--- |
| `mmio_uart_driver.c` | Memory-Mapped I/O | Memory-Mapped I/O (MMIO) hardware peripheral controller. Models volatile 32-bit register blocks (`SR`, `DR`, `CR`, `BSRR`), polling-driven TX/RX transfers, and atomic bit-set-reset operations without read-modify-write race conditions. |
| `pcie_config_driver.c` | PCIe Bus Architecture | PCIe Type 0 Configuration Space probe and Base Address Register (BAR) sizing engine. Implements the standard `0xFFFFFFFF` write-and-read aperture sizing probe, distinguishes 32-bit from 64-bit BARs, and programs memory base addresses. |
| `dma_ring_buffer_driver.c` | Direct Memory Access | Coherent DMA circular descriptor ring driver. Implements host-to-device ownership bit handoffs (`OWN`), hardware doorbell notification registers, memory fence barrier ordering, and descriptor completion reaping. |

## How to Build & Run

### From repository root
```bash
make -C embedded-c/device_drivers           # Build all binaries
make -C embedded-c/device_drivers test      # Run all tests
make -C embedded-c/device_drivers clean     # Clean artifacts
```

### From within this folder
```bash
make                                        # Build all binaries
make test                                   # Run all tests
make run FILE=mmio_uart_driver              # Build and run specific target
make run FILE=pcie_config_driver
make run FILE=dma_ring_buffer_driver
make clean                                  # Clean artifacts
```

### Compiling a single file directly
```bash
clang -Wall -Wextra -g mmio_uart_driver.c -o mmio_uart_driver
./mmio_uart_driver
```

## Academic Attribution & Technical References

The specifications and hardware models implemented in this module are derived from canonical computer architecture and systems programming references:
- **Computer Systems: A Programmer's Perspective (CS:APP)**, R. Bryant and D. O'Hallaron (Chapter 6: Memory Hierarchy & I/O).
- **Operating Systems: Three Easy Pieces (OSTEP)**, Remzi & Andrea Arpaci-Dusseau (Chapter 36: I/O Devices & Bus Protocols).
- **PCI Local Bus & PCI Express Architecture Specifications**, PCI-SIG (Type 0 Configuration Space & BAR Sizing).
- **Linux Kernel Documentation**, Device Drivers & Memory Barriers (`Documentation/networking/device_drivers`, `Documentation/memory-barriers.txt`).
