//STYRENHET.C

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/sleep.h>


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

//
void setup_motor(void)
{
	TCNT1 = 0x0000;
	ICR1 = 0x00FF;//Topvalue of counter
	DDRA=0xFF;
	DDRD=0xFF;

//TCCR1A=[
	TCCR1A =(1<<COM1A1)|(0<<COM1A0)|(1<<COM1B1)|(0<<COM1B0)|(0<<FOC1A)|(0<<FOC1B)|(0<<WGM11)|(0<<WGM10); //PMW uses ICR1 as TOP-value.;//phase and frequency correct PMW
			
//TCCR1B=[00-
	TCCR1B =(0<<ICNC1)|(0<<ICES1)|(1<<WGM13)|(0<<WGM12)|(0<<CS12)|(1<<CS11)|(1<<CS10);
			//no noise cancelation

//	TCCR1B = 0xDF & TCCR1B;//Bit5=0

	OCR1B =	0x00F0;//sets the length of pulses, right side - pin8
	OCR1A =	0x00F0;//sets the length of pulses, left side - pin7
	PORTA |=(1<<PORTA0);//Left wheel direction - pin5
	PORTA |=(1<<PORTA1);//Right wheel direction - pin6

}

void rotate_right(void)
{
	PORTA |=(0<<DDA0);//Right wheel direction
	PORTA |=(1<<DDA1);//Left wheel direction
	}	

void rotate_left(void)
{
	PORTA |=(1<<DDA0);//Right wheel direction
	PORTA |=(0<<DDA1);//Left wheel direction
	}

int main(void)
{

	griparm();

	setup_motor();

	while(1){
		asm volatile("nop");
		}	
	
	}		



