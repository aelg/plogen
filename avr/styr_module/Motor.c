/**
 * \addtogroup styr Styrenhet
 * @{
 */

/** @file 
 * Kod f�r att hantera motorerna i styrenheten, samt regleralgoritmer.
 * Inneh�ller mest enklar funktioner f�r enkla registeromst�llningar som reglerar hastigheten p� de olika hjulen.
 *
 * B�de linjef�ljningsregleringen och PD-regleringen efter v�ggarna finns h�r.
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>

#include "motor.h"
#include "../commands.h"
#include "../utility/send.h"

/// Hastighet om roboten k�r rakt fram.
int16_t max_speed = 250;
/// Hastighet p� innerhjulet om roboten sv�nger.
int16_t turn_speed = 0x0003;
/// Hastighet p� hjulen om roboten st�r stilla.
int16_t stop_speed = 0x0003;

/// Timer som anv�nds f�r att skicka regulatorparametrar till datorn med lagom intervall.
uint16_t delay = 0;



/** Griparmsfunktion
 *  Funktion f�r att kontrollera griparmen.
 */
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

/** Initiering av motorstyrningen.
 *  S�tter register f�r att st�lla PWM-modulerna f�r att kunna kontrollera motorerna.
 */
void setup_motor(void)
{
	TCNT1 = 0x0000;
	ICR1 = 0x00FF;//Topvalue of counter
	DDRA = 0xFF;
	DDRD = 0b11110011; //KANSKE DUMT? �NDRAS P� FLERA ST�LLEN. KANSKE B�TTRE ATT BARA �NDRA DE BER�RDA BITARNA

	TCCR1A =(1<<COM1A1)|(0<<COM1A0)|(1<<COM1B1)|(0<<COM1B0)|(0<<FOC1A)|(0<<FOC1B)|(0<<WGM11)|(0<<WGM10); //PMW uses ICR1 as TOP-value.;//phase and frequency correct PMW
	TCCR1B =(0<<ICNC1)|(0<<ICES1)|(1<<WGM13)|(0<<WGM12)|(0<<CS12)|(1<<CS11)|(1<<CS10);

	OCR1A = stop_speed;//sets the length of pulses, right side - pin7
	OCR1B = stop_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

/// Rotera h�ger
void rotate_right(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0) //Left wheel direction - pin5
		  |(0<<PORTA1);//Right wheel direction - pin6
}
	
/// Rotera v�nster
void rotate_left(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(0<<PORTA0)//Left wheel direction - pin5
	      |(1<<PORTA1);//Right wheel direction - pin6
}

/// K�r fram�t.
void forward(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		|(1<<PORTA1);//Right wheel direction - pin6
}

/// K�r h�ger.
/// Mjukt genom att bromsa innerhjulet.
void turn_right(void)
{
	OCR1A =	turn_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

/// K�r v�nster.
/// Mjukt genom att bromsa innerhjulet.
void turn_left(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	turn_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

/// Stanna.
void stop(void)
{
	OCR1A =	stop_speed;//sets the length of pulses, right side - pin7
	OCR1B =	stop_speed;//sets the length of pulses, left side - pin8
	PORTA =(1<<PORTA0)//Left wheel direction - pin5
		  |(1<<PORTA1);//Right wheel direction - pin6
}

/// K�r bak�t.
void reverse(void)
{
	OCR1A =	max_speed;//sets the length of pulses, right side - pin7
	OCR1B =	max_speed;//sets the length of pulses, left side - pin8
	PORTA =(0<<PORTA0)//Left wheel direction - pin5
		  |(0<<PORTA1);//Right wheel direction - pin6	
}

/** Reglering.
 *  K�r regleringsalgoritmen.
 *  Ber�knar P- och D-del genom att multiplicera diff och rot med k_p och k_d och sedan dra av 
 *  det fr�n max_speed p� r�tt hjul. Parametern run=FALSE anv�nds f�r att k�ra algoritmen och bara skicka
 *  reglerdata till datorn utan att �ndra n�gra hastigheter p� hjulen.
 */
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


/** Linjef�ljning.
 *  Reglerar roboten i m�lomr�det. Kontrollerar var linjen finns och sv�nger �t r�tt h�ll.
 *  Returnerar END_TAPE, NO_TAPE eller TAPE_DETECTED beroende p� om roboten ser en tv�rg�ende tejp, inte ser tejp
 *  eller ser tejp.
 */
uint8_t run_line_follow(uint8_t num_diods, uint8_t tape_position){
	if(num_diods > 5){
		stop();
		griparm(CLOSE);
		return END_TAPE;
	}
	else if(num_diods == 0){
		return NO_TAPE;
	}
	else if(tape_position<=4){
		turn_right();
	}
	else if(tape_position==5){
		forward();
	}
	else {
		turn_left();
	}
	return TAPE_DETECTED;

}

/** St�ll in hastighetsvariabler.
 *  St�ller in variablerna som kontrollerar robotens hastighet. Anv�nds dels f�r att k�ra saktare i m�lomr�det,
 *  samt f�r att st�lla in data fr�n datorn.
 */
void set_speed(int16_t max, int16_t turn, int16_t stop){
  max_speed = max;
  turn_speed = turn;
  stop_speed = stop;
}

/*
 * @}
 */
