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
#include "../commands.h"


int main(void)
{
	UART_init();
	TWI_init(COMM_ADDRESS);
	sei();

	// Send greeting
	//uint8_t s[13] = {'P', 'l', 'o', 'g', 'e', 'n', 'P', 'l', 'o', 'g', 'e', 'n', '\n'};
	//UART_write(s, 13);

	//uint8_t start[3] = {CMD_MANUAL, 0x01, 0x0c};
	//uint8_t stop[3] = {CMD_MANUAL, 0x01, 0x10};
	uint8_t end[2] = {CMD_END, 0};
	

	uint8_t buff[10];
	while (1){
				int len;
				len = UART_read(buff);
				if(len){
					switch(buff[0]){
					case CMD_SEND_NEXT:
						UART_write(end, 2);
						break;
					case CMD_MANUAL:
						TWI_write(CONTROL_ADDRESS, buff, len);
						break;
					}
				}
				len = TWI_read(buff);
				if(len){
					switch(buff[0]){
					case CMD_SENSOR_DATA:
						UART_write(buff, len);
						break;
					}
				}
	}
}
