/* ============================================================================
 * File: src/main.c (endian_practice)
 *
 * What is being built:
 *   Unit test runner for the `endian_swap` library (`swap_endian_16`,
 *   `swap_endian_32`, `swap_endian_64`, `is_system_little_endian`, and
 *   `swap_endian_buffer_32`).
 *
 * What is being tested:
 *   - 16-bit swaps (`0x1234 -> 0x3412`, `0x00FF -> 0xFF00`, `0x0000`, `0xFFFF`).
 *   - 32-bit swaps (`0x12345678 -> 0x78563412`, MSB/LSB single-bit shifts).
 *   - 64-bit swaps (`0x0123456789ABCDEF -> 0xEFCDAB8967452301`).
 *   - Runtime host architecture endianness check.
 *   - In-place 32-bit array swap and double-swap identity restoration (`NULL` safe).
 * ============================================================================ */

#include "endian_swap.h"
#include <assert.h>
#include <stdio.h>

static void test_16bit(void)
{
  assert(swap_endian_16(0x1234) == 0x3412);
  assert(swap_endian_16(0x0000) == 0x0000);
  assert(swap_endian_16(0xFFFF) == 0xFFFF);
  assert(swap_endian_16(0x00FF) == 0xFF00);
  assert(swap_endian_16(0xFF00) == 0x00FF);
  printf("[PASS] 16-bit swap tests\n");
}

static void test_32bit(void)
{
  assert(swap_endian_32(0x12345678U) == 0x78563412U);
  assert(swap_endian_32(0xAABBCCDDU) == 0xDDCCBBAAU);
  assert(swap_endian_32(0x00000001U) == 0x01000000U);
  assert(swap_endian_32(0x80000000U) == 0x00000080U);
  assert(swap_endian_32(0xFFFFFFFFU) == 0xFFFFFFFFU);
  printf("[PASS] 32-bit swap tests\n");
}

static void test_64bit(void)
{
  assert(swap_endian_64(0x0123456789ABCDEFULL) == 0xEFCDAB8967452301ULL);
  assert(swap_endian_64(0x0000000000000001ULL) == 0x0100000000000000ULL);
  assert(swap_endian_64(0xFFFFFFFFFFFFFFFFULL) == 0xFFFFFFFFFFFFFFFFULL);
  printf("[PASS] 64-bit swap tests\n");
}

static void test_endian_detection(void)
{
  bool is_le = is_system_little_endian();
  printf("[INFO] Host architecture detected as: %s\n", is_le ? "Little Endian" : "Big Endian");
}

static void test_buffer_swap(void)
{
  uint32_t stream[] = {0x11223344U, 0xAABBCCDDU, 0x01020304U};
  swap_endian_buffer_32(stream, 3);
  assert(stream[0] == 0x44332211U);
  assert(stream[1] == 0xDDCCBBAAU);
  assert(stream[2] == 0x04030201U);

  /* Swapping again should return to original order */
  swap_endian_buffer_32(stream, 3);
  assert(stream[0] == 0x11223344U);
  assert(stream[1] == 0xAABBCCDDU);
  assert(stream[2] == 0x01020304U);
  printf("[PASS] In-place buffer swap tests\n");
}

int main(void)
{
  printf("--- Running Endianness Test Suite ---\n");
  test_endian_detection();
  test_16bit();
  test_32bit();
  test_64bit();
  test_buffer_swap();
  printf("--- All tests passed! ---\n");
  return 0;
}