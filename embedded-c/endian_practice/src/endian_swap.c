/* ============================================================================
 * File: src/endian_swap.c
 *
 * What is being built:
 *   Bitwise implementations of 16/32/64-bit endianness byte swapping, runtime
 *   host endianness detection by inspecting the lowest-addressed byte of
 *   `0x12345678`, and in-place 32-bit network-to-host buffer conversion.
 *
 * What is being tested:
 *   Verified via `src/main.c` against 16/32/64-bit test vectors, single-bit
 *   MSB/LSB boundaries, all-zeros/all-ones patterns, and round-trip buffer swaps.
 * ============================================================================ */

#include "endian_swap.h"
#include <stdio.h>

uint16_t swap_endian_16(uint16_t val)
{
  // Shift low byte [7:0] into [15:8] and high byte [15:8] into [7:0]
  uint16_t swap = val;
  return (swap << 8) | (val >> 8);
}

uint32_t swap_endian_32(uint32_t val)
{
  // Mask each of the 4 bytes and shift to its mirrored position (byte 0<->3, byte 1<->2)
  uint32_t swap = (val & 0x000000FF) << 24 | (val & 0x0000FF00) << 8 | (val & 0xFF000000) >> 24 |
                  (val & 0x00FF0000) >> 8;
  return swap;
}

uint64_t swap_endian_64(uint64_t val)
{
  // Step symmetric lowerMask and upperMask inward across 4 byte pairs (56, 40, 24, 8 bit shifts)
  int count = sizeof(uint64_t);
  uint64_t lowerMask = 0x00000000000000FF;
  uint64_t upperMask = 0xFF00000000000000;
  uint64_t swap = 0x00;
  int shift = 8;

  for(int iter = 0; iter < count / 2; iter++)
  {
    uint64_t temp = 0;
    shift = 64 - (((2 * iter) + 1) * 8);
    temp = ((val & lowerMask) << shift) | ((val & upperMask) >> shift);
    swap |= temp;
    lowerMask = lowerMask << 8;
    upperMask = upperMask >> 8;
  }

  return swap;
}

bool is_system_little_endian(void)
{
  // Store 0x12345678 in memory and inspect the lowest-addressed byte:
  // Little-endian stores LSB (0x78) first; big-endian stores MSB (0x12) first.
  uint32_t var = 0x12345678;
  uint8_t *p = (uint8_t *)&var;
  if(p[0] == 0x78)
    return true;

  return false;
}

void swap_endian_buffer_32(uint32_t *buf, size_t count)
{
  // Swap each 32-bit element in-place after validating buffer pointer and length
  if((buf == NULL) || (count == 0))
    return;

  for(size_t iter = 0; iter < count; iter++)
  {
    buf[iter] = swap_endian_32(buf[iter]);
  }
}