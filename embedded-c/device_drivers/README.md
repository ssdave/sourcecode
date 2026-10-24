# Hardware Interfacing, MMIO & Device Drivers

Hardware-level peripheral controllers, bus configuration protocols, and direct memory access descriptor rings.

## Module Files

| File | Focus Area | Description / Concepts Verified |
| :--- | :--- | :--- |
| `mmio_uart_driver.c` | Memory-Mapped I/O | Memory-Mapped I/O (MMIO) hardware peripheral controller. Models volatile 32-bit register blocks (`SR`, `DR`, `CR`, `BSRR`), polling-driven TX/RX transfers, and atomic bit-set-reset operations without read-modify-write race conditions. |
| `pcie_config_driver.c` | PCIe Bus Architecture | PCIe Type 0 Configuration Space probe and Base Address Register (BAR) sizing engine. Implements the standard `0xFFFFFFFF` write-and-read aperture sizing probe, distinguishes 32-bit from 64-bit BARs, and programs memory base addresses. |
| `dma_ring_buffer_driver.c` | Direct Memory Access | Coherent DMA circular descriptor ring driver. Implements host-to-device ownership bit handoffs (`OWN`), hardware doorbell notification registers, memory fence barrier ordering, and descriptor completion reaping. |
| `i2c_bitbang_driver.c` | Bus Protocols & Open-Drain GPIO | Open-drain GPIO bit-banged I2C controller and peripheral driver. Implements START/STOP framing conditions, 8-bit MSB-first byte serialization, 9th clock ACK/NACK protocol arbitration, and multi-byte register read/write transactions against a mock sensor. |
| `rtos_sensor_pipeline.c` | RTOS & Interrupt Processing | Top-Half (ISR) vs. Bottom-Half (RTOS Task) deferred sensor processing pipeline. Implements hardware MMIO sensor registers (`STATUS`, `DATA`, `CR`), a lock-free Single-Producer Single-Consumer (SPSC) circular queue, non-blocking interrupt handling with drop counters, RTOS counting semaphore synchronization, and a sliding-window moving-average filter. |

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
make run FILE=i2c_bitbang_driver
make run FILE=rtos_sensor_pipeline
make clean                                  # Clean artifacts
```

### Compiling a single file directly
```bash
clang -Wall -Wextra -g -pthread rtos_sensor_pipeline.c -o rtos_sensor_pipeline
./rtos_sensor_pipeline
```

## Academic Attribution & Technical References

The specifications and hardware models implemented in this module are derived from canonical computer architecture and systems programming references:
- **Computer Systems: A Programmer's Perspective (CS:APP)**, R. Bryant and D. O'Hallaron (Chapter 6: Memory Hierarchy & I/O, Chapter 8: Exceptional Control Flow).
- **Operating Systems: Three Easy Pieces (OSTEP)**, Remzi & Andrea Arpaci-Dusseau (Chapter 30: Condition Variables, Chapter 31: Semaphores, Chapter 36: I/O Devices & Bus Protocols).
- **Real-Time Operating Systems Architecture**, FreeRTOS & POSIX-RTOS models (Deferred interrupt processing, `FromISR` synchronization semantics, and SPSC queues).
- **PCI Local Bus & PCI Express Architecture Specifications**, PCI-SIG (Type 0 Configuration Space & BAR Sizing).
- **I2C Bus Specification and User Manual (UM10204)**, NXP Semiconductors / Philips (Physical layer, open-drain framing, ACK/NACK arbitration).
- **Linux Kernel Documentation**, Device Drivers & Memory Barriers (`Documentation/networking/device_drivers`, `Documentation/memory-barriers.txt`).
