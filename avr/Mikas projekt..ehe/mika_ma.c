#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include "../TWI/TWI.h"
#include "../styr_module/motor.h"
#include "../commands.h"

uint8_t s[10];
int x = 0;

void manual_control(void){
	uint8_t len = TWI_read(s);
	if(len){
		switch(s[0]){
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

//autonom 		
void auto_control(void){
	rotate_right();
	return;
}
	

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
	interrupts();
	setup_motor();
	TWI_init(CONTROL_ADDRESS);
	while(1){
		if(x == 0)
			manual_control();
		else auto_control();
		}
}


