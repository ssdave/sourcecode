# Protocol Parsers & Hardware Drivers

Low-level stream parsing, byte-level serialization, and embedded camera ISP driver register manipulation.

## Module Files

| File | Focus Area | Description / Tests Verified |
| :--- | :--- | :--- |
| `tlv_parser.c` | Stream Decoding | Type-Length-Value (TLV) stream parser decoding heterogeneous payload packets (`TEMPERATURE = 0x01`, `HUMIDITY = 0x02`, `BATTERY = 0x03`). Validates buffer length bounds and detects truncated packets. |
| `uint32_serializer.c` | Byte Serialization | Pointer-based byte serialization and deserialization between 32-bit unsigned integers (`uint32_t`) and 4-byte raw arrays (`uint8_t[4]`). Demonstrates endian-sensitive memory layout. |
| `camera_isp_driver.c` | Hardware Register Bitfields | Embedded camera ISP hardware register manipulation. Updates bits [15:12] using safe read-modify-write bitwise masks without corrupting neighboring bit fields. |

## How to Build & Run

### From repository root
```bash
make -C embedded-c/parsers_drivers           # Build all binaries
make -C embedded-c/parsers_drivers test      # Run all tests
make -C embedded-c/parsers_drivers clean     # Clean artifacts
```

### From within this folder
```bash
make                                         # Build all binaries
make test                                    # Run all tests
make run FILE=tlv_parser                     # Run specific binary
make run FILE=uint32_serializer
make run FILE=camera_isp_driver
make clean                                   # Clean artifacts
```

### Compiling a single file directly
```bash
clang -Wall -Wextra -g tlv_parser.c -o tlv_parser
./tlv_parser
```
