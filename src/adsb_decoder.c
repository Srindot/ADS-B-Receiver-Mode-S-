#include "adsb_decoder.h"
// #include "uart_debug.h"

// --- Global State Variables ---
unsigned int previous_timestamp = 0;
int decoder_state = 0; // 0 = Preamble search, 1 = Data decoding
unsigned int delta_history[3] = {0, 0, 0};
uint8_t payload[14];
int bit_counter = 0;

// Standard ADS-B 24-bit generator polynomial
#define ADS_B_POLYNOMIAL 0xFFFA04

void adsb_process_timestamp(unsigned int current_timestamp) {
  // Calculate pulse gap in 0.1us ticks
  unsigned int delta = current_timestamp - previous_timestamp;

  // Handle 16-bit timer overflow
  if (current_timestamp < previous_timestamp) {
    delta += 0xFFFF;
  }
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
    // uart2_send_string("Valid Aircraft Data Intercepted!\r\n");
  }
}
