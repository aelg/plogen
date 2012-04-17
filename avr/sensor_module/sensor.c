#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include<avr/pgmspace.h>
//#include<../styr_module/motor.h>
#include "../TWI/TWI.h"
#include "../utility/send.h"
//#include <avr/sleep.h>
//#include <stdlib.h>

#define GYRO_TURN_LEFT -0x7fffffff
#define GYRO_TURN_RIGHT 0x7fffffff //Tolkas de decimalt??

#define STRAIGHT 0x04;
#define CMD_SENSOR_DATA 0x03;


uint16_t temp_count = 0; // Temporar fullosning
uint16_t temp_ir_count = 0; // Temporar fullosning
volatile uint8_t i = 2;
volatile uint8_t tape_value = 0; //Värdet på den analoga spänning som tejpdetektorn gett
volatile int32_t gyro_value; //Värdet på den analoga spänning som gyrot gett
volatile int32_t gyro_sum; //Summan av gyrovärden. Används som integral. 
volatile uint8_t gyro_mode = 0;
volatile uint8_t gyro_initialize = 0;
volatile uint8_t global_tape = 0;
volatile uint8_t tape_counter = 0;
volatile uint8_t timer = 0;

volatile int long_ir_1_value;//Värdet på den analoga spänning som lång avståndsmätare 1 gett(pinne 34/PA6)
volatile int long_ir_2_value;//Värdet på den analoga spänning som lång avståndsmätare 2 gett(pinne 38/PA2)
volatile int short_ir_1_value;//Värdet på den analoga spänning som kort avståndsmätare 1 gett(pinne 37/PA3)
volatile int short_ir_2_value;//Värdet på den analoga spänning som kort avståndsmätare 2 gett(pinne 36/PA4)
volatile int short_ir_3_value;//Värdet på den analoga spänning som kort avståndsmätare 3 gett(pinne 35/PA5)
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


//Referensevärden
volatile uint8_t voltage_ref_short1[91] = {120,	119,118,117,116,115,114,113,112,111,110,109,108,
											107,106,105,104,103,102,101,100,99, 98,	97,	96,	95,	
											94,	93,	92,	91,	90,	89,	88,	87,	86,	85,	84,	83,	82,	
											81,	80,	79,	78,	77,	76,	75,	74,	73,	72,	71,	70,	69,	
											68,	67,	66,	65,	64,	63,	62,	61,	60,	59,	58, 57,	56,	
											55,	54,	53,	52,	51,	50,	49,	48,	47,	46,	45,	44,	43,	
											42,	41,	40,	39,	38,	37,	36,	35,	34, 33,	32,	31,	30};

volatile uint8_t distance_ref_short1[91] = {0, 0, 0,  1,  1,  1,  2,  2,  2,  3,  3, 3,  4, 
											4,	5,	5,  6,  6,  7,  7,  8,  8,  9,  9,  10, 10,    
									 		11,11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17,  
									 		18,19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 
									 		31,32, 33, 34, 36, 38, 40, 41, 42, 44, 45, 47, 48,
											49,51, 53, 55, 57,	59,	61,	62,	64, 66,	69,	71,	74,
									 		77,80,	83,	86,	90,	93,	96,	100,104,108,112,119,127};

volatile uint8_t voltage_ref_short2[91] = {120,	119,118,117,116,115,114,113,112,111,110,109,108,	
											107,106,105,104,103,102,101,100,99, 98,	97,	96,	95,	
											94,	93,	92,	91,	90,	89,	88,	87,	86,	85,	84,	83,	82,	
											81,	80,	79,	78,	77,	76,	75,	74,	73,	72,	71,	70,	69,	
											68,	67,	66,	65,	64,	63,	62,	61,	60,	59,	58, 57,	56,	
											55,	54,	53,	52,	51,	50,	49,	48,	47,	46,	45,	44,	43,	
											42,	41,	40,	39,	38,	37,	36,	35,	34, 33,	32,	31,	30};

volatile uint8_t distance_ref_short2[91] = {0, 0, 0,  1,  1,  1,  2,  2,  2,  3,  3, 3,  4,
											4,	5,	5,  6,  6,  7,  7,  8,  8,  9,  9,  10, 10,    
									 		11,11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17,  
									 		18,19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 
									 		31,32, 33, 34, 36, 38, 40, 41, 42, 44, 45, 47, 48,
											49,51, 53, 55, 57,	59,	61,	62,	64, 66,	69,	71,	74,
									 		77,80,	83,	86,	90,	93,	96,	100,104,108,112,119,127};

volatile uint8_t voltage_ref_short3[91] = {120,	119,118,117,116,115,114,113,112,111,110,109,108,	
											107,106,105,104,103,102,101,100,99, 98,	97,	96,	95,	
											94,	93,	92,	91,	90,	89,	88,	87,	86,	85,	84,	83,	82,	
											81,	80,	79,	78,	77,	76,	75,	74,	73,	72,	71,	70,	69,	
											68,	67,	66,	65,	64,	63,	62,	61,	60,	59,	58, 57,	56,	
											55,	54,	53,	52,	51,	50,	49,	48,	47,	46,	45,	44,	43,	
											42,	41,	40,	39,	38,	37,	36,	35,	34, 33,	32,	31,	30};

volatile uint8_t distance_ref_short3[91] = {0, 0, 0,  1,  1,  1,  2,  2,  2,  3,  3, 3,  4, 
											4,	5,	5,  6,  6,  7,  7,  8,  8,  9,  9,  10, 10,    
									 		11,11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17,  
									 		18,19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 
									 		31,32, 33, 34, 36, 38, 40, 41, 42, 44, 45, 47, 48,
											49,51, 53, 55, 57,	59,	61,	62,	64, 66,	69,	71,	74,
									 		77,80,	83,	86,	90,	93,	96,	100,104,108,112,119,127};
//volatile uint8_t voltage_ref_long1[]; // Behöver endast en bransch!
//volatile uint8_t voltage_ref_long2[]; // Behöver endast en bransch!



uint8_t lowest_value(uint8_t *list);

ISR(BADISR_vect){ // Fånga felaktiga interrupt om något går snett.
	volatile uint8_t c;
	while(1) ++c;
}

//Subrutin som plockar ut det minsta värdet i arrayen
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

uint8_t short1 = distance_ref_short1[120 - lowest_value(short_ir_1_values)];
uint8_t short2 = distance_ref_short2[120 - lowest_value(short_ir_2_values)];

diff = 0x0f + short1 - short2;

return diff;
}

//Funktion för att skicka sensorvärden
void send_difference(){
	uint8_t s[3];
	s[0] = CMD_SENSOR_DATA;
	s[1] = 1;
	s[2] = difference();

	TWI_write(COMM_ADDRESS, s, 3);
}

//AD-omvandling klar. 
ISR(ADC_vect){

	if(gyro_mode){
		gyro_sum += (int8_t)ADCH;		

		if((gyro_sum >= GYRO_TURN_RIGHT) || (gyro_sum <= GYRO_TURN_LEFT)){ //Värde för fullbordad sväng
			PORTB = (0b11110000 | (PORTB & 0b11001111)); //Ettställ PB7-PB4
			gyro_mode = 0; //gå ur gyro_mode
			//gyro_sum = 0;
			ADMUX = (ADMUX & 0xE0) | (i & 0x1F); //Byter insignal.
			TIFR = 0b00010000; //Nollställ timer-interrupt så det inte blir interrupt direkt.
			TIMSK = 0b00010000; //Enable interrupt on Output compare A
		}
	
		ADCSRA = 0xCB;
		return;
	}
	else{	
		switch(i){
		case 2:
			// Spara värde från ad-omvandligen.
			long_ir_2_values[itr_long_ir_2]= ADCH;
			// Räkna upp iteratorn.
      		if(++itr_long_ir_2 > 3) itr_long_ir_2 = 0;
			break;
		case 3:
			// Spara värde från ad-omvandligen.
			short_ir_1_values[itr_short_ir_1]= ADCH;
			// Räkna upp iteratorn.
			if(++itr_short_ir_1 > 3) itr_short_ir_1 = 0;
			break;
		case 4:
			// Spara värde från ad-omvandligen.
			short_ir_2_values[itr_short_ir_2]= ADCH;
			// Räkna upp iteratorn.
			if(++itr_short_ir_2 > 3) itr_short_ir_2 = 0;
			send_difference();
			break;
		case 5:
			// Spara värde från ad-omvandligen.
			short_ir_3_values[itr_short_ir_3]= ADCH;
			// Räkna upp iteratorn.
			if(++itr_short_ir_3 > 3) itr_short_ir_3 = 0;
			break;
		case 6:
			// Spara värde från ad-omvandligen.
			long_ir_2_values[itr_long_ir_2]= ADCH;
			// Räkna upp iteratorn.
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
		ADCSRA = 0xCB;//Interrupt-bit nollställs
		return;
	}
}

//Timern har räknat klart, interrupt skickas, nu kommer ingen mer tejp.
ISR (TIMER1_COMPA_vect){

	volatile uint8_t tape = tape_counter/2; //Ger antalet tejpar

	switch(tape){
	case 0: 
		PORTB = (PORTB & 0b00001111); //Nollställ PB7-PB4
		break;
	case 1:
		PORTB = (0b10010000 | (PORTB & 0b11001111)); //Ettställ PB7, PB4
		break;
	case 2: 
		PORTB = (0b10100000 | (PORTB & 0b11001111)); //Ettställ PB7, PB5 
		break;
	case 3: 
		PORTB = (0b10110000 | (PORTB & 0b11001111)); //Ettställ PB7, PB5 och PB4
		break;
	}

	if (tape) send_tape(tape);
	tape_counter = 0; //Nollställ tape_counter då timern gått.
}


//Interrupt som tar hand om overflow i timer1. Borde inte nånsin hamna här......
ISR(TIMER1_OVF_vect){

	TCNT1 = 0; //Nollställ räknaren.
}



//Subrutin för att plocka ut det minsta värdet av två stycken
uint8_t min(uint8_t value_one, uint8_t value_two){
	if(value_one < value_two)
		return value_one;
	else 
		return value_two;
}


//Subrutin för tejpdetektering
void tape_detected(int tape){

	if(tape ^ global_tape){
	global_tape = tape;
	tape_counter++;
	TCNT1 = 0x0000;} //Nollställ timer


	//Målområdeskörning
	//if(tape_counter == 7){
		//PORTB |= 0b11000000; // Ettställ PB7, PB6
	//}
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
	DDRB = 0xFF; //utgångar, styr mux och signaler till styr

	//Initiera timer
	TCCR1A = 0b00000000; //Eventuellt 00001000 
	TCCR1B = 0b00001101; // gammalt: 4D;
	TIMSK = 0b00010000; //Enable interrupt on Output compare A
	TCNT1 = 0; //Nollställ timer
	OCR1A = 0x0200; //sätt in värde som ska trigga avbrott (Uträknat värde = 0x0194)



	uint8_t high_threshold = 160;//Tröskelvärde som vid jämförelse ger tejp/inte tejp
	uint8_t low_threshold = 20;
	uint8_t diod = 0b00001000;//Anger vilken diod som vi skriver/läser till i diodbryggan	
	PORTB = diod; //tänd diod

	volatile char c;
	c=1;

	//Starta AD-omvandling av insignalen på PA0 
	ADMUX = 0x27;
	ADCSRA = 0xCB; 
	sei(); //tillåt interrupt

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
		if (++temp_count > 0x0f00){
			send_tape_value(tape_value);
			send_sensor_values(lowest_value(long_ir_1_values),
							  lowest_value(long_ir_2_values),
							  lowest_value(short_ir_1_values),
							  lowest_value(short_ir_2_values),
							  lowest_value(short_ir_3_values));
			temp_count = 0;
		}
	}

	return 0;
}
