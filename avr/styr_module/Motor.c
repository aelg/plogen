//STYRENHET.C

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>

#include "motor.h"
#include "../commands.h"

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

//Körs alltid vid uppstart
void setup_motor(void)
{
	TCNT1 = 0x0000;
	ICR1 = 0x00FF;//Topvalue of counter
	DDRA=0xFF;
	DDRD=0xFF;

	TCCR1A =(1<<COM1A1)|(0<<COM1A0)|(1<<COM1B1)|(0<<COM1B0)|(0<<FOC1A)|(0<<FOC1B)|(0<<WGM11)|(0<<WGM10); //PMW uses ICR1 as TOP-value.;//phase and frequency correct PMW
	TCCR1B =(0<<ICNC1)|(0<<ICES1)|(1<<WGM13)|(0<<WGM12)|(0<<CS12)|(1<<CS11)|(1<<CS10);

	OCR1A = 0x0003;//sets the length of pulses, left side - pin7
	OCR1B = 0x0003;//sets the length of pulses, right side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

//Rotera höger
void rotate_right(void)
{
	OCR1A =	0x00F0;//sets the length of pulses, left side - pin7
	OCR1B =	0x00F0;//sets the length of pulses, right side - pin8
	PORTA =(1<<PORTA0) //Left wheel direction - pin5
		  |(0<<PORTA1);//Right wheel direction - pin6
}
	
//Rotera vänster
void rotate_left(void)
{
	OCR1A =	0x00F0;//sets the length of pulses, left side - pin7
	OCR1B =	0x00F0;//sets the length of pulses, right side - pin8
	PORTA =(0<<PORTA0)//Left wheel direction - pin5
	      |(1<<PORTA1);//Right wheel direction - pin6
}

//Köra framåt ur korsning
void drive_forward(void)
{
	OCR1A =	0x00F0;//sets the length of pulses, left side - pin7
	OCR1B =	0x00F0;//sets the length of pulses, right side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

//Rutin för vänstersväng
void turn_left(void)
{
	OCR1A =	0x0040;//sets the length of pulses, left side - pin7
	OCR1B =	0x00F0;//sets the length of pulses, right side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

//Rutin för högersväng
void turn_right(void)
{
	OCR1A =	0x00F0;//sets the length of pulses, left side - pin7
	OCR1B =	0x0040;//sets the length of pulses, right side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

//Rutin för körning rakt fram
void turn_forward(void)
{
	OCR1A =	0x00F0;//sets the length of pulses, left side - pin7
	OCR1B =	0x00F0;//sets the length of pulses, right side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

void manual_left(void)
{
	OCR1A =	0x0030;//sets the length of pulses, left side - pin7
	OCR1B =	0x00F0;//sets the length of pulses, right side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}
void manual_right(void)
{
	OCR1A =	0x00F0;//sets the length of pulses, left side - pin7
	OCR1B =	0x0030;//sets the length of pulses, right side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

void manual_forward(void)
{
	OCR1A =	0x00F0;//sets the length of pulses, left side - pin7
	OCR1B =	0x00F0;//sets the length of pulses, right side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6	
}

void manual_stop(void)
{
	OCR1A =	0x0003;//sets the length of pulses, left side - pin7
	OCR1B =	0x0003;//sets the length of pulses, right side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6	
}

void manual_reverse(void)
{
	OCR1A =	0x00f0;//sets the length of pulses, left side - pin7
	OCR1B =	0x00f0;//sets the length of pulses, right side - pin8
	PORTA =(0<<PORTA0)//Left wheel direction - pin5
		  |(0<<PORTA1);//Right wheel direction - pin6	
}
