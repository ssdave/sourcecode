# Hardware Interfacing, MMIO & Device Drivers

Hardware-level peripheral controllers, bus configuration protocols, and direct memory access descriptor rings.

## Module Files

| File | Focus Area | Description / Concepts Verified |
| :--- | :--- | :--- |
| `mmio_uart_driver.c` | Memory-Mapped I/O | Memory-Mapped I/O (MMIO) hardware peripheral controller. Models volatile 32-bit register blocks (`SR`, `DR`, `CR`, `BSRR`), polling-driven TX/RX transfers, and atomic bit-set-reset operations without read-modify-write race conditions. |
| `pcie_config_driver.c` | PCIe Bus Architecture | PCIe Type 0 Configuration Space probe and Base Address Register (BAR) sizing engine. Implements the standard `0xFFFFFFFF` write-and-read aperture sizing probe, distinguishes 32-bit from 64-bit BARs, and programs memory base addresses. |
| `dma_ring_buffer_driver.c` | Direct Memory Access | Coherent DMA circular descriptor ring driver. Implements host-to-device ownership bit handoffs (`OWN`), hardware doorbell notification registers, memory fence barrier ordering, and descriptor completion reaping. |
| `i2c_bitbang/` | Bus Protocols & Open-Drain GPIO | Open-drain GPIO bit-banged I2C controller and peripheral driver package. Modularized across header definitions (`i2c_bitbang.h`), GPIO open-drain line simulation and mock slave model (`i2c_hal.c`), core byte serialization and transaction driver (`i2c_driver.c`), and 7-phase verification test runner (`main.c`). Implements START/STOP framing conditions, 8-bit MSB-first byte serialization, 9th clock ACK/NACK protocol arbitration, and multi-byte register read/write transactions against a mock sensor. |
| `spi_bitbang/` | Full-Duplex Synchronous Serial | 4-wire full-duplex bit-banged SPI bus controller package. Modularized across header definitions (`spi_bitbang.h`), GPIO line simulation and mock flash model (`spi_hal.c`), full-duplex byte serialization and transaction driver (`spi_driver.c`), and multi-mode test runner (`main.c`). Implements all four SPI modes (Mode 0, 1, 2, 3: CPOL and CPHA matrix), active-low chip-select framing, simultaneous 8-bit full-duplex byte and buffer streaming, JEDEC identification queries, and memory array read/write transactions against a mock SPI Flash peripheral. |
| `i2s_bitbang_driver.c` | Digital Audio Serial Bus | 3-wire digital audio serial interface controller adhering to the Philips I2S specification (BCLK, WS/LRCK, SD). Implements the iconic 1-BCLK delay following WS transitions, 16-bit two's complement stereo PCM audio serialization (Left channel WS=0, Right channel WS=1), multi-frame streaming, and master-mode ADC sample capture against a mock digital audio codec. |
| `rtos_pipeline/` | RTOS & Interrupt Processing | Top-Half (ISR) vs. Bottom-Half (RTOS Task) deferred sensor processing pipeline package. Modularized across header definitions (`rtos_sensor.h`), lock-free SPSC circular ring buffer and RTOS semaphore synchronization (`spsc_ring.c`), Top-Half ISR and Bottom-Half deferred processing task (`sensor_pipeline.c`), and 7-phase asynchronous verification harness (`main.c`). Implements hardware MMIO sensor registers (`STATUS`, `DATA`, `CR`), non-blocking interrupt handling with drop counters, and a sliding-window moving-average filter. |
| `i2c_cdev/` | UNIX VFS Character Device Subsystem | Full UNIX Virtual File System (VFS) character device emulation package for I2C bus controllers (`struct inode`, `struct file`, `struct file_operations`). Modularized across header definitions (`i2c_cdev.h`), mock hardware sensor simulation (`i2c_mock_hw.c`), VFS and user-kernel memory isolation (`cdev_vfs.c`), driver switch table handlers (`i2c_cdev_driver.c`), and multi-threaded stress harness (`main.c`). Implements memory isolation primitives (`copy_to_user`, `copy_from_user`), file descriptor table allocation, per-open session management (`filp->private_data`), standard I2C ioctl commands (`I2C_SLAVE`, `I2C_GET_STATS`, `I2C_RESET_BUS`), and multi-threaded bus serialization against a mock LM75 sensor. |

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
make run FILE=spi_bitbang_driver
make run FILE=i2s_bitbang_driver
make run FILE=rtos_sensor_pipeline
make run FILE=i2c_cdev_subsystem
make clean                                  # Clean artifacts
```

### Compiling a single file directly
```bash
clang -Wall -Wextra -g -pthread mmio_uart_driver.c -o mmio_uart_driver
./mmio_uart_driver
```

## Academic Attribution & Technical References

The specifications and hardware models implemented in this module are derived from canonical computer architecture and systems programming references:
- **Computer Systems: A Programmer's Perspective (CS:APP)**, R. Bryant and D. O'Hallaron (Chapter 6: Memory Hierarchy & I/O, Chapter 8: Exceptional Control Flow).
- **Operating Systems: Three Easy Pieces (OSTEP)**, Remzi & Andrea Arpaci-Dusseau (Chapter 30: Condition Variables, Chapter 31: Semaphores, Chapter 36: I/O Devices & Bus Protocols).
- **Real-Time Operating Systems Architecture**, FreeRTOS & POSIX-RTOS models (Deferred interrupt processing, `FromISR` synchronization semantics, and SPSC queues).
- **PCI Local Bus & PCI Express Architecture Specifications**, PCI-SIG (Type 0 Configuration Space & BAR Sizing).
- **I2C Bus Specification and User Manual (UM10204)**, NXP Semiconductors / Philips (Physical layer, open-drain framing, ACK/NACK arbitration).
- **Motorola SPI Block Guide (V04.01)** & Serial Peripheral Interface Standards (Clock polarity CPOL, clock phase CPHA, full-duplex synchronous serialization).
- **Philips Semiconductors I2S Bus Specification (Feb 1986)**, Philips / NXP (3-wire digital audio serial interface, 1-BCLK delay, Word Select left/right channel framing).
- **Linux Kernel Documentation**, Device Drivers & Memory Barriers (`Documentation/networking/device_drivers`, `Documentation/memory-barriers.txt`).
