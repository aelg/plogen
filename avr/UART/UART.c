#include <avr/interrupt.h>
#include <avr/io.h>

#include "../error/error.h"
#include "UART.h"

// These are changed by ISRs and read by interuptable functions and should be volatile.
// Dont't really think this is necessary.
volatile uint8_t uread_buff[UART_BUFFER_SIZE];
volatile uint8_t uwrite_buff[UART_BUFFER_SIZE];
volatile uint8_t uread_end, uwrite_start;

// These are never changed by ISRs only read, and shouldn't need to be volatile.
uint8_t uread_start, uwrite_end;
// These are only used by interrupts.
uint8_t uremaining_bytes, uremaining_packets;

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
	if (uwrite_start != uwrite_end){
		// Write data to RXE to send it.
		UDR = uwrite_buff[uwrite_start];
		// Update buffer pointer.
		uwrite_start = UARTca(uwrite_start+1);
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
	if(uread_start == UARTca(uread_end+1)){
		error(UART_RXE_BUFFER_FULL);
		// Read UDR to clear interrupt.
		// FIXME: THIS BYTE WILL BE LOST AND WE WILL BE OUT OF SYNC WITH PACKETS.
		asm volatile("ldi r24, 0x2c" : : : "r24");
	}
	else{
		// Copy UDR to read_buffer.
		uread_buff[uread_end] = UDR;
		// Update pointer.
		uread_end = UARTca(uread_end+1);
	}
}

void UART_init(){
	// Set buffer pointers to beginning of buffers.
	uread_start = uread_end = uwrite_start = uwrite_end = 0;
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
	if(uwrite_start > uwrite_end){
		end = uwrite_end + UART_BUFFER_SIZE;
	}
	else end = uwrite_end;
	// Return false if the message doesn't fit in buffer.
	if(len > UART_BUFFER_SIZE - 2 - (end - uwrite_start)){
		// TODO: signal buffer error.
		return 0;
	}

	// Copy message to buffer.
	for(int i = 0; i < len; ++i){
		uwrite_buff[uwrite_end] = s[i];
		uwrite_end = UARTca(uwrite_end+1);
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
	if(uread_start == uread_end) return 0;
	// Temporary variable to handle circular list.
	uint8_t end;
	uint8_t start = uread_start;
	// Calculate the length of the circular list.
	if(uread_end < start)
		end = uread_end + UART_BUFFER_SIZE;
	else end = uread_end;
	// Check if read_buff contains correct number of bytes.
	if(end - start > 1){
		// Read length byte from packet.
		uint8_t len = uread_buff[UARTca(start+1)]+2;
			// Check if correct number of bytes in read_buff
			if(len <= end - start){
				// Copy the bytes to the list s
				for(uint8_t i = 0; i < len; i++){
					s[i]=uread_buff[start];
					start =UARTca(start + 1);
					}
				uread_start = start;
				//return the length of the list
				return len;
			}
			else return 0;
		}
		else return 0;
}
