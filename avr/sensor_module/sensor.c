#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
//#include <avr/sleep.h>
//#include <stdlib.h>

	volatile uint8_t gyro_mode = 0;
	volatile uint8_t i = 7;
	volatile uint8_t tape_value = 0; //Värdet på den analoga spänning som tejpdetektorn gett
//	volatile int long_ir_1_value;//Värdet på den analoga spänning som lång avståndsmätare 1 gett
//	volatile int long_ir_2_value;//Värdet på den analoga spänning som lång avståndsmätare 2 gett
//	volatile int short_ir_1_value;//Värdet på den analoga spänning som kort avståndsmätare 1 gett
//	volatile int short_ir_2_value;//Värdet på den analoga spänning som kort avståndsmätare 2 gett
//	volatile int short_ir_3_value;//Värdet på den analoga spänning som kort avståndsmätare 3 gett
	volatile int gyro_value; //Värdet på den analoga spänning som gyrot gett
	volatile uint8_t global_tape = 0;
	volatile uint8_t tape_counter = 0;
	volatile uint8_t timer = 0;


//AD-omvandling klar. 
ISR(ADC_vect){

	if(gyro_mode == 0){
		switch(i){
//			case 2: long_ir_right_value = ADCH;
//			break;
//			case 3: short_ir_left_value = ADCH;
//			break;
//			case 4: short_ir_right_value = ADCH;
//			break;
//			case 5: short_ir_back_value = ADCH;
//			break;
//			case 6: long_ir_left_value = ADCH;
//			break;
			case 7: tape_value = ADCH;
			break;
		}

//		i++;
//		if(i == 7){
//		i = 2;}

//		ADMUX = (ADMUX & 0xE0) | (i & 0x1F);
	}

	else {
		gyro_value = ADCH;
	}

	ADCSRA = 0xCB;
}

//Timern har räknat klart, interrupt skickas, nu kommer ingen mer tejp.
ISR (TIMER1_COMPA_vect){

	volatile uint8_t tape = tape_counter/2; //Ger antalet tejpar

	switch(tape){
		case 0: PORTB = (PORTB & 0b11001111); //Nollställ PB4 och PB5
		break;
		case 1: PORTB = (0b10010000 | (PORTB & 0b11001111)); //Ettställ PB7, PB4
		break;
		case 2: PORTB = (0b10100000 | (PORTB & 0b11001111)); //Ettställ PB7, PB5 
		break;
		case 3: PORTB = (0b10110000 | (PORTB & 0b11001111)); //Ettställ PB7, PB5 och PB4
		break;
	}

	tape_counter = 0; //Nollställ tape_counter då timern gått.
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
	
//Subrutin för att integrera gyrovärden
void integrate_gyro(){

}




//Huvudprogram
int main()
{
	//Initiering
	MCUCR = 0x03;
//	GICR = 0x40;
	DDRA = 0x00;
	DDRB = 0xFF;
	DDRD = 0xFF; //PORTD som utgångar för timern. 

	//Initiera timer
	TCCR1A = 0x88; 
	TCCR1B = 0x4D;
	TIMSK = 0x30;
	TCNT1 = 0; //Nollställ timer
	OCR1A = 0xFFFF; //sätt in värde som ska trigga avbrott (Uträknat värde = 0x0194)



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
		if(tape_value > high_threshold){
			tape_detected(1);
		}
		else if (tape_value < low_threshold){
			tape_detected(0);
		}

		if(gyro_mode == 1){
			ADMUX = 0b00110000;
			integrate_gyro();
		}

	}

	return 0;
}
