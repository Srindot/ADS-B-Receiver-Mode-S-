/* adsb_decoder.h
 */

#ifndef ADSB_DECODER_H
#define ADSB_DECODER_H

#include <stdint.h>

void adsb_process_timestamp(unsigned int current_timestamp);

void adsb_crc_check(void);

#endif
