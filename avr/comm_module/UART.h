#ifndef UART_H
#define UART_H

#include <inttypes.h>

#define UART_BUFFER_SIZE 32

void UART_init();
void UART_start();
uint8_t UART_read(uint8_t* s);
uint8_t UART_write(uint8_t *s, uint8_t len);

#endif
