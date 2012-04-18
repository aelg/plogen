//STYRENHET.C
#undef F_CPU 
#define F_CPU 18400000UL 

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>

#include "../TWI/TWI.h"
#include "motor.h"
#include "../commands.h"

uint8_t autonomous;
uint8_t manual_command;
uint8_t diff;

//Initialize interrupts
void interrupts(void){
  // Set interrupt on rising edge of INT0 pin
	MCUCR = (MCUCR & 0xfc) | 0x03;
  // Set INT0 as input pin.
  DDRD = (DDRD & 0xfb) 
  //Enable interrupts on INT0
  GICR |= (1<<INT0);
}

//Interrupt routine
ISR(INT0_vect){
	autonomous = autonomous - 1;
	return;
}

//Manuell körning
void manual_control(uint8_t* s){
	switch(manual_command){
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

void jag_legger_det_har(void){
	OCR1A =	0x0003;//sets the length of pulses, right side - pin7
	OCR1B =	0x0003;//sets the length of pulses, left side - pin8
	
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
	
}


//Autonom körning
void auto_control(){

  if(mode == STRAIGHT){
    run_straight(diff);
  }
}

// Kontrollera meddelanden.
void check_TWI(){
	uint8_t s[10];
  uint8_t len;
  len = TWI_read(s);
  if(len){
    switch(s[0]){
    case CMD_SENSOR_DATA:
      for(uint8_t i = 2; i < len; i = i+2){
        if(s[i] == DIFF){
          diff = s[i+1];
        }
      }
      break;
    case CMD_MANUAL_CONTROL:
      autonomous = 0;
      manual_command = s[2];
      break;
    }
  }
}

//MAIN
int main(void)
{
	interrupts();
	setup_motor();
	TWI_init(CONTROL_ADDRESS);
	sei();

	// Loop
	while (1){


    // Check TWI bus.
    check_TWI();

    if(autonomous){
      auto_control();
    }
    else {
      manual_control();
    }
	}
}
