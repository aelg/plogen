//STYRENHET.C
#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>

<<<<<<< HEAD

//Constants
#define CROSSING 0x00 //on�dig? v�ntl�ge tom mitten?
#define CROSSING_LEFT 0x01
#define CROSSING_RIGHT 0x02
#define CROSSING_FORWARD 0x03

#define STRAIGHT 0x04

#define TURN 0x05 // on�dig?
#define TURN_LEFT 0x06
#define TURN_RIGHT 0x07
#define TURN_FORWARD 0x08

#define MANUAL 0x09 // on�dig?
#define MANUAL_LEFT 0x0A
#define MANUAL_RIGHT 0x0B
#define MANUAL_FORWARD 0x0C
#define MANUAL_BACK 0x0D
#define MANUAL_ROTATE_LEFT 0x0E
#define MANUAL_ROTATE_RIGHT 0x0F

#define ROTATION_COMPLETE 0x10 //on�dig?
=======
#include "../TWI/TWI.h"
#include "motor.h"
#include "../commands.h"
>>>>>>> fa147aff99211a71087ec5c53ac62d27095dcbcb


//MAIN
int main(void)
{
<<<<<<< HEAD

setup_motor();

uint8_t TWI_read(uint8_t* s);
void TWI_init(uint8_t sla);

// Loop
while (1){
=======

	setup_motor();
	TWI_init(CONTROL_ADDRESS);
	sei();

	uint8_t s[10];
>>>>>>> fa147aff99211a71087ec5c53ac62d27095dcbcb

	// Loop
	while (1){

		uint8_t len = TWI_read(s);
		if(len){
			switch(s[0]){
			/*case CROSSING:
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
			break;
			case STRAIGHT:
	//			run_straight(difference,direction);
				break;
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
				break;*/
			case CMD_MANUAL:
				switch(s[2]){
				case LEFT:
					manual_left();
					break;
				case RIGHT:
					manual_right();
					break;
				case FORWARD:
					manual_forward();
					break;
				case REVERSE:
					manual_reverse();
					break;
				case ROTATE_LEFT:
					rotate_left();
					break;
				case ROTATE_RIGHT:
					rotate_right();
					break;
				case STOP:
					manual_stop();
					break;
				}
				break;
			}
		}
<<<<<<< HEAD
	case STRAIGHT:
		run_straight(difference,direction);
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
			manual_left();
			break;
		case MANUAL_RIGHT:
			manual_right();
			break;
		case MANUAL_FORWARD:
			manual_forward();
			break;
		case MANUAL_BACK:
			manual_back();
			break;
		case MANUAL_ROTATE_LEFT:
			rotate_left();
			break;
		case MANUAL_ROTATE_RIGHT:
			rotate_right();
			break;
		}
	}
	
	}
		
}
//	while(1){
//		asm volatile("nop");
//		}	
	}	
=======
	}
		
}
	
>>>>>>> fa147aff99211a71087ec5c53ac62d27095dcbcb
