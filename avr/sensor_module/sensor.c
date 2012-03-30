#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>

#include "../TWI/TWI.h"
#include "../utility/send.h"
//#include <avr/sleep.h>
//#include <stdlib.h>

#define GYRO_TURN_LEFT -0x7fffffff
#define GYRO_TURN_RIGHT 0x7fffffff //Tolkas de decimalt??



uint16_t temp_count = 0; // Temporar fullosning
uint16_t temp_ir_count = 0; // Temporar fullosning
volatile uint8_t i = 7;
volatile uint8_t tape_value = 0; //Värdet på den analoga spänning som tejpdetektorn gett
volatile int32_t gyro_value; //Värdet på den analoga spänning som gyrot gett
volatile int32_t gyro_sum; //Summan av gyrovärden. Används som integral. 
volatile uint8_t gyro_mode = 0;
volatile uint8_t gyro_initialize = 0;
volatile uint8_t global_tape = 0;
volatile uint8_t tape_counter = 0;
volatile uint8_t timer = 0;
volatile uint8_t long_ir_left_values[4];
volatile uint8_t long_ir_right_values[4];
volatile uint8_t short_ir_left_values[4];
volatile uint8_t short_ir_right_values[4];
volatile uint8_t short_ir_back_values[4];
volatile uint8_t itr_long_ir_left = 0;
volatile uint8_t itr_long_ir_right = 0;
volatile uint8_t itr_short_ir_left = 0;
volatile uint8_t itr_short_ir_right = 0;
volatile uint8_t itr_short_ir_back = 0;

uint8_t lowest_value(uint8_t *list);

ISR(BADISR_vect){ // Fånga felaktiga interrupt om något går snett.
	volatile uint8_t c;
	while(1) ++c;
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
			long_ir_right_values[itr_long_ir_right]= ADCH;
			// Räkna upp iteratorn.
      		if(++itr_long_ir_right > 3) itr_long_ir_right = 0;
			break;
		case 3:
			// Spara värde från ad-omvandligen.
			short_ir_left_values[itr_short_ir_left]= ADCH;
			// Räkna upp iteratorn.
			if(++itr_short_ir_left > 3) itr_short_ir_left = 0;
			break;
		case 4:
			// Spara värde från ad-omvandligen.
			short_ir_right_values[itr_short_ir_right]= ADCH;
			// Räkna upp iteratorn.
			if(++itr_short_ir_right > 3) itr_short_ir_right = 0;
			break;
		case 5:
			// Spara värde från ad-omvandligen.
			short_ir_back_values[itr_short_ir_back]= ADCH;
			// Räkna upp iteratorn.
			if(++itr_short_ir_back > 3) itr_short_ir_back = 0;
			break;
		case 6:
			// Spara värde från ad-omvandligen.
			long_ir_left_values[itr_long_ir_left]= ADCH;
			// Räkna upp iteratorn.
			if(++itr_long_ir_left > 3) itr_long_ir_left = 0;
			break;
		case 7: 
			tape_value = ADCH;
			break;
		}

		if(++i > 7){
			i = 2;
		}

		//i = 7;
		if (temp_count++ > 0x0f00){
			send_tape_value(tape_value);
			send_sensor_values(lowest_value(long_ir_left_values),
							  lowest_value(long_ir_right_values),
							  lowest_value(short_ir_left_values),
							  lowest_value(short_ir_right_values),
							  lowest_value(short_ir_back_values));
			temp_count = 0;
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

//Subrutin som plockar ut det minsta värdet i arrayen
uint8_t lowest_value(uint8_t list[])
{
	uint8_t minimum = list[0];
	for(uint8_t itr = 1; itr < 4; ++itr){
		if(minimum>list[itr])
			minimum=list[itr];
	}
	return minimum;
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
	itr_long_ir_left = 0;
	itr_long_ir_right = 0;
	itr_short_ir_left = 0;
	itr_short_ir_right = 0;
	itr_short_ir_back = 0;
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
	}

	return 0;
}
