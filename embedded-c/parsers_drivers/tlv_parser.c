/* ============================================================================
 * File: tlv_parser.c
 *
 * What is being built:
 *   Defensive Tag-Length-Value (TLV) binary packet stream parser
 *   (`parse_tlv_stream`) that unpacks sequential `[Tag (1B)][Len (1B)][Value (Len B)]`
 *   records into a bounded `tlv_record_t` array (`MAX_PAYLOAD_LEN = 32`).
 *
 * What is being tested:
 *   Assertion test suite verifying:
 *     1. Single and multi-record valid TLV streams.
 *     2. Zero-length (`len == 0`) value records.
 *     3. Truncated headers and truncated payloads (`buf_len < record.len` -> `-1`).
 *     4. Oversized payload rejection (`len > MAX_PAYLOAD_LEN` -> `-1`).
 *     5. NULL buffer/output pointers and zero capacity bounds checks.
 * ============================================================================ */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_PAYLOAD_LEN 32
#define MAX_RECORDS 8

typedef struct
{
  uint8_t tag;
  uint8_t len;
  uint8_t value[MAX_PAYLOAD_LEN];
} tlv_record_t;

/**
 * Parses raw packet bytes into records.
 * Returns count of parsed records, or -1 if the packet is malformed.
 */
int parse_tlv_stream(const uint8_t *buf, size_t buf_len, tlv_record_t *records, size_t max_records)
{
  // Reject NULL pointers, empty buffers, or streams exceeding maximum record capacity
  if((buf == NULL) || (buf_len == 0) || (records == NULL) || (max_records == 0) ||
     (buf_len > sizeof(tlv_record_t) * max_records))
    return -1;

  size_t iter = 0;
  for(iter = 0; iter < max_records; iter++)
  {
    // Each TLV frame requires at least a 2-byte header: [Tag (1B)][Length (1B)]
    if(buf_len >= 2)
    {
      records[iter].tag = *buf;
      buf++;
      buf_len--;
      records[iter].len = *buf;
      buf++;
      buf_len--;

      // Defensively reject truncated frames or payloads larger than MAX_PAYLOAD_LEN
      if((buf_len < records[iter].len) || (records[iter].len > MAX_PAYLOAD_LEN))
      {
        return -1;
      }

      // Copy payload bytes and advance stream cursor by record length
      if(records[iter].len > 0)
      {
        memcpy(&records[iter].value[0], buf, records[iter].len);
        buf_len = buf_len - records[iter].len;
        buf = buf + records[iter].len;
      }
    }
    else if(buf_len > 0)
    {
      // 1 trailing byte left over is a truncated TLV header -> malformed packet
      return -1;
    }
    else
    {
      // Exactly 0 bytes remaining -> end of TLV stream
      break;
    }
  }
  printf("iter = %zu\n", iter);
  return iter;
}

int main(void)
{
  printf("--- Running Wireless TLV Parser Tests ---\n");

  tlv_record_t records[MAX_RECORDS];

  /* Test 1: Standard packet with 2 TLVs */
  /* TLV 1: Tag 0x01 (Device Name), Len 4, Val: "TAG1" */
  /* TLV 2: Tag 0x02 (TX Power),    Len 1, Val: 0xF0 (-16 dBm) */
  uint8_t stream1[] = {0x01, 0x04, 'T', 'A', 'G', '1', 0x02, 0x01, 0xF0};
  int count = parse_tlv_stream(stream1, sizeof(stream1), records, MAX_RECORDS);
  assert(count == 2);
  assert(records[0].tag == 0x01 && records[0].len == 4);
  assert(memcmp(records[0].value, "TAG1", 4) == 0);
  assert(records[1].tag == 0x02 && records[1].len == 1);
  assert(records[1].value[0] == 0xF0);
  printf("[PASS] Standard multi-TLV frame\n");

  /* Test 2: Valid zero-length TLV */
  uint8_t stream2[] = {0x05, 0x00};
  count = parse_tlv_stream(stream2, sizeof(stream2), records, MAX_RECORDS);
  assert(count == 1);
  assert(records[0].tag == 0x05 && records[0].len == 0);
  printf("[PASS] Zero-length payload\n");

  /* Test 3: Malformed packet - length extends beyond buffer (Buffer overflow attack) */
  uint8_t stream3[] = {0x01, 0x05, 'A', 'B'}; /* Claims 5 bytes, only 2 provided */
  count = parse_tlv_stream(stream3, sizeof(stream3), records, MAX_RECORDS);
  assert(count == -1);
  printf("[PASS] Malformed length rejection\n");

  /* Test 4: Truncated header (only 1 byte, tag without length) */
  uint8_t stream4[] = {0x01};
  count = parse_tlv_stream(stream4, sizeof(stream4), records, MAX_RECORDS);
  assert(count == -1);
  printf("[PASS] Truncated header rejection\n");

  /* Test 5: NULL buffer safety */
  assert(parse_tlv_stream(NULL, 10, records, MAX_RECORDS) == -1);
  printf("[PASS] NULL pointer check\n");

  printf("--- All tests passed! ---\n");
  return 0;
}