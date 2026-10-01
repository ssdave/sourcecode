/* ============================================================================
 * File: include/endian_swap.h
 *
 * What is being built:
 *   Public interface for 16-bit, 32-bit, and 64-bit byte-order reversal
 *   (`swap_endian_16`, `swap_endian_32`, `swap_endian_64`), runtime host CPU
 *   endianness detection (`is_system_little_endian`), and in-place 32-bit
 *   buffer byte swapping (`swap_endian_buffer_32`).
 * ============================================================================ */

#ifndef ENDIAN_SWAP_H
#define ENDIAN_SWAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Reverses the byte order of a 16-bit unsigned integer.
 * Converts Big Endian <-> Little Endian.
 */
uint16_t swap_endian_16(uint16_t val);

/**
 * Reverses the byte order of a 32-bit unsigned integer.
 * Converts Big Endian <-> Little Endian.
 */
uint32_t swap_endian_32(uint32_t val);

/**
 * Reverses the byte order of a 64-bit unsigned integer.
 * Converts Big Endian <-> Little Endian.
 */
uint64_t swap_endian_64(uint64_t val);

/**
 * Checks at runtime if the host CPU is little-endian.
 * Returns true if little-endian, false if big-endian.
 */
bool is_system_little_endian(void);

/**
 * In-place byte swap for a buffer of 32-bit integers (e.g., network payload).
 */
void swap_endian_buffer_32(uint32_t *buf, size_t count);

#endif /* ENDIAN_SWAP_H */