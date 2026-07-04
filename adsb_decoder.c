/* adsb_decoder.c used to decode the bit steam.
 */

unsigned int previous_timestamp;
int decoder_state;
unsigned int delta_history[DELTA_HISTORY_SIZE];
uint8_t payload[PAYLOAD_SIZE_BYTES]; // Array of size 14
int bit_counter = 0;

void adsb_process_timestamp(unsigned int current_timestamp) {}
