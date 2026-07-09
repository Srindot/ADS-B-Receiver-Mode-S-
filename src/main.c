/* main.c
 *
 * purpose: Entry point. Initializes UART communication and Timer 1 input
 * capture, then idles while hardware interrupts handle data processing.
 */

#include "rcc_clock.h"
#include "timer_capture.h"
#include "uart_debug.h"
#include "adsb_decoder.h"

// Read PA8 pin state directly (IDR always reflects actual electrical level)
#define GPIOA_IDR (*(volatile unsigned int *)(0x40020000 + 0x10))
// Read the live timer counter to verify TIM1 is actually ticking
#define TIM1_CNT (*(volatile unsigned int *)(0x40010000 + 0x24))

void SystemInit(void) {}
void _init(void) {}
int main(void) {
  // 1. Switch from default 16MHz HSI to 180MHz PLL (must happen before any
  //    peripheral init, since baud rates and timer prescalers depend on it)
  clock_setup();

  // Brief delay for clock tree to stabilize after PLL switch
  for (volatile unsigned int i = 0; i < 100000; i++);

  // 2. Initialize UART2 for USB serial communication (115200 Baud)
  uart2_init();
  // 3. Print startup message to verify serial connection
  uart2_send_string("\r\n====================================\r\n");
  uart2_send_string("  STM32 ADS-B Receiver Active\r\n");
  uart2_send_string("====================================\r\n");
  uart2_send_string("Listening on 1090 MHz...\r\n");
  // 4. Initialize Timer 1 and enable input capture interrupts on Pin PA8
  tim1_init();

  // 5. Main loop — print pipeline status every ~3 seconds so we can see
  //    exactly where the decode pipeline stalls (or succeeds).
  //    Edges=0 → no signal reaching PA8 (check antenna/comparator wiring)
  //    Preambles=0 → edges arrive but no ADS-B preamble pattern matched
  //    CRC Fail>0, Pass=0 → frames decoded but data is corrupted
  //    CRC Pass>0 → working! valid ADS-B frames printed above
  while (1) {
    // Crude delay: ~3 seconds at 180 MHz (each iteration ≈ ~6 cycles)
    for (volatile unsigned int d = 0; d < 90000000; d++);

    uart2_send_string("[STATUS] Edges:");
    uart2_send_uint(edge_count);
    uart2_send_string(" Preambles:");
    uart2_send_uint(preamble_count);
    uart2_send_string(" CRC Fail:");
    uart2_send_uint(crc_fail_count);
    uart2_send_string(" CRC Pass:");
    uart2_send_uint(crc_pass_count);
    uart2_send_string(" | PA8:");
    uart2_send_string((GPIOA_IDR & (1 << 8)) ? "HIGH" : "LOW");
    uart2_send_string(" TIM1:");
    uart2_send_uint(TIM1_CNT);
    uart2_send_string("\r\n");
  }

  return 0;
}
