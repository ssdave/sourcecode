/* ============================================================================
 * LeetCode #693: Binary Number with Alternating Bits (Easy)
 * URL:      https://leetcode.com/problems/binary-number-with-alternating-bits/
 * Language: C
 *
 * What is being built / Approach:
 *   Checks whether a positive integer `n` has alternating 0 and 1 bits in its
 *   binary representation by comparing adjacent bits from LSB up to MSB.
 *   Time Complexity: O(log2(n)) | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: n = 5  (0b101)  -> true
 *   - Example 2: n = 7  (0b111)  -> false
 *   - Example 3: n = 11 (0b1011) -> false
 * ============================================================================ */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

bool hasAlternatingBits(int n)
{
  int mask = 0x1;
  int currentBit = n & mask;
  int count = (int)(sizeof(int) * 8);
  int iter = 1;
  mask |= mask << 1;

  while(iter <= count)
  {
    int temp = (n & mask) >> iter;
    if(temp == currentBit)
    {
      return false;
    }
    else
    {
      currentBit = temp;
    }
    mask = mask << 1;
    iter++;
    if(mask > n)
    {
      break;
    }
  }
  return true;
}

int main(void)
{
  printf("=== LeetCode #693: Binary Number with Alternating Bits ===\n");

  bool r1 = hasAlternatingBits(5);
  printf("Example 1: n=5  (0b101)  -> %s\n", r1 ? "true" : "false");
  assert(r1 == true);

  bool r2 = hasAlternatingBits(7);
  printf("Example 2: n=7  (0b111)  -> %s\n", r2 ? "true" : "false");
  assert(r2 == false);

  bool r3 = hasAlternatingBits(11);
  printf("Example 3: n=11 (0b1011) -> %s\n", r3 ? "true" : "false");
  assert(r3 == false);

  printf("All tests passed!\n");
  return 0;
}
