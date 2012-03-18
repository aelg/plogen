#ifndef ERROR_H
#define ERROR_H

#include <inttypes.h>

#define UART_RXE_BUFFER_FULL 1
#define TWI_UNKNOWN_ADDRESS 2
#define TWI_READ_BUFFER_FULL 3
#define TWI_NOT_ACK_RECEIVED 4
#define TWI_BUS_ERROR 5
#define TWI_UNKNOWN_STATUS 6

void error(uint8_t e);

#endif
