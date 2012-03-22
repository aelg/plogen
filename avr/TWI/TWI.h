#ifndef TWI_H
#define TWI_H

#include <inttypes.h>

#define TWI_BUFFER_SIZE 32

//Addresses
#define COMM_ADDRESS 0x01
#define SENSOR_ADDRESS 0x02
#define CONTROL_ADDRESS 0x03

uint8_t TWI_write(uint8_t addr, uint8_t *s, uint8_t len);
uint8_t TWI_read(uint8_t* s);
void TWI_init(uint8_t sla);

#endif
