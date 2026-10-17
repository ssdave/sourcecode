/* ============================================================================
 * File: src/str_conv.c
 *
 * What is being built:
 *   Custom implementations of `my_atoi` (skips leading whitespace, handles
 *   optional `+`/`-` signs, accumulates decimal digits, stops on non-digits)
 *   and `my_itoa` (extracts base-10 digits in reverse order, appends `-` sign
 *   for negative numbers, and reverses the output buffer in-place).
 *
 * What is being tested:
 *   Verified by `src/main.c` across positive/negative numbers, leading spaces,
 *   trailing alphanumeric characters, zero (`"0"`), empty strings, and `NULL`
 *   input pointers.
 * ============================================================================ */

#include "str_conv.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

int my_atoi(const char *str)
{
  if(str == NULL)
  {
    return 0;
  }

  int len = strlen(str);

  if(len == 0)
  {
    return 0;
  }

  int negative = 0;
  int result = 0;

  // Skip leading whitespace and optional '+' sign
  while((*str == ' ') || (*str == '\t') || (*str == '\n') || (*str == '+'))
  {
    str++;
    len--;
  }

  // Record negative sign if present
  if(*str == '-')
  {
    negative = 1;
    str++;
    len--;
  }

  // Accumulate base-10 digits using Horner's method (result = result * 10 + digit)
  while(len)
  {
    if(*str >= '0' && *str <= '9')
    {
      result = result * 10 + (*str - '0');
    }
    else
    {
      break;
    }
    len--;
    str++;
  }

  if(negative)
  {
    result = -1 * result;
  }

  return result;
}

char *my_itoa(int val, char *buf)
{
  if(buf == NULL)
  {
    return NULL;
  }

  char *orig = buf;

  // Handle zero explicitly since the modulo loop below terminates on val == 0
  if(val == 0)
  {
    *buf++ = '0';
    *buf = '\0';
    return orig;
  }

  // Emit leading '-' sign for negative values and convert magnitude
  if(val < 0)
  {
    *buf++ = '-';
    val = val * -1;
  }

  // Track start of digit characters so we only reverse the digits, not the '-' sign
  char *rev = buf;

  // Extract least-significant base-10 digits in reverse order
  while(val)
  {
    int rem = val % 10;
    val = val / 10;
    *buf++ = '0' + rem;
  }

  *buf = '\0';

  // Reverse the digit substring [rev .. buf - 1] in-place
  char *left = rev;
  char *right = buf - 1;
  while(left < right)
  {
    char temp = *right;
    *right = *left;
    *left = temp;
    left++;
    right--;
  }

  return orig;
}