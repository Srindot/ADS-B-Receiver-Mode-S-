/* main.c
 *
 * purpose: Entry point. Initializes UART communication and Timer 1 input
 * capture, then idles while hardware interrupts handle data processing.
 */

#include "timer_capture.h"
#include "uart_debug.h"

void SystemInit(void) {}
void _init(void) {}
int main(void) {
  // 1. Initialize UART2 for USB serial communication (115200 Baud)
  uart2_init();
  // 2. Print startup message to verify serial connection
  uart2_send_string("\r\n====================================\r\n");
  uart2_send_string("  STM32 ADS-B Receiver Active\r\n");
  uart2_send_string("====================================\r\n");
  uart2_send_string("Listening on 1090 MHz...\r\n");
  // 3. Initialize Timer 1 and enable input capture interrupts on Pin PA8
  tim1_init();

  // 4. Infinite Loop
  // The main CPU remains idle here.
  // All pulse timing and ADS-B decoding is handled asynchronously
  // in the background by the hardware timer and the TIM1_CC_IRQHandler.
  while (1) {
    // Idle
  }

  return 0;
}
