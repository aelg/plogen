//STYRENHET.C

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>

#include "motor.h"
#include "../commands.h"
#include "../utility/send.h"

int16_t max_speed = 0x0080;
int16_t turn_speed = 0x0003;
int16_t stop_speed = 0x0003;

uint16_t delay = 0;


//Functions

//Griparmsfunktion
void griparm(void)
{
	TCNT2 = 0x00;
	DDRD=0xFF;

	TCCR2 |=(1<<FOC2);
	TCCR2 |=(0 << WGM21)|(1<<WGM20);
	TCCR2 |= (1<< COM21)|(1<< COM20);
	TCCR2 |= (1<< CS22) |(1<< CS21)|(0<< CS20);

	OCR2 = 0xF0;//sets the length of pulses
}

//K�rs alltid vid uppstart
void setup_motor(void)
{
	TCNT1 = 0x0000;
	ICR1 = 0x00FF;//Topvalue of counter
	DDRA=0xFF;
	DDRD=0b11111011;

	TCCR1A =(1<<COM1A1)|(0<<COM1A0)|(1<<COM1B1)|(0<<COM1B0)|(0<<FOC1A)|(0<<FOC1B)|(0<<WGM11)|(0<<WGM10); //PMW uses ICR1 as TOP-value.;//phase and frequency correct PMW
	TCCR1B =(0<<ICNC1)|(0<<ICES1)|(1<<WGM13)|(0<<WGM12)|(0<<CS12)|(1<<CS11)|(1<<CS10);

	OCR1A = stop_speed;//sets the length of pulses, right side - pin7
	OCR1B = stop_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

//Rotera h�ger
void rotate_right(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0) //Left wheel direction - pin5
		  |(0<<PORTA1);//Right wheel direction - pin6
}
	
//Rotera v�nster
void rotate_left(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(0<<PORTA0)//Left wheel direction - pin5
	      |(1<<PORTA1);//Right wheel direction - pin6
}

//K�ra fram�t ur korsning
void drive_forward(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

//Rutin f�r v�nstersv�ng
void turn_left(void)
{
	OCR1A =	turn_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

//Rutin f�r h�gersv�ng
void turn_right(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	turn_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

//Rutin f�r k�rning rakt fram
void turn_forward(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

void manual_left(void)
{
	OCR1A =	0x0030;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}
void manual_right(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	0x0030;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

void manual_forward(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6	
}

void manual_stop(void)
{
	OCR1A =	stop_speed;//sets the length of pulses, right side - pin7
	OCR1B =	stop_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6	
}

void manual_reverse(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(0<<PORTA0)//Left wheel direction - pin5
		  |(0<<PORTA1);//Right wheel direction - pin6	
}

void run_straight(uint8_t diff, uint8_t rot, uint8_t k_p, uint8_t k_d){

	int16_t difference = diff;
	int16_t rotation = rot;

	int16_t p = k_p*(difference - 127) >> REGULATOR_CORR;
	int16_t d = k_d*(rotation - 127) >> REGULATOR_CORR;

	/*if(++delay > 0xf000){
		send_reg_params(p, d);
		delay = 0;
	}*/
	
	//if (!(delay & 0x00ff)){ // Don't run to often.
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
