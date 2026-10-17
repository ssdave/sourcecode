/* ============================================================================
 * LeetCode #3: Longest Substring Without Repeating Characters (Medium)
 * URL:      https://leetcode.com/problems/longest-substring-without-repeating-characters/
 * Language: C
 *
 * What is being built / Approach:
 *   Finds the length of the longest substring without duplicate characters
 *   using two pointers `i` and `j` scanning the active window `[i, j)`.
 *   Time Complexity: O(n^2) | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: s = "abcabcbb" -> 3 ("abc")
 *   - Example 2: s = "bbbbb"    -> 1 ("b")
 *   - Example 3: s = "pwwkew"   -> 3 ("wke")
 * ============================================================================ */

#include <assert.h>
#include <stdio.h>

int lengthOfLongestSubstring(char *s)
{
  int i = 0;
  int j = 1;
  int length = 1;
  int max_length = 1;

  if(s == NULL || s[i] == '\0')
  {
    return 0;
  }

  while(s[j] != '\0')
  {
    int match = 0;

    for(int idx = i; idx < j; idx++)
    {
      if(s[idx] == s[j])
      {
        match = 1;
      }
    }

    if((s[i] != s[j]) && (!match))
    {
      j++;
      length++;

      if(length > max_length)
      {
        max_length = length;
      }
    }
    else
    {
      length = 1;
      i++;
      j = i + 1;
    }
  }

  return max_length;
}

int main(void)
{
  printf("=== LeetCode #3: Longest Substring Without Repeating Characters ===\n");

  int r1 = lengthOfLongestSubstring("abcabcbb");
  printf("Example 1: s=\"abcabcbb\" -> %d\n", r1);
  assert(r1 == 3);

  int r2 = lengthOfLongestSubstring("bbbbb");
  printf("Example 2: s=\"bbbbb\"    -> %d\n", r2);
  assert(r2 == 1);

  int r3 = lengthOfLongestSubstring("pwwkew");
  printf("Example 3: s=\"pwwkew\"   -> %d\n", r3);
  assert(r3 == 3);

  printf("All tests passed!\n");
  return 0;
}
