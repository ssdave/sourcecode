/* ============================================================================
 * File: register_union_overlay.c
 *
 * What is being built:
 *   Memory-mapped hardware register overlay using a C `union` (`register_access`)
 *   combining a full 32-bit word (`uint32_t value`) with four 8-bit bitfield
 *   slices (`val1`, `val2`, `val3`, `val4`).
 *
 * What is being tested:
 *   Initializes the 32-bit register to `0xdeadbeef`, overwrites each 8-bit
 *   sub-field (`0xf`, `0xe`, `0xd`, `0xc`), and inspects the resulting 32-bit
 *   composite register value (`0x0c0d0e0f` on little-endian architectures).
 * ============================================================================ */

#include <stdint.h>
#include <stdio.h>

typedef union
{
  uint32_t value;
  struct
  {
    uint32_t val1 : 8;
    uint32_t val2 : 8;
    uint32_t val3 : 8;
    uint32_t val4 : 8;
  } bits;
} register_access;

int main()
{
  register_access reg;
  reg.value = 0xdeadbeef;
  printf("Value read is %x\n", reg.value);
  reg.bits.val1 = 0xf;
  reg.bits.val2 = 0xe;
  reg.bits.val3 = 0xd;
  reg.bits.val4 = 0xc;
  printf("Val1 read is %x\n", reg.bits.val1);
  printf("Val2 read is %x\n", reg.bits.val2);
  printf("Val3 read is %x\n", reg.bits.val3);
  printf("Val4 read is %x\n", reg.bits.val4);
  printf("Value read is %x\n", reg.value);
  return 0;
}