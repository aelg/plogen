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

#define UART_BUFFER_SIZE 32

volatile uint8_t read_buff[UART_BUFFER_SIZE];
volatile uint8_t write_buff[UART_BUFFER_SIZE];
volatile uint8_t read_start, read_end, write_start, write_end;

ISR(USART_UDRE_vect){
	// Is there more data to send.
	if (write_start != write_end){
		// Write data to RXE to send it.
		UDR = write_buff[write_start];
		// Update buffer pointer.
		write_start = (write_start+1) % UART_BUFFER_SIZE;
	}
	else {
	    // Disable UDRIE to stop sending data.
		UCSRB = UCSRB & 0xdf;
	}
}

void UART_init(){
	// Set buffer pointers to beginning of buffers.
	read_start = read_end = write_start = write_end = 0;
	// Init UCSRA just to make sure.
	UCSRA = 0x0;
	// Write to UCSRA; set Asyncronous, no parity, 8 databits.
	UCSRC = 0x86;
	// Set baudrate 115200
	UBRRL = 0x09;
	// Activate interrupts on RX Complete.
	UCSRB = 0x98;
}

void UART_start(){
    // Enable interrupt on UDRE
    UCSRB = UCSRB | 0x20;
}

uint8_t UART_write(uint8_t *s, uint8_t len){
	// Temporary end to handle circular buffer.
	uint8_t end;
	// Make sure end always is bigger than write_start.
	if(write_start > write_end){
		end = write_end + UART_BUFFER_SIZE;
	}
	else end = write_end;
	// Return false if the message doesn't fit in buffer.
	if(len > UART_BUFFER_SIZE - 1 - (end - write_start)){
		// TODO: signal buffer error.
		return 0;
	}

	// Copy message to buffer.
	for(int i = 0; i < len; ++i){
		write_buff[write_end] = s[i];
		write_end = (write_end+1) % UART_BUFFER_SIZE;
	}
	UART_start();
	return 1;
}

int main(void)
{
	UART_init();
	sei();

	uint8_t s[13] = {'P', 'l', 'o', 'g', 'e', 'n', 'P', 'l', 'o', 'g', 'e', 'n', '\n'};

	while (1){
		UART_write(s, 13);
	}
}
