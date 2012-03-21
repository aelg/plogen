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

#define COMM_ADDRESS 0x01
#define SENSOR_ADDRESS 0x02
#define CONTROL_ADDRESS 0x03

#define CMD_OK = 0x1;

int main(void)
{
	UART_init();
	TWI_init(COMM_ADDRESS);
	sei();

	// Send greeting
	//uint8_t s[13] = {'P', 'l', 'o', 'g', 'e', 'n', 'P', 'l', 'o', 'g', 'e', 'n', '\n'};
	//UART_write(s, 13);

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
}
