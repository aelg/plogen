/*
 * main.c
 *
 *  Created on: 15 mar 2012
 *      Author: aelg
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/sleep.h>

#include "../UART/UART.h"

int main(void)
{
	UART_init();
	sei();

	// Send greeting
	uint8_t s[13] = {'P', 'l', 'o', 'g', 'e', 'n', 'P', 'l', 'o', 'g', 'e', 'n', '\n'};
	//UART_write(s, 13);

	uint8_t buff[10];

	while (1){
		uint8_t len = UART_read(buff);
		if(len != 0){
			// Write greeting again and send back what was received.
			UART_write(s, 13);
			UART_write(buff+2, len-2);
		}
	}
}
