#include "adsb_decoder.h"
#include "uart_debug.h"  // Bug #2 fix: was commented out, no output could ever print

// --- Global State Variables ---
unsigned int previous_timestamp = 0;
int decoder_state = 0; // 0 = Preamble search, 1 = Data decoding
unsigned int delta_history[3] = {0, 0, 0};
uint8_t payload[14];
int bit_counter = 0;

// Debug counters — incremented in ISR, read by main loop
volatile unsigned int edge_count = 0;
volatile unsigned int preamble_count = 0;
volatile unsigned int crc_fail_count = 0;
volatile unsigned int crc_pass_count = 0;

// Standard ADS-B / Mode S CRC-24 generator polynomial:
//   P(x) = x^24 + x^23 + x^10 + x^3 + 1
//   In 24-bit hex (without the implied x^24 leading 1): 0xFFF409
//
// Bug #7 fix: was 0xFFFA04 which has wrong bits set — every CRC would fail
// and all valid frames would be silently discarded.
#define ADS_B_POLYNOMIAL 0xFFF409

void adsb_process_timestamp(unsigned int current_timestamp) {
  // Calculate pulse gap in timer ticks (0.1 us at 10 MHz timer clock).
  //
  // Bug #6 fix: The old code did a 32-bit subtraction then added 0xFFFF on
  // overflow, which is off-by-one. Since both timestamps are 16-bit values
  // from a 16-bit counter (ARR=0xFFFF), masking the 32-bit subtraction result
  // to 16 bits gives the correct modular delta and naturally handles wrap-
  // around with no branch needed.
  unsigned int delta = (current_timestamp - previous_timestamp) & 0xFFFF;

  // Count every edge the ISR delivers to the decoder
  edge_count++;

  previous_timestamp = current_timestamp;

  // State 0: Preamble Detection
  if (decoder_state == 0) {
    delta_history[0] = delta_history[1];
    delta_history[1] = delta_history[2];
    delta_history[2] = delta;

    // Match expected intervals (10, 25, 12 ticks) with +/- 2 margin
    if ((delta_history[0] >= 8 && delta_history[0] <= 12) &&
        (delta_history[1] >= 23 && delta_history[1] <= 27) &&
        (delta_history[2] >= 10 && delta_history[2] <= 14)) {

      decoder_state = 1;
      bit_counter = 0;
      preamble_count++;
      for (int i = 0; i < 14; i++) {
        payload[i] = 0;
      }
    }
  }

  // State 1: PPM Payload Decoding
  else if (decoder_state == 1) {
    int byte_index = bit_counter / 8;
    int bit_index = 7 - (bit_counter % 8);

    // Early pulse (4-7 ticks) = Logic 1
    if (delta >= 4 && delta <= 7) {
      payload[byte_index] |= (1 << bit_index);
      bit_counter++;
    }
    // Late pulse (9-12 ticks) = Logic 0
    else if (delta >= 9 && delta <= 12) {
      bit_counter++;
    }
    // Timing violation, reset state
    else {
      decoder_state = 0;
    }

    // 112 bits received, trigger validation
    if (bit_counter == 112) {
      decoder_state = 0;
      adsb_crc_check();
    }
  }
}

void adsb_crc_check(void) {
  // Buffer payload to protect original data during division
  uint8_t temp_payload[14];
  for (int i = 0; i < 14; i++) {
    temp_payload[i] = payload[i];
  }

  // 24-bit sliding XOR window across the first 88 bits
  for (int i = 0; i < 88; i++) {
    int byte_index = i / 8;
    int bit_index = 7 - (i % 8);

    if ((temp_payload[byte_index] & (1 << bit_index)) != 0) {
      for (int j = 0; j < 24; j++) {
        if ((ADS_B_POLYNOMIAL & (1 << (23 - j))) != 0) {
          int target_byte = (i + j) / 8;
          int target_bit = 7 - ((i + j) % 8);
          temp_payload[target_byte] ^= (1 << target_bit);
        }
      }
    }
  }

  // Valid CRC results in 0x000000 for the final 3 bytes
  if (temp_payload[11] == 0 && temp_payload[12] == 0 && temp_payload[13] == 0) {
    crc_pass_count++;
    // Print the raw hex frame: "[ADS-B] 8D4840D6202CC371C32CE0576098"
    uart2_send_string("[ADS-B] ");
    for (int i = 0; i < 14; i++) {
      uart2_send_hex_byte(payload[i]);
    }
    uart2_send_string("\r\n");
  } else {
    crc_fail_count++;
  }
}
