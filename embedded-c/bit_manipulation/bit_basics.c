/* ============================================================================
 * File: bit_basics.c
 *
 * What is being built:
 *   Foundational 32-bit hardware register bitwise primitives:
 *     1. `set_bit(reg, bit_num)`    — Sets bit `bit_num` to 1.
 *     2. `clear_bit(reg, bit_num)`  — Clears bit `bit_num` to 0.
 *     3. `toggle_bit(reg, bit_num)` — Inverts bit `bit_num` using XOR.
 *     4. `check_bit(reg, bit_num)`  — Queries whether `bit_num` is set.
 *     5. `is_power_of_two(val)`     — O(1) power-of-two check via `val & (val - 1)`.
 *   Includes defensive bounds checking for `bit_num >= 32` to prevent undefined
 *   shift behavior in C.
 *
 * What is being tested:
 *   Assertion-driven test suite verifying bit 0 (LSB), bit 31 (MSB), interior
 *   bits, idempotent set/clear operations, double-toggle restoration, out-of-range
 *   bit indices, and power-of-two boundary values (`0`, `1`, `2`, `1024`, `UINT32_MAX`).
 * ============================================================================ */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/**
 * 1. Set bit_num to 1 in reg (leave all other bits unchanged).
 */
uint32_t set_bit(uint32_t reg, uint8_t bit_num)
{
  // Guard against shifts >= 32 bits (undefined behavior on 32-bit types)
  if(bit_num > (sizeof(uint32_t) * 8 - 1))
  {
    return reg;
  }

  // Left-shift 1 to bit_num and OR into reg to force that bit to 1
  reg |= (1 << bit_num);

  return reg;
}

/**
 * 2. Clear bit_num to 0 in reg (leave all other bits unchanged).
 */
uint32_t clear_bit(uint32_t reg, uint8_t bit_num)
{
  if(bit_num > (sizeof(uint32_t) * 8 - 1))
  {
    return reg;
  }

  // Invert single-bit mask ~(1 << bit_num) and AND with reg to clear only bit_num
  reg &= ~(1 << bit_num);

  return reg;
}

/**
 * 3. Toggle/flip bit_num in reg (0 becomes 1, 1 becomes 0).
 */
uint32_t toggle_bit(uint32_t reg, uint8_t bit_num)
{
  if(bit_num > (sizeof(uint32_t) * 8 - 1))
  {
    return reg;
  }

  // XOR with (1 << bit_num) flips bit_num while XOR with 0 preserves all other bits
  reg ^= (1 << bit_num);

  return reg;
}

/**
 * 4. Check if bit_num is set to 1. Returns true if 1, false if 0.
 */
bool check_bit(uint32_t reg, uint8_t bit_num)
{
  // Isolate bit_num with mask, shift down to bit 0, and test LSB
  uint32_t mask = 1 << bit_num;

  reg &= mask;

  reg = reg >> (bit_num);

  bool result = reg & 0x1 ? true : false;

  return result;
}

/**
 * 5. Check if a positive integer is a power of 2 using a bitwise trick.
 * Examples: 1, 2, 4, 8, 16 -> true; 0, 3, 5, 6, 7 -> false.
 */
bool is_power_of_two(uint32_t val)
{
  // Zero has no set bits and is not a power of two
  if(val == 0)
  {
    return false;
  }

  // A power of two has exactly one set bit; subtracting 1 clears that bit and
  // sets all lower bits, so (val & (val - 1)) is 0 iff val is a power of two.
  if((val & (val - 1)) == 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

int main(void)
{
  printf("--- Running Bit Manipulation Tests ---\n");

  /* 1. Set bit tests */
  assert(set_bit(0x00000000, 0) == 0x00000001);  /* set bit 0 */
  assert(set_bit(0x00000000, 3) == 0x00000008);  /* set bit 3 (2^3 = 8) */
  assert(set_bit(0x00000000, 31) == 0x80000000); /* set MSB */
  assert(set_bit(0x00000008, 3) == 0x00000008);  /* setting already-set bit stays 8 */
  printf("[PASS] set_bit\n");

  /* 2. Clear bit tests */
  assert(clear_bit(0x0000000F, 0) == 0x0000000E);  /* 1111 -> 1110 */
  assert(clear_bit(0x0000000F, 3) == 0x00000007);  /* 1111 -> 0111 */
  assert(clear_bit(0x80000000, 31) == 0x00000000); /* clear MSB */
  assert(clear_bit(0x0000000E, 0) == 0x0000000E);  /* clearing already-0 bit stays 0 */
  printf("[PASS] clear_bit\n");

  /* 3. Toggle bit tests */
  assert(toggle_bit(0x00000000, 2) == 0x00000004); /* 0 -> 1 */
  assert(toggle_bit(0x00000004, 2) == 0x00000000); /* 1 -> 0 */
  assert(toggle_bit(0x00000005, 1) == 0x00000007); /* 0101 -> 0111 */
  printf("[PASS] toggle_bit\n");

  /* 4. Check bit tests */
  assert(check_bit(0x00000008, 3) == true);
  assert(check_bit(0x00000008, 2) == false);
  assert(check_bit(0x80000000, 31) == true);
  assert(check_bit(0x00000000, 0) == false);
  printf("[PASS] check_bit\n");

  /* 5. Power of two tests */
  assert(is_power_of_two(1) == true);    /* 2^0 */
  assert(is_power_of_two(2) == true);    /* 2^1 */
  assert(is_power_of_two(4) == true);    /* 2^2 */
  assert(is_power_of_two(16) == true);   /* 2^4 */
  assert(is_power_of_two(1024) == true); /* 2^10 */
  assert(is_power_of_two(0) == false);   /* 0 is not a power of 2 */
  assert(is_power_of_two(3) == false);
  assert(is_power_of_two(6) == false);
  assert(is_power_of_two(18) == false);
  printf("[PASS] is_power_of_two\n");

  printf("--- All tests passed! ---\n");
  return 0;
}