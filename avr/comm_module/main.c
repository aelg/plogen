/*
 * main.c
 *
 *  Created on: 15 mar 2012
 *      Author: aelg
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>

//#include "../UART/UART.h"
#include "../TWI/TWI.h"

#define COMM_ADDRESS 0x01
#define SENSOR_ADDRESS 0x02

int main(void)
{
	//UART_init();
	TWI_init(COMM_ADDRESS);
	sei();

	// Send greeting
	//uint8_t s[13] = {'P', 'l', 'o', 'g', 'e', 'n', 'P', 'l', 'o', 'g', 'e', 'n', '\n'};
	//UART_write(s, 13);

	uint8_t send[6] = {2, 4, 'H', 'e', 'j', '!'};
	uint8_t recv[6] = {0, 0, '0', '0', '0', '0'};
	
	//for(volatile int i = 0; i < 0xff; ++i)
	//	for(volatile int j = 0; j < 0xff; ++j);

	TWI_write(SENSOR_ADDRESS, send, 6);
	while (1){
		TWI_read(recv);
	}
}
