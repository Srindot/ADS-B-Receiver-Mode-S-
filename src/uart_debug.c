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

  // Baud rate for 45 MHz APB1 clock (after PLL + APB1 /4 prescaler):
  // USARTDIV = 45MHz / (16 * 115200) = 24.4140625
  // Mantissa = 24 = 0x18, Fraction = 0.414 * 16 ≈ 7 → BRR = 0x0187
  UART2_BRR = 0x0187;

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

// Print one byte as two uppercase hex characters (e.g. 0x8D → "8D")
void uart2_send_hex_byte(unsigned char byte) {
  const char hex[] = "0123456789ABCDEF";
  uart2_send_char(hex[(byte >> 4) & 0x0F]);
  uart2_send_char(hex[byte & 0x0F]);
}

// Print an unsigned integer as a decimal string (e.g. 12345 → "12345")
void uart2_send_uint(unsigned int val) {
  char buf[11]; // max 10 digits for 32-bit uint + safety
  int i = 0;
  if (val == 0) {
    uart2_send_char('0');
    return;
  }
  // Extract digits in reverse order
  while (val > 0) {
    buf[i++] = '0' + (val % 10);
    val /= 10;
  }
  // Print digits in correct order
  while (i > 0) {
    uart2_send_char(buf[--i]);
  }
}
