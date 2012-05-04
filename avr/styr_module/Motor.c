//STYRENHET.C

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>

#include "motor.h"
#include "../commands.h"
#include "../utility/send.h"

int16_t max_speed = 180;
int16_t turn_speed = 0x0003;
int16_t stop_speed = 0x0003;

uint16_t delay = 0;


//Functions

//Griparmsfunktion
void griparm(uint8_t grip)
{	 
	TCNT2 = 0x00;
	DDRD= DDRD | 0x80;

	TCCR2 |=(1<<FOC2);
	TCCR2 |=(0 << WGM21)|(1<<WGM20);
	TCCR2 |= (1<< COM21)|(1<< COM20);
	TCCR2 |= (1<< CS22) |(1<< CS21)|(0<< CS20);
	if (grip == CLOSE){
		OCR2 = 0xe0;//sets the length of pulses
	} 
	else if(grip == OPEN) {
		OCR2 = 0x50; //sets the length of pulses
	}
}

//Körs alltid vid uppstart
void setup_motor(void)
{
	TCNT1 = 0x0000;
	ICR1 = 0x00FF;//Topvalue of counter
	DDRA = 0xFF;
	DDRD = 0b11110011; //KANSKE DUMT? ÄNDRAS PÅ FLERA STÄLLEN. KANSKE BÄTTRE ATT BARA ÄNDRA DE BERÖRDA BITARNA

	TCCR1A =(1<<COM1A1)|(0<<COM1A0)|(1<<COM1B1)|(0<<COM1B0)|(0<<FOC1A)|(0<<FOC1B)|(0<<WGM11)|(0<<WGM10); //PMW uses ICR1 as TOP-value.;//phase and frequency correct PMW
	TCCR1B =(0<<ICNC1)|(0<<ICES1)|(1<<WGM13)|(0<<WGM12)|(0<<CS12)|(1<<CS11)|(1<<CS10);

	OCR1A = stop_speed;//sets the length of pulses, right side - pin7
	OCR1B = stop_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

//Rotera höger
void rotate_right(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0) //Left wheel direction - pin5
		  |(0<<PORTA1);//Right wheel direction - pin6
}
	
//Rotera vänster
void rotate_left(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(0<<PORTA0)//Left wheel direction - pin5
	      |(1<<PORTA1);//Right wheel direction - pin6
}

//Köra framåt ur korsning
void forward(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		|(1<<PORTA1);//Right wheel direction - pin6
}

//Rutin för hogersväng
void turn_right(void)
{
	OCR1A =	turn_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

//Rutin för vanstersväng
void turn_left(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	turn_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

void stop(void)
{
	OCR1A =	stop_speed;//sets the length of pulses, right side - pin7
	OCR1B =	stop_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

void reverse(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(0<<PORTA0)//Left wheel direction - pin5
		  |(0<<PORTA1);//Right wheel direction - pin6	
}

void run_straight(uint8_t diff, uint8_t rot, uint8_t k_p, uint8_t k_d, uint8_t run){

	int16_t difference = (diff - 127) << REGULATOR_CORR;
	int16_t rotation = (rot - 127) << REGULATOR_CORR;

	int16_t p = (k_p*difference) >> REGULATOR_CORR;
	int16_t d = (k_d*rotation) >> REGULATOR_CORR;

	if(++delay > 0x1000){
		send_reg_params(p, d);
		delay = 0;
	}
	
	if(!run) return;

	int16_t pdreg_value = p + d;

	//if((uint16_t)pdreg_value > max_speed) pdreg_value = max_speed;

	if((int16_t)pdreg_value < 0){
		if(max_speed+pdreg_value < stop_speed)
			OCR1A = stop_speed;
		else 
			OCR1A =	(uint16_t) max_speed+pdreg_value;//sets the length of pulses, right side - pin7

		OCR1B =	max_speed;//sets the length of pulses, left side - pin8

	
	}
	else{
		OCR1A =	max_speed;//sets the length of pulses, right side - pin7

		if(max_speed-pdreg_value < stop_speed)
			OCR1B = stop_speed;
		else
			OCR1B =	(uint16_t) max_speed-pdreg_value;//sets the length of pulses, left side - pin8
	}
	
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
	//}
	
	return;
}


// Målområdeskörning
uint8_t run_line_follow(uint8_t num_diods, uint8_t tape_position){
	if(num_diods > 5){
		stop();
		griparm(CLOSE);
		return 1;
	}
	else if(num_diods == 0){
		return NO_TAPE;
	}
	else if(tape_position<4){
		turn_right();
	}
	else if(tape_position>=4 && tape_position<=6){
		forward();
	}
	else {
		turn_left();
	}
	return 0;

}

void set_speed(int16_t max, int16_t turn, int16_t stop){
  max_speed = max;
  turn_speed = turn;
  stop_speed = stop;
}

