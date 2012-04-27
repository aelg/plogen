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

#define LINE_START_MAX 0x4000

uint16_t line_start_timer = 0;
uint8_t driving_back = 0;
uint8_t autonomous = 0;
uint8_t manual_command = STOP;
uint8_t diff = 127;
uint8_t mode = MODE_STRAIGHT;
uint8_t tape_position = 5;
uint8_t num_diods = 0;

uint8_t way_home[20]; //här sparas hur vi har kört på väg in i labyrinten
uint8_t way_home_iterator = 0; //pekar på hur vi ska svänga i nästa kurva.

uint8_t last_tape_detected = 0; //Sparar senaste tejpmarkering

uint8_t rot = 5;
uint32_t crossing_timer = 0;
uint32_t crossing_timer_max = 0x5000;
uint8_t ir_long_left = 0;
uint8_t ir_long_right = 0;

uint8_t k_p = REG_P;
uint8_t k_d = REG_D;

ISR(BADISR_vect){ // Fånga felaktiga interrupt om något går snett.
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
	if (mode == MODE_CROSSING_FORWARD) return;
	uint8_t t_mode = PINB & 0b00001111;
	
	switch(t_mode){
		/*case MODE_TURN_LEFT:
			last_tape_detected = 3;
     		mode = MODE_STRAIGHT;
			break;
		case MODE_TURN_RIGHT:
			last_tape_detected = 2;
      		mode = MODE_STRAIGHT;
			break;
		case MODE_TURN_FORWARD:
			last_tape_detected = 1;
      		mode = MODE_STRAIGHT;
			break;*/
		case MODE_LINE_FOLLOW:
		 	num_diods = 1;
			tape_position = 5;
			line_start_timer = 0;
			mode = MODE_LINE_FOLLOW;
			send_sensor_mode(MODE_LINE_FOLLOW);
			break;
		case MODE_GYRO_COMPLETE:
			if(mode == MODE_TURN_AROUND){
				mode = MODE_CROSSING_LEFT;
				send_sensor_mode(MODE_GYRO);
			}
			else {
				crossing_timer = 0; // För att hindra roboten att tro 
										// att den är ute ur svängen för tidigt.
				mode = MODE_CROSSING_FORWARD;
				send_sensor_mode(MODE_STRAIGHT);
			}
			break;
    default:
      mode = MODE_CROSSING;
			crossing_timer = 0;
			break;
	}
}

//Routine to verify a crossing and decide which way to turn.
void check_crossing(void){
	manual_forward();
	++crossing_timer;
	if((PINB & 0b00001111) == MODE_STRAIGHT){
		mode = MODE_STRAIGHT; //Om vi inte var i korsning, fortsätt i MODE_STRAIGHT;
		return;
	}
	//Kolla alla sensorer flera gånger för att verifiera en sväng.
	if(crossing_timer > crossing_timer_max){ // 0xf4240 == 1000000
		if(driving_back){
			switch(way_home[way_home_iterator]){
			case 0:
				mode = MODE_FINISH;
				return;
			case 1:
				mode = MODE_CROSSING_LEFT;
				send_sensor_mode(MODE_GYRO);
				--way_home_iterator;
				return;
			case 2:
				mode = MODE_CROSSING_FORWARD;
				--way_home_iterator;
				return;
			case 3:
				mode = MODE_CROSSING_RIGHT;
				send_sensor_mode(MODE_GYRO);
				--way_home_iterator;
				return;
			}
		}
		switch(last_tape_detected){
		case 1:
			way_home[++way_home_iterator] = 2;
			mode = MODE_CROSSING_FORWARD;
			last_tape_detected = 0;
			return;
		case 2:
			way_home[++way_home_iterator] = 1;
			mode = MODE_CROSSING_RIGHT;
			send_sensor_mode(MODE_GYRO);
			last_tape_detected = 0;
			return;
		case 3:
			way_home[++way_home_iterator] = 3;
			mode = MODE_CROSSING_LEFT;
			send_sensor_mode(MODE_GYRO);
			last_tape_detected = 0;
			return;
		}
		switch(PINB & 0b00001111){
		case MODE_CROSSING_FORWARD: 
			crossing_timer = 0;
			way_home[++way_home_iterator] = 2;
			mode = MODE_CROSSING_FORWARD;
			return;
		case MODE_CROSSING_RIGHT:
			way_home[++way_home_iterator] = 1;
			mode = MODE_CROSSING_RIGHT;
			send_sensor_mode(MODE_GYRO);
			return;
		case MODE_CROSSING_LEFT:
			way_home[++way_home_iterator] = 3;
			mode = MODE_CROSSING_LEFT;
			send_sensor_mode(MODE_GYRO);
			return;
		}
	}		
}

/*
void check_crossing_way_back(void){
	manual_forward();
	++crossing_timer;
	if((PINB & 0b00001111) == MODE_STRAIGHT){
		mode = MODE_STRAIGHT; //Om vi inte var i korsning, fortsätt i MODE_STRAIGHT;
		return;
	}
	//Kolla alla sensorer flera gånger för att verifiera en sväng.
	if(crossing_timer > crossing_timer_max){ // 0xf4240 == 1000000
		switch(way_home[way_home_iterator]){
			case 0:
				mode = MODE_FINISH;
			case 1:
				mode = MODE_CROSSING_LEFT;
				send_sensor_mode(MODE_GYRO);
				--way_home_iterator;
			return;
			case 2:
				mode = MODE_CROSSING_FORWARD;
				--way_home_iterator;
			return;
			case 3:
				mode = MODE_CROSSING_RIGHT;
				send_sensor_mode(MODE_GYRO);
				--way_home_iterator;
			return;
		}
	}
}*/

//Manuell körning
void manual_control(){
	run_straight(diff, rot, k_p, k_d, FALSE);
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
			stop();
			break;
		
	}
}

//Autonom körning
void auto_control(){

	switch(mode){
		case MODE_TURN_AROUND:
		case MODE_CROSSING_LEFT:
			rotate_left();
			break;
		case MODE_CROSSING_RIGHT:
			rotate_right();
			break;
		case MODE_CROSSING_FORWARD:
			turn_forward();
			if (crossing_timer < (crossing_timer_max)){
				++crossing_timer;
				break;
			}
			else if((PINB & 0b00001111) == MODE_STRAIGHT) mode = MODE_STRAIGHT;
			break;
		case MODE_STRAIGHT:
			run_straight(diff, rot, k_p, k_d, TRUE);
			break;
		case MODE_CROSSING:
			check_crossing();
			break;
        case MODE_LINE_FOLLOW:
			if(line_start_timer < LINE_START_MAX){
				++line_start_timer;
				turn_forward();
				break;
			}
			if(line_follow(num_diods, tape_position)){
				mode = MODE_TURN_AROUND;
				send_sensor_mode(MODE_GYRO);
				driving_back = 1;
			}
			break;
		case MODE_FINISH:
			stop();
			while(1);
			break;
	}
}

/*
//Tillbakavägskörning
void way_back(){

	switch(mode){
		case MODE_TURN_AROUND:
		case MODE_CROSSING_LEFT:
			rotate_left();
			break;
		case MODE_CROSSING_RIGHT:
			rotate_right();
			break;
		case MODE_CROSSING_FORWARD:
			turn_forward();
			if (crossing_timer < (crossing_timer_max)){
				++crossing_timer;
				break;
			}
			else if((PINB & 0b00001111) == MODE_STRAIGHT) mode = MODE_STRAIGHT;
			break;
		case MODE_STRAIGHT:
			run_straight(diff, rot, k_p, k_d, TRUE);
			break;
		case MODE_CROSSING:
			check_crossing_way_back();
			break;
	}
}
*/
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
		if(s[i] == TAPE){
		  	last_tape_detected = s[i+1];
		}
      }
      break;
    case CMD_MANUAL:
      autonomous = 0;
	  driving_back = 0;
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
	    if(s[i] == REG_SPEED){
          set_speed(s[i+1], 3, 3);
        }
		if(s[i] == REG_TIMER){
		  crossing_timer_max = ((uint32_t) s[i+1]) << 12;
		}
      }
      break;
	  case CMD_AUTO_ON:
      autonomous = 1;
	  driving_back = 0;
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
	way_home[0] = 0;

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
