/* ============================================================================
 * File: src/main.c (atoi_itoa_practice)
 *
 * What is being built:
 *   Assertion-driven unit test suite for `my_atoi` and `my_itoa`.
 *
 * What is being tested:
 *   - `my_atoi`: positive (`"123"`), negative (`"-456"`), leading whitespace
 *     (`"   789"`), trailing non-digits (`"42abc"`), zero (`"0"`), empty string
 *     (`""`), and `NULL` pointer safety.
 *   - `my_itoa`: positive (`123`), negative (`-456`), zero (`0`), and powers of
 *     ten with trailing zeros (`100`).
 * ============================================================================ */

#include "str_conv.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
  printf("--- Running Essential Tests ---\n");

  /* 1. my_atoi essential cases */
  assert(my_atoi("123") == 123);    /* standard positive */
  assert(my_atoi("-456") == -456);  /* standard negative */
  assert(my_atoi("   789") == 789); /* leading whitespace */
  assert(my_atoi("42abc") == 42);   /* stop at non-digit */
  assert(my_atoi("0") == 0);        /* zero */
  assert(my_atoi("") == 0);         /* empty */
  assert(my_atoi(NULL) == 0);       /* NULL safety */
  printf("[PASS] my_atoi\n");

  /* 2. my_itoa essential cases */
  char buf[32];
  assert(strcmp(my_itoa(123, buf), "123") == 0);   /* standard positive */
  assert(strcmp(my_itoa(-456, buf), "-456") == 0); /* standard negative */
  assert(strcmp(my_itoa(0, buf), "0") == 0);       /* zero */
  assert(strcmp(my_itoa(100, buf), "100") == 0);   /* trailing zeros */
  printf("[PASS] my_itoa\n");

  printf("--- All essential tests passed! ---\n");
  return 0;
}