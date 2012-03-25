//STYRENHET.C
#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>

#include "../TWI/TWI.h"
#include "motor.h"
#include "../commands.h"


//MAIN
int main(void)
{

	setup_motor();
	TWI_init(CONTROL_ADDRESS);
	sei();

	uint8_t s[10];

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
	}
		
}
	
