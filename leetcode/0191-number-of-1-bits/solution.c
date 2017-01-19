/* ============================================================================
 * LeetCode #191: Number of 1 Bits (Easy)
 * URL:      https://leetcode.com/problems/number-of-1-bits/
 * Language: C
 *
 * What is being built / Approach:
 *   Counts the number of set bits (Hamming weight) in a 32-bit unsigned integer
 *   using Brian Kernighan's bit-clearing trick `n = n & (n - 1)`.
 *   Time Complexity: O(k) where k is number of set bits | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: n = 11         (0b1011)     -> 3
 *   - Example 2: n = 128        (0b10000000) -> 1
 *   - Example 3: n = 2147483645              -> 30
 * ============================================================================ */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

int hammingWeight(uint32_t n)
{
    int count = 0;
    if(n == 0)
    {
        return 0;
    }
    while(n != 0)
    {
        n = n & (n - 1);
        count++;
    }
    return count;
}

int main(void)
{
    printf("=== LeetCode #191: Number of 1 Bits ===\n");

    int r1 = hammingWeight(11U);
    printf("Example 1: n=11         -> %d\n", r1);
    assert(r1 == 3);

    int r2 = hammingWeight(128U);
    printf("Example 2: n=128        -> %d\n", r2);
    assert(r2 == 1);

    int r3 = hammingWeight(2147483645U);
    printf("Example 3: n=2147483645 -> %d\n", r3);
    assert(r3 == 30);

    printf("All tests passed!\n");
    return 0;
}
