/* ============================================================================
 * File: serialize.c
 *
 * What is being built:
 *   Byte-level pointer serialization (`serialize`) and deserialization
 *   (`deserialize`) between a 32-bit unsigned integer (`uint32_t`) and a raw
 *   byte buffer (`uint8_t[4]`).
 *
 * What is being tested:
 *   Serializes `0x12345678` into a 4-byte array, prints the byte layout in
 *   memory order, clears the source integer, and deserializes the byte stream
 *   back into the `uint32_t` variable to verify round-trip fidelity.
 * ============================================================================ */

#include <limits.h>
#include <stdint.h>
#include <stdio.h>

void serialize(uint32_t *input, uint8_t *output)
{
  // Cast uint32_t address to byte pointer to copy each byte sequentially in memory order
  uint8_t *ptr = (uint8_t *)input;
  int size = sizeof(input);
  while(size > 0)
  {
    *output++ = *ptr++;
    size--;
  }
}

void deserialize(uint8_t *input, uint32_t *output)
{
  // Reconstruct the 32-bit word by copying bytes back into the target uint32_t memory address
  uint8_t *ptr = (uint8_t *)input;
  uint8_t *out = (uint8_t *)output;
  int size = sizeof(output);

  while(size > 0)
  {
    *out++ = *ptr++;
    size--;
  }
}

int main()
{
  uint32_t input = 0x12345678;
  uint8_t output[4] = {0};

  serialize(&input, &output[0]);
  printf("Serialized data for input %x is:\t", input);
  for(int i = 0; i < 4; i++)
  {
    printf("%x\t", output[i]);
  }
  printf("\n");

  input = 0x00;
  deserialize(&output[0], &input);
  printf("De-serialized data is: 0x%x", input);
  printf("\n");

  return 0;
}
