/* ============================================================================
 * LeetCode #190: Reverse Bits (Easy)
 * URL:      https://leetcode.com/problems/reverse-bits/
 * Language: C
 *
 * What is being built / Approach:
 *   Reverses the bits of a 32-bit unsigned integer by testing each input bit
 *   from LSB to MSB and setting/clearing the mirrored bit position.
 *   Time Complexity: O(1) (32 iterations) | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: n = 43261596   (00000010100101000001111010011100) -> 964176192
 *   - Example 2: n = 4294967293 (11111111111111111111111111111101) -> 3221225471
 * ============================================================================ */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

uint32_t reverseBits(uint32_t n)
{
  uint32_t reverse = 0;
  int position = 31;
  uint32_t mask = 1U;
  int count = 1;

  while(position >= 0)
  {
    if(n & mask)
    {
      reverse |= (1U << position);
    }
    else
    {
      reverse &= ~(1U << position);
    }
    mask = 0;
    if(count < 32)
    {
      mask |= (1U << count);
    }
    count++;
    position--;
  }

  return reverse;
}

int main(void)
{
  printf("=== LeetCode #190: Reverse Bits ===\n");

  uint32_t r1 = reverseBits(43261596U);
  printf("Example 1: n=43261596   -> %u\n", r1);
  assert(r1 == 964176192U);

  uint32_t r2 = reverseBits(4294967293U);
  printf("Example 2: n=4294967293 -> %u\n", r2);
  assert(r2 == 3221225471U);

  printf("All tests passed!\n");
  return 0;
}
