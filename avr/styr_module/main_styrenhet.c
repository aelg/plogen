//STYRENHET.C
#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>

#include "../TWI/TWI.h"
#include "motor.h"
#include "../commands.h"

uint8_t s[10];
int x = 0;

//Initialize interrupts
void interrupts(void){
	sei();
	MCUCR |= (MCUCR & 0xfc) | ((0<<ISC00) | (1<<ISC01));
    GICR |= (1<<INT0);
}

//Interrupt routine
ISR(INT0_vect){
	x = 1-x;
	return;
}

//MAIN
int main(void)
{
//	uint8_t TWI_read(uint8_t* s);
//	void TWI_init(uint8_t sla);
	interrupts();
	setup_motor();
	TWI_init(CONTROL_ADDRESS);

	uint8_t s[10];

	// Loop
	while (1){


		uint8_t len = TWI_read(s);

	if(len){

		if(x == 0){

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
		}
		else{
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
			break;
			case STRAIGHT:
				run_straight(s[2]);
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
				break;
			}
			}
		}
	}
}

