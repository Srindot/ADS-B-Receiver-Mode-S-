#ifndef UART_DEBUG_H
#define UART_DEBUG_H

// Function Definations
void uart2_init(void);
void uart2_send_char(char);
void uart2_send_string(char *string);
void uart2_send_hex_byte(unsigned char byte);
void uart2_send_uint(unsigned int val);

#endif
