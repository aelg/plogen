#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
//#include <avr/sleep.h>
//#include <stdlib.h>

	volatile uint8_t gyro_mode = 0;
	volatile uint8_t i = 7;
	volatile uint8_t tape_value = 0; //V�rdet p� den analoga sp�nning som tejpdetektorn gett
//	volatile int long_ir_1_value;//V�rdet p� den analoga sp�nning som l�ng avst�ndsm�tare 1 gett
//	volatile int long_ir_2_value;//V�rdet p� den analoga sp�nning som l�ng avst�ndsm�tare 2 gett
//	volatile int short_ir_1_value;//V�rdet p� den analoga sp�nning som kort avst�ndsm�tare 1 gett
//	volatile int short_ir_2_value;//V�rdet p� den analoga sp�nning som kort avst�ndsm�tare 2 gett
//	volatile int short_ir_3_value;//V�rdet p� den analoga sp�nning som kort avst�ndsm�tare 3 gett
	volatile int gyro_value; //V�rdet p� den analoga sp�nning som gyrot gett
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

//Timern har r�knat klart, interrupt skickas, nu kommer ingen mer tejp.
ISR (TIMER1_COMPA_vect){

	volatile uint8_t tape = tape_counter/2; //Ger antalet tejpar

	switch(tape){
		case 0: PORTB = (PORTB & 0b11001111); //Nollst�ll PB4 och PB5
		break;
		case 1: PORTB = (0b10010000 | (PORTB & 0b11001111)); //Ettst�ll PB7, PB4
		break;
		case 2: PORTB = (0b10100000 | (PORTB & 0b11001111)); //Ettst�ll PB7, PB5 
		break;
		case 3: PORTB = (0b10110000 | (PORTB & 0b11001111)); //Ettst�ll PB7, PB5 och PB4
		break;
	}

	tape_counter = 0; //Nollst�ll tape_counter d� timern g�tt.
}





//Subrutin f�r tejpdetektering
void tape_detected(int tape){

	if(tape ^ global_tape){
	global_tape = tape;
	tape_counter++;
	TCNT1 = 0x0000;} //Nollst�ll timer

	//M�lomr�desk�rning
	//if(tape_counter == 7){
		//PORTB |= 0b11000000; // Ettst�ll PB7, PB6
	//}
}
	
//Subrutin f�r att integrera gyrov�rden
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
	DDRD = 0xFF; //PORTD som utg�ngar f�r timern. 

	//Initiera timer
	TCCR1A = 0x88; 
	TCCR1B = 0x4D;
	TIMSK = 0x30;
	TCNT1 = 0; //Nollst�ll timer
	OCR1A = 0xFFFF; //s�tt in v�rde som ska trigga avbrott (Utr�knat v�rde = 0x0194)



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
