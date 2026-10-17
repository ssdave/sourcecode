/* ============================================================================
 * File: ble_payload_parser.c
 *
 * The Scenario:
 *   BLE proximity tracker accessories broadcast their location via tiny BLE
 *   advertisement packets. Every bit is precious. The payload contains a
 *   rotated public key, a status flag, and a battery indicator, all packed
 *   together without standard byte alignment.
 *
 * What is being built:
 *   `parse_ble_payload()` — Unpacks a raw 31-byte BLE advertisement buffer into
 *   `struct BleTrackerPayload`:
 *     - Bytes 0-27 (224 bits): P-224 Elliptic Curve Public Key.
 *     - Byte 28, Bits 0-1 (2 bits): Device State (00 = Paired, 01 = Unpaired, 10 = Lost Mode).
 *     - Byte 28, Bits 2-7 (6 bits): Battery percentage (0-100).
 *     - Bytes 29-30 (16 bits): Little-endian 16-bit packet checksum.
 *
 * What is being tested:
 *   Constructs a synthetic 31-byte BLE packet (`0xAA` key bytes, `0xCA` packed
 *   status/battery byte encoding State=2 and Battery=50%, and `0xCD, 0xAB`
 *   little-endian checksum `0xABCD`) and verifies each extracted field.
 * ============================================================================ */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct BleTrackerPayload
{
  uint8_t public_key[28];
  uint8_t device_state;
  uint8_t battery_level;
  uint16_t checksum;
};

void parse_ble_payload(const uint8_t *raw_data, struct BleTrackerPayload *parsed_data)
{
  if((raw_data == NULL) || (parsed_data == NULL))
  {
    return;
  }

  // Bytes 0..27: Copy the 224-bit (28-byte) P-224 elliptic curve public key
  for(int i = 0; i < 28; i++)
  {
    parsed_data->public_key[i] = raw_data[i];
  }

  // Byte 28, bits [1:0]: Mask lowest 2 bits (0x03) for device state
  parsed_data->device_state = raw_data[28] & 0x3;
  // Byte 28, bits [7:2]: Mask upper 6 bits (0xFC) and shift right by 2 for battery (0..100%)
  parsed_data->battery_level = ((raw_data[28] & 0xFC) >> 2);

  // Bytes 29..30: Reassemble 16-bit little-endian checksum (LSB at byte 29, MSB at byte 30)
  parsed_data->checksum = (raw_data[29] | (raw_data[30] << 8));
}

// Test Harness (Verification)
int main()
{
  // Step A: Create our mock 31-byte raw payload initialized to 0
  uint8_t mock_payload[31] = {0};

  // Fill the public key section with dummy data (e.g., 0xAA)
  memset(mock_payload, 0xAA, 28);

  // Step B: Construct Byte 28
  // Let's test with: State = 2 (Lost Mode), Battery = 50
  // Battery 50 in binary is 110010. Shifted left by 2 = 11001000.
  // State 2 in binary is 00000010.
  // Combined: 11001000 | 00000010 = 11001010 (Hex: 0xCA)
  mock_payload[28] = 0xCA;

  // Step C: Construct Bytes 29-30 (Checksum)
  // Let's use a known target: 0xABCD
  // Since it's Little-Endian, the least significant byte (0xCD) goes first.
  mock_payload[29] = 0xCD;
  mock_payload[30] = 0xAB;

  // Step D: Run the function
  struct BleTrackerPayload parsed;
  parse_ble_payload(mock_payload, &parsed);

  // Step E: Verify the results
  printf("--- BLE Payload Verification ---\n");

  printf("Expected State: 2\t | Parsed State: %d \t ", parsed.device_state);
  if(parsed.device_state == 2)
  {
    printf("[PASS]\n");
  }
  else
  {
    printf("[FAIL]\n");
  }

  printf("Expected Battery: 50\t | Parsed Battery: %d \t ", parsed.battery_level);
  if(parsed.battery_level == 50)
  {
    printf("[PASS]\n");
  }
  else
  {
    printf("[FAIL]\n");
  }

  printf("Expected Checksum: ABCD | Parsed Checksum: %04X \t ", parsed.checksum);
  if(parsed.checksum == 0xABCD)
  {
    printf("[PASS]\n");
  }
  else
  {
    printf("[FAIL]\n");
  }

  return 0;
}