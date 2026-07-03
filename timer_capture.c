/* timer_capture.c
 * purpose : Sets up GPIO PA8, configures Timer 1 for Input Capture, enables
 * hardware interrupts.
 */
#include "timer_capture.h"
#define RCC_BASE 0x40023800
#define RCC_AHB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x30))
#define RCC_APB2ENR (*(volatile unsigned int *)(RCC_BASE + 0x44))

#define GPIOA_BASE 0x40020000
#define GPIOA_MODER (*(volatile unsigned int *)(GPIOA_BASE + 0x00))
#define GPIOA_AFRH (*(volatile unsigned int *)(GPIOA_BASE + 0x24))

#define TIM1_BASE 0x40010000
#define TIM1_CR1 (*(volatile unsigned int *)(TIM1_BASE + 0x00))
#define TIM1_DIER (*(volatile unsigned int *)(TIM1_BASE + 0x0C))
#define TIM1_SR (*(volatile unsigned int *)(TIM1_BASE + 0x10))
#define TIM1_CCMR1 (*(volatile unsigned int *)(TIM1_BASE + 0x18))
#define TIM1_CCER (*(volatile unsigned int *)(TIM1_BASE + 0x20))
#define TIM1_PSC (*(volatile unsigned int *)(TIM1_BASE + 0x28))
#define TIM1_ARR (*(volatile unsigned int *)(TIM1_BASE + 0x2C))
#define TIM1_CCR1 (*(volatile unsigned int *)(TIM1_BASE + 0x34))

#define NVIC_BASE 0xE000E100
#define NVIC_ISER0 (*(volatile unsigned int *)(NVIC_BASE + 0x00))

volatile unsigned int last_captured_time = 0;

void tim1_init(void) {
  RCC_AHB1ENR |= (1 << 0);
  RCC_APB2ENR |= (1 << 0);

  GPIOA_MODER &= ~(3 << 16);
  GPIOA_MODER |= (2 << 16);
  GPIOA_AFRH &= ~(0xF << 0);
  GPIOA_AFRH |= (1 << 0);

  TIM1_PSC = 17;
  TIM1_ARR = 0xFFFF;

  TIM1_CCMR1 = 0;         // Clean slate
  TIM1_CCMR1 |= (1 << 0); // Set to Input mode

  TIM1_CCER = 0;                               // Clean slate
  TIM1_CCER |= (1 << 1) | (1 << 3) | (1 << 0); // Both edges + Enable

  TIM1_DIER |= (1 << 1);
  NVIC_ISER0 |= (1 << 27);

  TIM1_CR1 |= (1 << 0);
}

void TIM1_CC_IRQHandler() {
  if ((TIM1_SR & (1 << 1)) != 0) {

    last_captured_time = TIM1_CCR1;
    TIM1_SR &= ~(1 << 1);
  }
}
