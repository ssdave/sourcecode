/* ============================================================================
 * LeetCode #7: Reverse Integer (Medium)
 * URL:      https://leetcode.com/problems/reverse-integer/
 * Language: C
 *
 * What is being built / Approach:
 *   Reverses the base-10 digits of a signed 32-bit integer and returns 0 if
 *   the reversed value overflows `[INT_MIN, INT_MAX]`.
 *   Time Complexity: O(log10(|x|)) | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: x = 123  -> 321
 *   - Example 2: x = -123 -> -321
 *   - Example 3: x = 120  -> 21
 * ============================================================================ */

#include <stdio.h>
#include <limits.h>
#include <assert.h>

int reverse(int x)
{
    long int num = 0;
    while(x)
    {
        num = num * 10 + (x % 10);
        x = x / 10;
    }

    return (((num < INT_MIN) || (num > INT_MAX)) ? 0 : (int)num);
}

int main(void)
{
    printf("=== LeetCode #7: Reverse Integer ===\n");

    int r1 = reverse(123);
    printf("Example 1: x=123  -> %d\n", r1);
    assert(r1 == 321);

    int r2 = reverse(-123);
    printf("Example 2: x=-123 -> %d\n", r2);
    assert(r2 == -321);

    int r3 = reverse(120);
    printf("Example 3: x=120  -> %d\n", r3);
    assert(r3 == 21);

    printf("All tests passed!\n");
    return 0;
}
