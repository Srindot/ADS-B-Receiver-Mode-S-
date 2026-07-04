/* timer_capture.c
 *
 * purpose : Sets up GPIO PA8, configures Timer 1 for Input Capture, enables
 * hardware interrupts.
 *
 * Functions:
 * - tim1_init :
 * - TIM1_CC_IRQHandler
 */

// Include the Header file for the Function Definition
#include "timer_capture.h"

// ==============================================================================
// Registers Definition
// ==============================================================================
//
// Reset Clock and Control Registers
// Base Definition
#define RCC_BASE 0x40023800
// Advanced High-performance Bus 1 Enable Register : for the AHB1 bus Peripheral
// Clock Enable Register
#define RCC_AHB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x30))
// Advanced Peripheral Bus 2 Enable Register : RCC_APB2ENR stands for Reset and
// Clock Control register APB2 bus Peripheral Clock Enable Register.
#define RCC_APB2ENR (*(volatile unsigned int *)(RCC_BASE + 0x44))

// GPIO Register Base
#define GPIOA_BASE 0x40020000
// The GPIO Mode Register
// Input (00), an Output (01), an Alternate Function (10), or an Analog pin (11)
#define GPIOA_MODER (*(volatile unsigned int *)(GPIOA_BASE + 0x00))
// Alternate Function Register High : to decide which internal engine the pin
// should connect to
#define GPIOA_AFRH (*(volatile unsigned int *)(GPIOA_BASE + 0x24))

// timer 1 Base
#define TIM1_BASE 0x40010000
//  Control Register 1 : The power control for the Timer 1
#define TIM1_CR1 (*(volatile unsigned int *)(TIM1_BASE + 0x00))
// DMA/Interrupt Enable Register : Register which configures the Timer to send a
// interrupt to the processor
#define TIM1_DIER (*(volatile unsigned int *)(TIM1_BASE + 0x0C))
// State Register : raises the flag when it detects a pulse
#define TIM1_SR (*(volatile unsigned int *)(TIM1_BASE + 0x10))
// Capture/Compare Mode Register 1 : Mode register to set to generate signals or
// listen to signals
#define TIM1_CCMR1 (*(volatile unsigned int *)(TIM1_BASE + 0x18))
// Capture/Compare Enable Register: Either to listen falling/rising/both
// voltage.
#define TIM1_CCER (*(volatile unsigned int *)(TIM1_BASE + 0x20))
// Prescaler : The speed math divider. It slows down the 180 MHz master clock to
// whatever speed the timer needs.
#define TIM1_PSC (*(volatile unsigned int *)(TIM1_BASE + 0x28))
// Auto-Reload Register : The ceiling. When the timer counts up to this number,
// it rolls back over to 0.
#define TIM1_ARR (*(volatile unsigned int *)(TIM1_BASE + 0x2C))
// Capture/Compare Register 1 : The camera memory. When a pulse hits the pin,
// the timer instantly copies its current count into this register
#define TIM1_CCR1 (*(volatile unsigned int *)(TIM1_BASE + 0x34))

#define NVIC_BASE 0xE000E100
// NVIC_ISER0 (Nested Vectored Interrupt Controller - Interrupt Set-Enable
// Register 0): The main alarm panel for the CPU cortex. Even if the Timer sends
// an alarm, the CPU will ignore it unless this master panel allows it through.
#define NVIC_ISER0 (*(volatile unsigned int *)(NVIC_BASE + 0x00))

volatile unsigned int last_captured_time = 0;

void tim1_init(void) {

  // Turn power on to GPIO A
  RCC_AHB1ENR |= (1 << 0);
  // Turn power on for Timer 1
  RCC_APB2ENR |= (1 << 0);
  // Emptying + setting the pin to alternate function
  GPIOA_MODER &= ~(3 << 16);
  GPIOA_MODER |= (2 << 16);

  // Emptying + routing to timer1
  GPIOA_AFRH &= ~(0xF << 0);
  GPIOA_AFRH |= (1 << 0);

  // Divider for the clock
  TIM1_PSC = 17;
  // timer ceiling to 65,535
  TIM1_ARR = 0xFFFF;

  TIM1_CCMR1 = 0;         // Clean slate
  TIM1_CCMR1 |= (1 << 0); // Set to Input mode

  TIM1_CCER = 0;                               // Clean slate
  TIM1_CCER |= (1 << 1) | (1 << 3) | (1 << 0); // Both edges + Enable

  // trigger interupt when pulse detected
  TIM1_DIER |= (1 << 1);

  // interupt wired to alram slot 27
  NVIC_ISER0 |= (1 << 27);
  // Timer Start
  TIM1_CR1 |= (1 << 0);
}

void TIM1_CC_IRQHandler() {
  // Checking for incoming pulse
  if ((TIM1_SR & (1 << 1)) != 0) {
    // last captured pulse's time
    last_captured_time = TIM1_CCR1;
    // erasing for new bits
    TIM1_SR &= ~(1 << 1);
  }
}
