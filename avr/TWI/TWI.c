#include <avr/interrupt.h>
#include <avr/io.h>

#include "error.h"
#include "TWI.h"

//TWCR BITS(TWxx):   INT EA STA STO WC EN Res IE
#define SEND 0xc5 // 1   1  0   0   0  1  0   1
#define STOP 0xd5 // 1   1  0   1   0  1  0   1
#define START 0xe5// 1   1  1   0   0  1  0   1
#define ACK 0xc5  // 1   1  0   0   0  1  0   1
#define NACK 0x85 // 1   0  0   0   0  1  0   1
#define RESET 0xc5// 1   1  0   0   0  1  0   1

volatile uint8_t read_buff[TWI_BUFFER_SIZE];
volatile uint8_t write_buff[TWI_BUFFER_SIZE];
volatile uint8_t read_start, read_end, write_start, write_end,
                 write_current, read_current, remaining_bytes;

/**
 * TWI calculate address: make sure the address is within queue.
 */
inline uint8_t TWIca(uint8_t);
uint8_t TWIca(uint8_t addr){
	return addr % TWI_BUFFER_SIZE;
}

/**
 *  The write queue needs to know which address to send packets to.
 *  Put address to receiver before every packet.
 */

ISR(TWI_vect){
	uint8_t sr = TWSR & 0xfc;
	switch(sr){
	case 0x8: // Start sent.
	case 0x10: // Repeated start sent.
		// Copy write_start to current_start so we know what to send next,
		// let write_start remain as is, in case of arbitration error.
		write_current = write_start;
		// Read number of bytes in packet, assume there is a complete packet in the queue.
		// Add 3 to count SLA+W, command, and length.
		remaining_bytes = write_buff[TWIca(write_start+2)]+3;
		// Load addressbyte, clear W/R to select write.
		TWDR = write_buff[write_start] << 1;
		TWCR = SEND;
		write_current = TWIca(write_current + 1);
		break;
	case 0x18: // SLA+W has been sent, ACK received.
	case 0x28: // Data has been sent, ACK received.
		TWDR = write_buff[write_current];
		// Check if this is the last byte in transmission.
		if(remaining_bytes == 0){
			// Check if there are more packets to be sent.
			if(write_current == write_end){
				// Last byte, send STOP;
				TWCR = STOP;
			}
			else {
				// More to send, send START
				TWCR = START;
			}
			// Update start_write, there was no arbitration error.
			write_start = TWIca(write_current+1);
		}
		else {
			// Send byte.
			TWCR = SEND;
			write_current = TWIca(write_current+1);
		}
		break;
	case 0x38: // Arbitration error.
		// We still want to send what we was sending, send START when bus becomes free.
		TWCR = START;
		break;
	case 0x20: // Not ACK received after SLA+W, unknown address.
	case 0x30: // Not ACK received after data. THIS IS BAD WE ARE LOSING DATA!
		// Signal error
		if(sr == 20) error(TWI_UNKNOWN_ADDRESS);
		else if(sr == 30) error(TWI_NOT_ACK_RECEIVED);
		// Find next packet.
		while(remaining_bytes--)
			++write_current;

		write_start = TWIca(write_current);
		// Check if there are more packets.
		if(write_start == write_end){
			// No more packets, send STOP.
			TWCR = STOP;
		}
		else{
			// More packets in queue, send REPEATED START.
			TWCR = START;
		}
		break;
	case 0x60: // Own SLA+W received, ACK returned.
	case 0x68: // Arbitration lost in MR mode, own SLA+W received, ACK returned.
	case 0x70: // General call address received.
	case 0x78: // Arbitration lost in MR mode, general call address received, ACK returned.
		// Read byte
		read_buff[read_end] = TWDR;
		read_current = TWIca(read_end+1);
		// Check if buffer is full.
		if(read_current == read_start){
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
		read_buff[read_end] = TWDR;
		read_current = TWIca(read_current+1);
		// Check if buffer is full.
		if(read_current == read_start){
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
		read_end = read_current;
		//Check if we want to send data.
		if(write_start != write_end)
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
