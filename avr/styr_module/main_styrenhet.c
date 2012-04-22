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
uint8_t rot = 5;

ISR(BADISR_vect){ // Fånga felaktiga interrupt om något går snett.
	volatile uint8_t c;
	while(1) ++c;
}

//Initialize interrupts
void interrupts(void){
  // Set interrupt on rising edge of INT0 pin
  MCUCR = (MCUCR & 0xfc) | 0x03;
  // Set INT0 as input pin.
  DDRD = (DDRD & 0xf3);
  //Enable interrupts on INT0
  GICR |= (1<<INT0);
}

//Interrupt routine
ISR(INT0_vect){
	autonomous = 1 - autonomous;
	return;
}

//Interrupt routine for changing sensormode on interrupt from sensor
ISR(INT1_vect){

	mode = PORTB & 0b00000111;
	
	switch(mode){
		case CROSSING_LEFT:
			send_sensor_mode(CROSSING_LEFT);
			break;
		case CROSSING_RIGHT:
			send_sensor_mode(CROSSING_RIGHT);
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
	}
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


//Autonom körning
void auto_control(){

	switch(mode){
		case CROSSING_LEFT:
			send_sensor_mode(CROSSING_LEFT);
			break;
		case CROSSING_RIGHT:
			send_sensor_mode(CROSSING_RIGHT);
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

  if(mode == STRAIGHT){
    run_straight(diff);
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
		if(s[i] == IRROT){
          rot = s[i+1];
        }
      }
      break;
    case CMD_MANUAL:
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
