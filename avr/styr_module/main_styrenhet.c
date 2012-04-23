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
#include "../utility/send.h"

uint8_t autonomous = 0;
uint8_t manual_command = STOP;
uint8_t diff = 127;
uint8_t mode = STRAIGHT;
uint8_t tape_position = 5;
uint8_t num_diods = 0;
uint8_t way_home[1]; //h�r sparas hur vi har k�rt p� v�g in i labyrinten

uint8_t rot = 5;
uint8_t crossing_counter = 0;
uint8_t ir_long_left = 0;
uint8_t ir_long_right = 0;

uint8_t k_p = REG_P;
uint8_t k_d = REG_D;

ISR(BADISR_vect){ // F�nga felaktiga interrupt om n�got g�r snett.
	volatile uint8_t c;
	while(1) ++c;
}

//Initialize interrupts
void interrupts(void){
  // Set interrupt on rising edge of INT0 and INT1 pin
  MCUCR = (MCUCR & 0xf0) | 0x0f;
  // Set INT0 and INT1 as input pins.
  DDRD = (DDRD & 0xf3);
  //Enable interrupts on INT0 and INT1
  GICR |= 0b11000000;
}

//Interrupt routine
ISR(INT0_vect){
	autonomous = 1 - autonomous;
	return;
}

//Interrupt routine for changing sensormode on interrupt from sensor
ISR(INT1_vect){

	mode = PORTB & 0b00001111;
	
	switch(mode){
		case CROSSING_LEFT:
			//send_sensor_mode(CROSSING_LEFT);
			break;
		case CROSSING_RIGHT:
			//send_sensor_mode(CROSSING_RIGHT);
			break;
		case CROSSING_FORWARD:
			send_sensor_mode(CROSSING_FORWARD);
			break;
		case STRAIGHT:
			send_sensor_mode(STRAIGHT);
			break;
		case TURN:
			send_sensor_mode(TURN);
			break;
		case TURN_LEFT:
			send_sensor_mode(TURN_LEFT);
			break;
		case TURN_RIGHT:
			send_sensor_mode(TURN_RIGHT);
			break;
		case TURN_FORWARD:
			send_sensor_mode(TURN_FORWARD);
			break;
		case CROSSING:
			send_sensor_mode(CROSSING);
			break;
	}
}

//Routine to verify a crossing and decide which way to turn.
void check_crossing(void){

	//Kolla alla sensorer flera g�nger f�r att verifiera en sv�ng.
	if(crossing_counter < 0xff){
	
	//anv�nd	ir_long_left och ir_long_right
	}
	++crossing_counter;
		
}

	

//Manuell k�rning
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


//Autonom k�rning
void auto_control(){

	switch(mode){
		case CROSSING_LEFT:
			rotate_left();
			break;
		case CROSSING_RIGHT:
			rotate_right();
			break;
		case CROSSING_FORWARD:
			crossing_forward();
			break;
		case STRAIGHT:
			run_straight(diff, rot, K_P, K_D);
			break;
		case TURN:
			break;
		case TURN_LEFT:
			turn_left();
			break;
		case TURN_RIGHT:
			turn_right();
			break;
		case TURN_FORWARD:
			turn_forward();
			break;
		case CROSSING:
			check_crossing();
			break;

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
		if(s[i] == IR_LONG_LEFT){
          ir_long_left = s[i+1];
        }
		if(s[i] == IR_LONG_RIGHT){
          ir_long_right = s[i+1];
        }
      }
      break;
    case CMD_MANUAL:
      autonomous = 0;
      manual_command = s[2];
      break;
	  case CMD_REG_PARAMS:
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

// M�lomr�desk�rning

void end_of_the_line(){
	if(num_diods > 4){
		OCR1A = 0x0003;//sets the length of pulses, right side - pin7
		OCR1B = 0x0003;//sets the length of pulses, left side - pin8
		griparm();
	}
	else if(tape_position<4){
		turn_left();
	}
	else if(tape_position>=4 && tape_position<=6){
		turn_forward();
	}
	else {
		turn_right();
	}	
}




//Autonom k�rning
//void auto_control(){
//    end_of_the_line();
//}




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
