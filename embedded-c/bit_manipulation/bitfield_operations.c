/* ============================================================================
 * File: bitfield_operations.c
 *
 * What is being built:
 *   Intermediate 32-bit register bitfield and bit-counting routines:
 *     1. `count_set_bits(val)` — Counts set bits (Hamming weight / popcount)
 *        with fast-path checks for `0` and powers of two (`val & (val - 1)`).
 *     2. `extract_bitfield(reg, offset, width)` — Masks and right-aligns a
 *        `width`-bit field starting at `offset`.
 *     3. `insert_bitfield(reg, val, offset, width)` — Read-modify-write insertion
 *        of `val` into `reg[offset + width - 1 : offset]` without disturbing
 *        adjacent bits.
 *
 * What is being tested:
 *   Assertion test suite covering zero, single-bit, nibble, alternating-bit, and
 *   all-ones (`0xFFFFFFFF`) popcounts, plus multi-bit field extraction and
 *   read-modify-write register field updates across byte and word boundaries.
 * ============================================================================ */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/**
 * 1. Count the number of set bits (1s) in a 32-bit integer.
 * Example: 0x0000000F -> 4; 0x00000000 -> 0; 0x80000001 -> 2
 */
uint8_t count_set_bits(uint32_t val)
{
  uint8_t numOfBits = 0;
  // Fast paths for 0 and exact powers of two (single bit set)
  if(val == 0)
  {
    return 0;
  }
  else if((val & (val - 1)) == 0)
  {
    return 1;
  }
  else if(val == 1)
  {
    return 1;
  }
  else
  {
    // Shift through all 32 bits, accumulating LSB matches
    uint8_t count = sizeof(uint32_t) * 8;
    uint8_t mask = 0x1;

    while(count)
    {
      if(val & mask)
      {
        numOfBits++;
      }
      count--;
      val = val >> 1;
    }
  }
  return numOfBits;
}

/**
 * 2. Extract a bitfield of 'width' bits starting at 'offset' (0-indexed).
 * The result should be right-aligned (shifted to start at bit 0).
 * Example: reg = 0x00000050 (binary ...01010000), offset = 4, width = 4
 *          Extracts '0101', returning 5 (0x05).
 */
uint32_t extract_bitfield(uint32_t reg, uint8_t offset, uint8_t width)
{
  if((offset > 31) || (width > 31) || ((offset + width) > 31))
  {
    return reg;
  }

  // Build a mask with 1s across bits [offset .. offset + width - 1]
  uint32_t mask = 0;
  uint8_t start = offset;
  uint8_t end = offset + width;
  while(start < end)
  {
    mask |= 1 << start;
    start++;
  }

  // Isolate the bitfield and right-align to bit 0
  return (reg & mask) >> offset;
}

/**
 * 3. Insert 'val' into 'reg' across 'width' bits starting at 'offset'.
 * Must clear the existing bits at [offset ... offset+width-1] first,
 * then insert 'val' without disturbing any neighboring bits.
 * Example: reg = 0xFFFFFFFF, offset = 4, width = 4, val = 0x0
 *          Clears bits [7:4], returning 0xFFFFFF0F.
 */
uint32_t insert_bitfield(uint32_t reg, uint8_t offset, uint8_t width, uint32_t val)
{
  if((offset > 31) || (width > 31) || ((offset + width) > 31))
  {
    return reg;
  }

  // Step 1: Clear target bit range [offset .. offset + width - 1] in reg
  uint8_t start = offset;
  uint8_t end = offset + width;
  while(start < end)
  {
    reg &= ~(1 << start);
    start++;
  }

  // Step 2: Shift new field value to offset and OR into cleared bitfield slot
  val = val << offset;

  reg |= val;

  return reg;
}

int main(void)
{
  printf("--- Running Bit Manipulation Level 2 Tests ---\n");

  /* 1. Count set bits */
  assert(count_set_bits(0x00000000) == 0);
  assert(count_set_bits(0x00000001) == 1);
  assert(count_set_bits(0x0000000F) == 4);
  assert(count_set_bits(0x55555555) == 16); /* alternating 0101... */
  assert(count_set_bits(0xFFFFFFFF) == 32);
  printf("[PASS] count_set_bits\n");

  /* 2. Extract bitfield */
  /* Register: 0b...1011_0000 (0xB0), offset 4, width 4 -> should extract 0b1011 (11) */
  assert(extract_bitfield(0x000000B0, 4, 4) == 0x0B);
  /* Extract single bit at offset 3 */
  assert(extract_bitfield(0x00000008, 3, 1) == 1);
  /* Extract 8 bits from middle of register: 0x12345678, offset 8, width 8 -> 0x56 */
  assert(extract_bitfield(0x12345678, 8, 8) == 0x56);
  printf("[PASS] extract_bitfield\n");

  /* 3. Insert bitfield */
  /* In 0x00000000, insert 0x5 at offset 4 with width 4 -> 0x00000050 */
  assert(insert_bitfield(0x00000000, 4, 4, 0x5) == 0x00000050);
  /* In 0xFFFFFFFF, clear bits [7:4] by inserting 0x0 -> 0xFFFFFF0F */
  assert(insert_bitfield(0xFFFFFFFF, 4, 4, 0x0) == 0xFFFFFF0F);
  /* Modify baud rate field in register without touching other control bits:
     reg = 0xABCD0012, insert 0xEF into bits [15:8] (width 8, offset 8) -> 0xABCDEF12 */
  assert(insert_bitfield(0xABCD0012, 8, 8, 0xEF) == 0xABCDEF12);
  printf("[PASS] insert_bitfield\n");

  printf("--- All tests passed! ---\n");
  return 0;
}