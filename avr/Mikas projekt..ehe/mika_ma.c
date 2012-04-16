#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include "../TWI/TWI.h"
#include "../styr_module/motor.h"
#include "../commands.h"

uint8_t s[10];
int x = 0;
int k_p = 1;
uint8_t difference;
int pdreg_value = 0;

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


//Funktion för pd-reglering
void pdreg(void){

	pdreg_value = k_p*(difference - 0b10000000);	

	if(pdreg_value < 0){

		OCR1A =	0x00F0;//sets the length of pulses, left side - pin7
		OCR1B =	0x00F0+pdreg_value;//sets the length of pulses, right side - pin8
	}
	else{
		OCR1A =	0x00F0-pdreg_value;//sets the length of pulses, left side - pin7
		OCR1B =	0x00F0;//sets the length of pulses, right side - pin8
	}
	
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
	
	return;
}


//autonom 		
void auto_control(void){
	pdreg();
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


