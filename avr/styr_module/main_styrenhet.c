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

uint8_t autonomous = 0;
uint8_t manual_command = STOP;
uint8_t diff = 127;
uint8_t mode = STRAIGHT;
uint8_t tape_position = 5;
uint8_t num_diods = 0;
uint8_t way_home[1]; //här sparas hur vi har kört på väg in i labyrinten

uint8_t rot = 5;
uint8_t k_p = REG_P;
uint8_t k_d = REG_D;

ISR(BADISR_vect){ // Fånga felaktiga interrupt om något går snett.
	volatile uint8_t c;
	while(1) ++c;
}

//Initialize interrupts
void interrupts(void){
  // Set interrupt on rising edge of INT0 pin
  MCUCR = (MCUCR & 0xfc) | 0x03;
  // Set INT0 as input pin.
  DDRD = (DDRD & 0xfb);
  //Enable interrupts on INT0
  GICR |= (1<<INT0);
}

//Interrupt routine
ISR(INT0_vect){
	autonomous = 1 - autonomous;
	return;
}


//Manuell körning
void manual_control(){
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
    run_straight(diff, rot, k_p, k_d);
  }
  if(mode == LINE_FOLLOW){
  	line_follow(num_diods, tape_position);
  }
}


// Kontrollera meddelanden.
void check_TWI(){
  uint8_t s[16];
  uint8_t len;
  len = TWI_read(s);
  if(len){
    switch(s[0]){
    case CMD_SENSOR_DATA:
      for(uint8_t i = 2; i < len; i = i+2){
        if(s[i] == IRDIFF){
          diff = s[i+1];
        }
		    if(s[i] == LINE_POSITION){
          tape_position = s[i+1];
        }
		    if(s[i] == DIOD){
          num_diods = s[i+1];
		    }
		    if(s[i] == IRROT){
          rot = s[i+1];
        }
      }
      break;
    case CMD_MANUAL:
      autonomous = 0;
      manual_command = s[2];
      break;
	case CMD_SET_REG_PARAMS:
	  for(uint8_t i = 2; i < len; i = i+2){
        if(s[i] == REG_P){
          k_p = s[i+1];
        }
	    if(s[i] == REG_D){
          k_d = s[i+1];
        }
      }
      break;
	  case CMD_AUTO_ON:
      autonomous = 1;
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
