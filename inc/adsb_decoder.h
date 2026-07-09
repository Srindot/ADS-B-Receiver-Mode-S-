/* adsb_decoder.h
 */

#ifndef ADSB_DECODER_H
#define ADSB_DECODER_H

#include <stdint.h>

void adsb_process_timestamp(unsigned int current_timestamp);

void adsb_crc_check(void);

// Debug counters — read from main loop to print periodic status
extern volatile unsigned int edge_count;
extern volatile unsigned int preamble_count;
extern volatile unsigned int crc_fail_count;
extern volatile unsigned int crc_pass_count;

#endif
