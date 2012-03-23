/*
 * main.c
 *
 *  Created on: 15 mar 2012
 *      Author: aelg
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>

#include "../UART/UART.h"
#include "../TWI/TWI.h"

#define CMD_OK = 0x1;

int main(void)
{
	UART_init();
	TWI_init(COMM_ADDRESS);
	sei();

	// Send greeting
	//uint8_t s[13] = {'P', 'l', 'o', 'g', 'e', 'n', 'P', 'l', 'o', 'g', 'e', 'n', '\n'};
	//UART_write(s, 13);

	uint8_t start[3] = {0x09, 0x01, 0x0c};
	uint8_t stop[3] = {0x09, 0x01, 0x10};
	

	uint8_t buff[10];
	//UART_write(s, 12);
	while (1){
		//for(volatile int i = 0; i < 0xff; ++i)
		//	for(volatile int j = 0; j < 0xff; ++j){
				//if(i == 0 && j == 0) UART_write(ok, 2);
				int len;
				len = UART_read(buff);
				if(len){
					TWI_write(CONTROL_ADDRESS, buff, len);
				}
		//	}
	}
	//uint8_t buff[10];
	//UART_write(s, 12);
	/*while (1){
		for(volatile int i = 0; i < 0xffff; ++i)
			for(volatile int j = 0; j < 0x000f; ++j);
		TWI_write(CONTROL_ADDRESS, start, 3);
		for(volatile int i = 0; i < 0xffff; ++i)
			for(volatile int j = 0; j < 0x000f; ++j);
		TWI_write(CONTROL_ADDRESS, stop, 3);
		while(1);
	}*/
}
