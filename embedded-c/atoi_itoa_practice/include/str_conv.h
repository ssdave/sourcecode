/* ============================================================================
 * File: include/str_conv.h
 *
 * What is being built:
 *   Public API for custom C string-to-integer (`my_atoi`) and integer-to-string
 *   (`my_itoa`) conversion routines without standard library `<stdlib.h>` helpers.
 * ============================================================================ */

#ifndef STR_CONV_H
#define STR_CONV_H

/**
 * Converts a string to an integer.
 * Handles leading whitespace, optional sign ('+' or '-'), and digits.
 * Stops parsing at the first non-digit character.
 */
int my_atoi(const char *str);

/**
 * Converts an integer to a null-terminated string in base 10.
 * Writes output into `buf` and returns `buf`.
 */
char *my_itoa(int val, char *buf);

#endif /* STR_CONV_H */