#include <avr/interrupt.h>
#include <avr/io.h>

#include "../error/error.h"
#include "TWI.h"

//TWCR BITS(TWxx):   INT EA STA STO WC EN Res IE
#define SEND 0xc5 // 1   1  0   0   0  1  0   1
#define STOP 0xd5 // 1   1  0   1   0  1  0   1
#define START 0xe5// 1   1  1   0   0  1  0   1
#define ACK 0xc5  // 1   1  0   0   0  1  0   1
#define NACK 0x85 // 1   0  0   0   0  1  0   1
#define RESET 0xc5// 1   1  0   0   0  1  0   1

volatile uint8_t tread_buff[TWI_BUFFER_SIZE];
volatile uint8_t twrite_buff[TWI_BUFFER_SIZE];
volatile uint8_t tread_start, tread_end, twrite_start, twrite_end,
                 twrite_current, tread_current, tremaining_bytes;

/**
 * TWI calculate address: make sure the address is within queue.
 */
inline uint8_t TWIca(uint8_t);
uint8_t TWIca(uint8_t addr){
	return addr & 0x1f;
}

/**
 *  The write queue needs to know which address to send packets to.
 *  Put address to receiver before every packet.
 */

ISR(TWI_vect){
	uint8_t sr = TWSR & 0xf8;
	switch(sr){
	case 0x8: // Start sent.
	case 0x10: // Repeated start sent.
		// Copy twrite_start to current_start so we know what to send next,
		// let twrite_start remain as is, in case of arbitration error.
		twrite_current = twrite_start;
		// Read number of bytes in packet, assume there is a complete packet in the queue.
		// Add 3 to count SLA+W, command, and length.
		tremaining_bytes = twrite_buff[TWIca(twrite_start+2)]+2;
		// Load addressbyte, clear W/R to select write.
		TWDR = twrite_buff[twrite_current] << 1;
		TWCR = SEND;
		twrite_current = TWIca(twrite_current + 1);
		break;
	case 0x18: // SLA+W has been sent, ACK received.
	case 0x28: // Data has been sent, ACK received.
		TWDR = twrite_buff[twrite_current];
		// Check if this is the last byte in transmission.
		if(tremaining_bytes == 0){
			// Check if there are more packets to be sent.
			if(twrite_current == twrite_end){
				// Last byte, send STOP;
				TWCR = STOP;
			}
			else {
				// More to send, send START
				TWCR = START;
			}
			// Update start_twrite, there was no arbitration error.
			twrite_start = TWIca(twrite_current);
		}
		else {
			// Send byte.
			--tremaining_bytes;
			TWCR = SEND;
			twrite_current = TWIca(twrite_current+1);
		}
		break;
	case 0x38: // Arbitration error.
		// We still want to send what we was sending, send START when bus becomes free.
		TWCR = START;
		break;
	case 0x20: // Not ACK received after SLA+W, unknown address.
		error(TWI_UNKNOWN_ADDRESS);
		// Remove current packet.
		while(tremaining_bytes--)
			++twrite_current;

		twrite_start = TWIca(twrite_current);

		// Check if there are more packets.
		if(twrite_start == twrite_end){
			// No more packets, send STOP.
			TWCR = STOP;
		}
		else{
			// More packets in queue, send REPEATED START.
			TWCR = START;
		}
		break;
	case 0x30: // Not ACK received after data.
		// Signal error
		error(TWI_NOT_ACK_RECEIVED);
		// Start over and that the receiver has more space in buffer this time.
		TWCR = START;
		break;
	case 0x60: // Own SLA+W received, ACK returned.
	case 0x68: // Arbitration lost in MR mode, own SLA+W received, ACK returned.
	case 0x70: // General call address received.
	case 0x78: // Arbitration lost in MR mode, general call address received, ACK returned.
		// Read byte
		//tread_buff[tread_current] = TWDR;
		tread_current = TWIca(tread_end);
		// Check if buffer is full.
		if(TWIca(tread_current+1) == tread_start){
			// Return NOT ACK.
			TWCR = NACK;
		}
		else { // Send ACK
			TWCR = ACK;
		}
		break;
	case 0x80: // Data received, ACK sent. (Addressed)
	case 0x90: // Data received, ACK sent. (General call)
		// Read byte
		tread_buff[tread_current] = TWDR;
		tread_current = TWIca(tread_current+1);
		// Check if buffer is full.
		if(tread_current == tread_start){
			// Return NOT ACK.
			TWCR = NACK;
		}
		else { // Send ACK
			TWCR = ACK;
		}
		break;
	case 0x88: // Data received, NACK sent. (Addressed)
	case 0x98: // Data received, NACK sent. (General call)
		// Reset and hope that we have room in buffer next time.
		TWCR = RESET;
		error(TWI_READ_BUFFER_FULL);
		break;
	case 0xA0: // STOP received.
		tread_end = tread_current;
		//Check if we want to send data.
		if(twrite_start != twrite_end)
			TWCR = START;
		else
			TWCR = RESET;
		break;
	case 0x00: // Bus error due to illegal START or STOP
		// Send STOP to reset hardware, no STOP will be sent.
		error(TWI_BUS_ERROR);
		TWCR = STOP;
		break;
	default: // This shouldn't happen;
		error(TWI_UNKNOWN_STATUS);
		break;
	}
}

void TWI_start(){
	TWCR = START;
}

void TWI_init(uint8_t sla){
	tread_start = tread_end = twrite_start = twrite_end = 0;
	// Set correct bitrate.
	TWBR = 0x0c;
	// Set slave address and receive general calls.
	TWAR = (sla << 1) | 0x01;
	// Just to make sure.
	TWSR = 0;
	// Reset to make TWI ready for use.
	TWCR = RESET;
}
	

uint8_t TWI_write(uint8_t addr, uint8_t *s, uint8_t len){
	// Temporary end to handle circular buffer.
	uint8_t end;
	// Make sure end always is bigger than twrite_start.
	if(twrite_start > twrite_end){
		end = twrite_end + TWI_BUFFER_SIZE;
	}
	else end = twrite_end;
	// Return false if the message doesn't fit in buffer.
	if(len + 1> TWI_BUFFER_SIZE - 1 -(end - twrite_start)){
		// TODO: signal buffer error.
		return 0;
	}
	twrite_buff[twrite_end] = addr;
	twrite_end = TWIca(twrite_end+1);
	// Copy message to buffer.
	for(int i = 0; i < len; ++i){
		twrite_buff[twrite_end] = s[i];
		twrite_end = TWIca(twrite_end+1);
	}
	TWI_start();
	return 1;
}

uint8_t TWI_read(uint8_t* s){
	// Temporary variable to handle circular list.
	uint8_t end;
	// Calculate the length of the circular list.
	if(tread_end < tread_start)
		end = tread_end + TWI_BUFFER_SIZE;
	else end = tread_end;
	// Check if tread_buff contains correct number of bytes.
	if(end-tread_start >= 2){
		// Read length byte from packet.
		uint8_t len = tread_buff[TWIca(tread_start+1)]+2;
			// Check if correct number of bytes in read_buff
			if(len <= end-tread_start){
				// Copy the bytes to the list s
				for(uint8_t i = 0; i < len; i++){
					s[i]=tread_buff[tread_start];
					tread_start =TWIca(tread_start + 1);
					}
				//return the length of the list
				return len;
			}
			else return 0;
		}
		else return 0;
}
