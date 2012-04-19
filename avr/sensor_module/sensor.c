#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include<avr/pgmspace.h>
#include"../styr_module/motor.h"
#include "../TWI/TWI.h"
#include "../utility/send.h"
//#include <avr/sleep.h>
//#include <stdlib.h>

#define GYRO_TURN_LEFT -0x7fffffff
#define GYRO_TURN_RIGHT 0x7fffffff //Tolkas de decimalt??

#define CMD_SENSOR_DATA 0x03;


uint16_t temp_count = 0; // Temporar fullosning
uint16_t temp_ir_count = 0; // Temporar fullosning
volatile uint8_t i = 2;
volatile uint8_t tape_value = 0; //V�rdet p� den analoga sp�nning som tejpdetektorn gett
volatile int32_t gyro_value; //V�rdet p� den analoga sp�nning som gyrot gett
volatile int32_t gyro_sum; //Summan av gyrov�rden. Anv�nds som integral. 
volatile uint8_t gyro_mode = 0;
volatile uint8_t gyro_initialize = 0;
volatile uint8_t global_tape = 0;
volatile uint8_t tape_counter = 0;
volatile uint8_t timer = 0;

volatile uint8_t line_following = 0;
int diod_iterator = 0;
uint8_t diod[11];

volatile int long_ir_1_value;//V�rdet p� den analoga sp�nning som l�ng avst�ndsm�tare 1 gett(pinne 34/PA6)
volatile int long_ir_2_value;//V�rdet p� den analoga sp�nning som l�ng avst�ndsm�tare 2 gett(pinne 38/PA2)
volatile int short_ir_1_value;//V�rdet p� den analoga sp�nning som kort avst�ndsm�tare 1 gett(pinne 37/PA3)
volatile int short_ir_2_value;//V�rdet p� den analoga sp�nning som kort avst�ndsm�tare 2 gett(pinne 36/PA4)
volatile int short_ir_3_value;//V�rdet p� den analoga sp�nning som kort avst�ndsm�tare 3 gett(pinne 35/PA5)
int itr_long_ir_1 = 0;
int itr_long_ir_2 = 0;
int itr_short_ir_1 = 0;
int itr_short_ir_2 = 0;
int itr_short_ir_3 = 0;
uint8_t long_ir_1_values[4];
uint8_t long_ir_2_values[4];
uint8_t short_ir_1_values[4];
uint8_t short_ir_2_values[4];
uint8_t short_ir_3_values[4];


//Referensev�rden

const uint8_t distance_ref_short1[118] = 
	{127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 
  	 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 
  	 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 
  	 119, 112, 108, 104, 100, 96, 93, 90, 86, 83, 
	 80, 77, 74, 71, 69, 66, 64, 62, 61, 59, 
	 57, 55, 53, 51, 49, 48, 47, 45, 44, 42, 
	 41, 40, 38, 36, 34, 33, 32, 31, 30, 29,
	 28, 27, 26, 25, 24, 23, 22, 21, 20, 19,
	 18, 17, 16, 16, 15, 15, 14, 14, 13, 13,
	 12, 12, 11, 11, 10, 10, 9, 9, 8, 8,
	 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 
	 3, 2, 2, 2, 1, 1, 1, 0};

const uint8_t distance_ref_short2[118] =
	{127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 
  	 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 
  	 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 
  	 119, 112, 108, 104, 100, 96, 93, 90, 86, 83, 
	 80, 77, 74, 71, 69, 66, 64, 62, 61, 59, 
	 57, 55, 53, 51, 49, 48, 47, 45, 44, 42, 
	 41, 40, 38, 36, 34, 33, 32, 31, 30, 29,
	 28, 27, 26, 25, 24, 23, 22, 21, 20, 19,
	 18, 17, 16, 16, 15, 15, 14, 14, 13, 13,
	 12, 12, 11, 11, 10, 10, 9, 9, 8, 8,
	 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 
	 3, 2, 2, 2, 1, 1, 1, 0};


const uint8_t distance_ref_short3[118] = 
	{127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 
  	 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 
  	 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 
  	 119, 112, 108, 104, 100, 96, 93, 90, 86, 83, 
	 80, 77, 74, 71, 69, 66, 64, 62, 61, 59, 
	 57, 55, 53, 51, 49, 48, 47, 45, 44, 42, 
	 41, 40, 38, 36, 34, 33, 32, 31, 30, 29,
	 28, 27, 26, 25, 24, 23, 22, 21, 20, 19,
	 18, 17, 16, 16, 15, 15, 14, 14, 13, 13,
	 12, 12, 11, 11, 10, 10, 9, 9, 8, 8,
	 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 
	 3, 2, 2, 2, 1, 1, 1, 0};

										   /*{0, 0, 0,  1,  1,  1,  2,  2,  2,  3,  3, 3,  4, 
											4,	5,	5,  6,  6,  7,  7,  8,  8,  9,  9,  10, 10,    
									 		11,11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17,  
									 		18,19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 
									 		31,32, 33, 34, 36, 38, 40, 41, 42, 44, 45, 47, 48,
											49,51, 53, 55, 57,	59,	61,	62,	64, 66,	69,	71,	74,
									 		77,80,	83,	86,	90,	93,	96,	100,104,108,112,119,127};*/
//volatile uint8_t voltage_ref_long1[]; // Beh�ver endast en bransch!
//volatile uint8_t voltage_ref_long2[]; // Beh�ver endast en bransch!


ISR(BADISR_vect){ // F�nga felaktiga interrupt om n�got g�r snett.
	volatile uint8_t c;
	while(1) ++c;
}

//Subrutin som plockar ut det minsta v�rdet i arrayen
uint8_t lowest_value(uint8_t *list)
{
	int minimum = list[0];
	int itr;
	for(itr=1;itr<4;itr++){
		if(minimum>list[itr]){
		minimum=list[itr];
		}
	}
	return minimum;
}

//Differensfunktion
uint8_t difference(){

	uint8_t diff;
	uint8_t short1;
	uint8_t short2;

	uint8_t low_short1 = lowest_value(short_ir_1_values);
	uint8_t low_short2 = lowest_value(short_ir_2_values);

	if(low_short1 > 117)
		low_short1 = 117;
	if(low_short2 > 117)
		low_short2 = 117;
	short1 = distance_ref_short1[low_short1];
	short2 = distance_ref_short1[low_short2];


	diff = 127 + short1 - short2;

	return diff;
}


//Rotationsfunktion
uint8_t rotation(){

	uint8_t rot;
	uint8_t short3;
	uint8_t short2;

	uint8_t low_short3 = lowest_value(short_ir_3_values);
	uint8_t low_short2 = lowest_value(short_ir_2_values);

	if(low_short3 > 117)
		low_short3 = 117;
	if(low_short2 > 117)
		low_short2 = 117;
	short3 = distance_ref_short3[low_short3];
	short2 = distance_ref_short2[low_short2];


	rot = 127 + short3 - short2;

	return rot;
}





//AD-omvandling klar. 
ISR(ADC_vect){

	if(gyro_mode){
		gyro_sum += (int8_t)ADCH;		

		if((gyro_sum >= GYRO_TURN_RIGHT) || (gyro_sum <= GYRO_TURN_LEFT)){ //V�rde f�r fullbordad sv�ng
			PORTB = (0b11110000 | (PORTB & 0b11001111)); //Ettst�ll PB7-PB4
			gyro_mode = 0; //g� ur gyro_mode
			//gyro_sum = 0;
			ADMUX = (ADMUX & 0xE0) | (i & 0x1F); //Byter insignal. //ska det vara i h�r?
			TIFR = 0b00010000; //Nollst�ll timer-interrupt s� det inte blir interrupt direkt.
			TIMSK = 0b00010000; //Enable interrupt on Output compare A
		}
	
		ADCSRA = 0xCB;
		return;
	} 
	else if(line_following){
		//To do // r�kna upp mux
		diod[diod_iterator] = ADCH;// l�gg ADCH i arrayen
		if(diod_iterator < 10) ++diod_iterator; // r�kna upp iteratorn
		else diod_iterator = 0;
		ADCSRA = 0xCB;//Interrupt-bit nollst�lls
	}
	else{	
		switch(i){
		case 2:
			// Spara v�rde fr�n ad-omvandligen.
			long_ir_2_values[itr_long_ir_2]= ADCH;
			// R�kna upp iteratorn.
      		if(++itr_long_ir_2 > 3) itr_long_ir_2 = 0;
			break;
		case 3:
			// Spara v�rde fr�n ad-omvandligen.
			short_ir_1_values[itr_short_ir_1]= ADCH;
			// R�kna upp iteratorn.
			if(++itr_short_ir_1 > 3) itr_short_ir_1 = 0;
			break;
		case 4:
			// Spara v�rde fr�n ad-omvandligen.
			short_ir_2_values[itr_short_ir_2]= ADCH;
			// R�kna upp iteratorn.
			if(++itr_short_ir_2 > 3) itr_short_ir_2 = 0;
			break;
		case 5:
			// Spara v�rde fr�n ad-omvandligen.
			short_ir_3_values[itr_short_ir_3]= ADCH;
			// R�kna upp iteratorn.
			if(++itr_short_ir_3 > 3) itr_short_ir_3 = 0;
			break;
		case 6:
			// Spara v�rde fr�n ad-omvandligen.
			long_ir_2_values[itr_long_ir_2]= ADCH;
			// R�kna upp iteratorn.
			if(++itr_long_ir_2 > 3) itr_long_ir_2 = 0;
			break;
		case 7: 
			tape_value = ADCH;
			break;
		}

		if(++i > 7){
			i = 2;
		}


		ADMUX = (ADMUX & 0xE0) | (i & 0x1F); //Byter insignal.
		ADCSRA = 0xCB;//Interrupt-bit nollst�lls
		return;
	}
}

//Timern har r�knat klart, interrupt skickas, nu kommer ingen mer tejp.
ISR (TIMER1_COMPA_vect){

	volatile uint8_t tape = tape_counter/2; //Ger antalet tejpar

	switch(tape){
	case 0: 
		PORTB = (PORTB & 0b00001111); //Nollst�ll PB7-PB4
		break;
	case 1:
		PORTB = (0b10010000 | (PORTB & 0b11001111)); //Ettst�ll PB7, PB4
		break;
	case 2: 
		PORTB = (0b10100000 | (PORTB & 0b11001111)); //Ettst�ll PB7, PB5 
		break;
	case 3: 
		PORTB = (0b10110000 | (PORTB & 0b11001111)); //Ettst�ll PB7, PB5 och PB4
		break;
	}

	if (tape) send_tape(tape);
	tape_counter = 0; //Nollst�ll tape_counter d� timern g�tt.
}


//Interrupt som tar hand om overflow i timer1. Borde inte n�nsin hamna h�r......
ISR(TIMER1_OVF_vect){

	TCNT1 = 0; //Nollst�ll r�knaren.
}



//Subrutin f�r att plocka ut det minsta v�rdet av tv� stycken
uint8_t min(uint8_t value_one, uint8_t value_two){
	if(value_one < value_two)
		return value_one;
	else 
		return value_two;
}



//Hittar positionen f�r dioden med h�gsta v�rdet 
uint8_t find_max(){
	uint8_t max_value = 0, max_pos = 0;
	for(uint8_t i = 0; i < 10; ++i){
		if (max_value < diod[i]){
			max_pos = i;
			max_value = diod[i];
		}
	}
	return max_pos;
}


//Subrutin f�r tejpdetektering
void tape_detected(int tape){

	if(tape ^ global_tape){
	global_tape = tape;
	tape_counter++;
	TCNT1 = 0x0000;} //Nollst�ll timer


	//Till M�lomr�desk�rning
	if(tape_counter == 7){
		PORTB |= 0b11000000; // Ettst�ll PB7, PB6
	}
}
	


void init_gyro(){

	ADMUX = 0b00110000;
	TIMSK = 0b00000000; //Disable interrupt on Output compare A
	gyro_initialize = 0;
	gyro_sum = 0;
}

void init_sensor_buffers(){
	itr_long_ir_1 = 0;
	itr_long_ir_2 = 0;
	itr_short_ir_1 = 0;
	itr_short_ir_2 = 0;
	itr_short_ir_3 = 0;
}

//Huvudprogram
int main()
{
	TWI_init(SENSOR_ADDRESS);
	init_sensor_buffers();
	//Initiering
	MCUCR = 0x03;
//	GICR = 0x40;
	DDRA = 0x00;
	DDRB = 0xFF; //utg�ngar, styr mux och signaler till styr

	//Initiera timer
	TCCR1A = 0b00000000; //Eventuellt 00001000 
	TCCR1B = 0b00001101; // gammalt: 4D;
	TIMSK = 0b00010000; //Enable interrupt on Output compare A
	TCNT1 = 0; //Nollst�ll timer
	OCR1A = 0x0200; //s�tt in v�rde som ska trigga avbrott (Utr�knat v�rde = 0x0194)



	uint8_t high_threshold = 160;//Tr�skelv�rde som vid j�mf�relse ger tejp/inte tejp
	uint8_t low_threshold = 20;
	uint8_t diod = 0b00001000;//Anger vilken diod som vi skriver/l�ser till i diodbryggan	
	PORTB = diod; //t�nd diod

	volatile char c;
	c=1;

	//Starta AD-omvandling av insignalen p� PA0 
	ADMUX = 0x27;
	ADCSRA = 0xCB; 
	sei(); //till�t interrupt

	while(c) {

		switch(gyro_mode){
			case 1: if(gyro_initialize == 1) 
						init_gyro();
			break;	
			case 0: if(tape_value > high_threshold){
						tape_detected(1);
					}
					else if (tape_value < low_threshold){
						tape_detected(0);
					}
			break;	
		}

		if (line_following){
		    if(++temp_count > 0x2000){
				uint8_t pos = find_max();
				send_line_pos(pos);
			}
		}
		else if (++temp_count > 0x2000){
			send_differences(difference(), rotation());
			send_tape_value(tape_value);
			send_sensor_values(lowest_value(long_ir_1_values),
							  lowest_value(long_ir_2_values),
							  lowest_value(short_ir_1_values),
							  lowest_value(short_ir_2_values),
							  lowest_value(short_ir_3_values));
			temp_count = 0;
		}
		//if(temp_count == 0x2000)
		//	send_difference(difference());
	}

	return 0;
}
