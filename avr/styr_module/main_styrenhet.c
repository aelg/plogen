//STYRENHET.C

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/sleep.h>
#include "TWI/TWI.h"

//Constants
#define CROSSING 0x00 //onödig?
#define CROSSING_LEFT 0x01
#define CROSSING_RIGHT 0x02
#define CROSSING_FORWARD 0x03

#define STRAIGHT 0x04

#define TURN 0x05 // onödig?
#define TURN_LEFT 0x06
#define TURN_RIGHT 0x07
#define TURN_FORWARD 0x08

#define MANUAL 0x09 // onödig?
#define MANUAL_LEFT 0x0A
#define MANUAL_RIGHT 0x0B
#define MANUAL_FORWARD 0x0C
#define MANUAL_BACK 0x0D
#define MANUAL_ROTATELEFT 0x0E
#define MANUAL_ROTATERIGHT 0x0F


//MAIN
int main(void)
{
uint8_t TWI_read(uint8_t* s);
void TWI_init(uint8_t sla);

volatile char c;
c=1;

// Loop
while (c=1){

	TWI_read(s);

	switch(s[0]){
	case CROSSING:
		switch(s[2]){
		case CROSSING_LEFT:
			rotate_left();
			break;
		case CROSSING_RIGHT:
			rotate_right();
			break;
		case CROSSING_FORWARD:
			drive_forward();
			break;
		}
	case STRAIGHT:
		run_straight(diff);
	case TURN:
		switch(s[2]){
		case TURN_LEFT:
			turn_left();
			break;
		case TURN_RIGHT:
			turn_right();
			break;
		case TURN_FORWARD:
			turn_forward();
			break;
		}
	case MANUAL
		switch(s[2]){
		case MANUAL_LEFT:
			turn_left();
			break;
	}
//	while(1){
//		asm volatile("nop");
//		}	
	}	
