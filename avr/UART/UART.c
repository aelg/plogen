#include <avr/interrupt.h>
#include <avr/io.h>

#include "error.h"
#include "UART.h"

volatile uint8_t read_buff[UART_BUFFER_SIZE];
volatile uint8_t write_buff[UART_BUFFER_SIZE];
volatile uint8_t read_start, read_end, write_start, write_end;

/**
 * TWI calculate address: make sure the address is within queue.
 */
inline uint8_t UARTca(uint8_t);
uint8_t UARTca(uint8_t addr){
	return addr & 0x1f;
}

/**
 * Interrupt vector for UDR Empty. UDR is empty and ready to receive a new byte.
 * Sends next byte from the USART packet queue.
 */
ISR(USART_UDRE_vect){
	// Is there more data to send.
	if (write_start != write_end){
		// Write data to RXE to send it.
		UDR = write_buff[write_start];
		// Update buffer pointer.
		write_start = UARTca(write_start+1);
	}
	else {
	    // Disable UDRIE to stop sending data.
		UCSRB = UCSRB & 0xdf;
	}
}

/**
 * Interrupt vector for Rx Complete. UDR contains last recieved byte and is ready to be read.
 * Copy UDR, to USART packet queue.
 */
ISR(USART_RXC_vect){
	// Is the buffer full?
	if(UARTca(read_start+1) == read_end){
		error(UART_RXE_BUFFER_FULL);
		// Read UDR to clear interrupt.
		// FIXME: THIS BYTE WILL BE LOST AND WE WILL BE OUT OF SYNC WITH PACKETS.
		asm volatile("ldi r24, 0x2c" : : : "r24");
	}
	else{
		// Copy UDR to read_buffer.
		read_buff[read_end] = UDR;
		// Update pointer.
		read_end = UARTca(read_end+1);
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

/**
 * Write packet to UART packet queue.
 * Parameters:
 * uint8_t* s buffer to receive the packet.
 * Return value: Length of packet, 0 if no complete packet is in the buffer.
 */
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
		write_end = UARTca(write_end+1);
	}
	UART_start();
	return 1;
}

/**
 * Read first packet from the UART packet queue.
 * Parameters:
 * uint8_t* s buffer to receive the packet.
 * Return value: Length of packet, 0 if no complete packet is in the buffer.
 */
uint8_t UART_read(uint8_t* s){
	// Temporary variable to handle circular list.
	uint8_t end;
	// Calculate the length of the circular list.
	if(read_end < read_start)
		end = read_end + UART_BUFFER_SIZE;
	else end = read_end;
	// Check if read_buff contains correct number of bytes.
	if(end-read_start >= 2){
		// Read length byte from packet.
		uint8_t len = read_buff[UARTca(read_start+1)]+2;
			// Check if correct number of bytes in read_buff
			if(len >= end-read_start){
				// Copy the bytes to the list s
				for(uint8_t i = 0; i < len; i++){
					s[i]=read_buff[read_start];
					read_start =UARTca(read_start + 1);
					}
				//return the length of the list
				return len;
			}
			else return 0;
		}
		else return 0;
}
