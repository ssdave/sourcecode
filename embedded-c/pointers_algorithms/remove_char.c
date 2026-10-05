/* ============================================================================
 * File: remove_char.c
 *
 * What is being built:
 *   In-place O(N) time, O(1) space string compaction (`remove_char`) using
 *   two pointers (read pointer `str` and write pointer `wr`) to strip all
 *   occurrences of `target` from a mutable C string.
 *
 * What is being tested:
 *   Assertion suite testing interior character removal (`"hello world"` ->
 *   `"heo word"`), all-matching characters (`"aaaa"` -> `""`), character not
 *   present, empty strings (`""`), single-character strings, and `NULL` pointer
 *   safety.
 * ============================================================================ */

#include <assert.h>
#include <stdio.h>
#include <string.h>

void remove_char(char *str, char target)
{
  if(str == NULL)
    return;

  int len = strlen(str);
  if(len == 0)
    return;

  if((len == 1) && (target != *str))
    return;

  // Two-pointer in-place compaction: wr lags behind str whenever target is skipped
  char *wr = str;

  while(*str != '\0')
  {
    if(*str != target)
    {
      *wr++ = *str;
    }
    str++;
  }

  // Terminate the compacted string at the final write position
  *wr = '\0';
  return;
}

int main(void)
{
  printf("--- Running remove_char Tests ---\n");

  char s1[] = "hello world";
  remove_char(s1, 'l');
  assert(strcmp(s1, "heo word") == 0);

  char s2[] = "aaaa";
  remove_char(s2, 'a');
  assert(strcmp(s2, "") == 0);

  char s3[] = "embedded systems";
  remove_char(s3, 'e');
  assert(strcmp(s3, "mbddd systms") == 0);

  char s4[] = "no match";
  remove_char(s4, 'z');
  assert(strcmp(s4, "no match") == 0);

  char s5[] = "";
  remove_char(s5, 'a');
  assert(strcmp(s5, "") == 0);

  remove_char(NULL, 'a');

  printf("--- All tests passed! ---\n");
  return 0;
}