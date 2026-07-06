/* uart_debug.c
 * purpose: Responsible for streaming the data through uart to host computer.
 */

// Reset Clock and control Register
#define RCC_BASE 0x40023800
#define RCC_AHB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x30))
#define RCC_APB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x40))

// Pin contorl
#define GPIOA_BASE 0x40020000
#define GPIOA_MODER (*(volatile unsigned int *)(GPIOA_BASE + 0x00))
#define GPIOA_AFRL (*(volatile unsigned int *)(GPIOA_BASE + 0x20))

// UART Registers
#define UART2_BASE 0x40004400
#define UART2_SR (*(volatile unsigned int *)(UART2_BASE + 0x00))
#define UART2_DR (*(volatile unsigned int *)(UART2_BASE + 0x04))
#define UART2_BRR (*(volatile unsigned int *)(UART2_BASE + 0x08))
#define UART2_CR1 (*(volatile unsigned int *)(UART2_BASE + 0x0C))

void uart2_init(void) {
  RCC_AHB1ENR |= (1 << 0);
  RCC_APB1ENR |= (1 << 17);

  GPIOA_MODER &= ~(3 << 4);
  GPIOA_MODER |= (2 << 4);

  //
  GPIOA_AFRL &= ~(0xF << 8);
  GPIOA_AFRL |= (7 << 8);

  // Set Buad Rate = 390.625
  UART2_BRR = 0x008B; // UART2_BRR = 0x0187;

  // Enable Tx
  UART2_CR1 = 0;
  UART2_CR1 |= (1 << 3) | (1 << 13);
}

void uart2_send_char(char c) {

  while ((UART2_SR & (1 << 7)) == 0) {
    // Wait until the last bit is cleared
  }

  UART2_DR = c;
}

void uart2_send_string(char *string) {
  while (*string) {
    uart2_send_char(*string++);
  }
}
