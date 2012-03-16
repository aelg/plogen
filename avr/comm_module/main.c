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

#define UART_BUFFER_SIZE

char read_buff[UART_BUFFER_SIZE];
char write_buff[UART_BUFFER_SIZE];
char read_start, read_end, write_start, write_end;
bool writing;

ISR(USART_UDRE_vect){
	if (write_start != write_end){
		UDR = write_buff[read_start];
		read_start = (read_start+1) % UART_BUFFER_SIZE
	}
	else writing = false;
}

void USART_init(){
	// Set buffer pointers to beginning of buffers.
	read_start = read_end = write_start = write_end = 0;
	writing = false;
	// Init UCSRA just to make sure.
	USCRA = 0x0;
	// Write to UCSRA; set Asyncronous, no parity, 8 databits.
	UCSRC = 0x86;
	// Set baudrate 115200
	UBRR = 0x09;
	// Activate interrupts on UDRE and RX Complete.
	UCSRB = 0xb8;
}

void UART_start(){

}

int main(void)
{
	volatile char c;
	c=1;


	MCUCR = 0x03;
	GICR = 0x40;
	ADMUX = 0x20;
	ADCSRA = 0xAB;
	SFIOR = 0x40;
	sei();

	while (c){}

}
