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

#include "UART.h"

int main(void)
{
	UART_init();
	sei();

	uint8_t s[13] = {'P', 'l', 'o', 'g', 'e', 'n', 'P', 'l', 'o', 'g', 'e', 'n', '\n'};

	while (1){
		UART_write(s, 13);
	}
}
