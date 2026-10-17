/* ============================================================================
 * LeetCode #28: Find the Index of the First Occurrence in a String (Easy)
 * URL:      https://leetcode.com/problems/find-the-index-of-the-first-occurrence-in-a-string/
 * Language: C
 *
 * What is being built / Approach:
 *   Finds the first starting index of substring `needle` in `haystack`, or -1
 *   if `needle` does not occur in `haystack`.
 *   Time Complexity: O(n * m) | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: haystack = "sadbutsad", needle = "sad"  -> 0
 *   - Example 2: haystack = "leetcode",  needle = "leeto" -> -1
 * ============================================================================ */

#include <assert.h>
#include <stdio.h>
#include <string.h>

int strStr(char *haystack, char *needle)
{
  int index = 0, count1 = 0, count2 = 0;
  int len1 = (int)strlen(haystack);
  int len2 = (int)strlen(needle);

  if(len1 < len2)
  {
    return -1;
  }
  else if((len1 == 0) && (len2 == 0))
  {
    return 0;
  }
  else if(len2 == 0)
  {
    return 0;
  }

  while(count1 < len1)
  {
    if(haystack[count1] == needle[count2])
    {
      index = count1;
      int flag = 0;
      for(int i = count1; i < count1 + len2; i++, count2++)
      {
        if(haystack[i] != needle[count2])
        {
          flag = 1;
          break;
        }
      }
      if(!flag)
      {
        return index;
      }
      else
      {
        count2 = 0;
      }
    }
    count1++;
  }
  return -1;
}

int main(void)
{
  printf("=== LeetCode #28: Find the Index of the First Occurrence in a String ===\n");

  int r1 = strStr("sadbutsad", "sad");
  printf("Example 1: haystack=\"sadbutsad\", needle=\"sad\"   -> %d\n", r1);
  assert(r1 == 0);

  int r2 = strStr("leetcode", "leeto");
  printf("Example 2: haystack=\"leetcode\",  needle=\"leeto\" -> %d\n", r2);
  assert(r2 == -1);

  printf("All tests passed!\n");
  return 0;
}
