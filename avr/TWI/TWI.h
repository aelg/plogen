/** @file
 * Definerar konstanter och funktioner som definerats i TWI.c
 */

#ifndef TWI_H
#define TWI_H

#include <inttypes.h>

/// Längden på bufferterna.
#define TWI_BUFFER_SIZE 32

/// @name Adresser
///@{
#define COMM_ADDRESS 1
#define SENSOR_ADDRESS 2
#define CONTROL_ADDRESS 3
#define GENERAL_CALL 0
///@}

uint8_t TWI_write(uint8_t addr, uint8_t *s, uint8_t len);
uint8_t TWI_read(uint8_t* s);
void TWI_init(uint8_t sla);

#endif
