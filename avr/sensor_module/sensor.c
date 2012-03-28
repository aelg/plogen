#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
//#include <avr/sleep.h>
//#include <stdlib.h>

	volatile uint8_t i = 0;
	volatile uint8_t tape_value = 0; //V�rdet p� den analoga sp�nning som tejpdetektorn gett
	volatile int long_ir_1_value;//V�rdet p� den analoga sp�nning som l�ng avst�ndsm�tare 1 gett
	volatile int long_ir_2_value;//V�rdet p� den analoga sp�nning som l�ng avst�ndsm�tare 2 gett
	volatile int short_ir_1_value;//V�rdet p� den analoga sp�nning som kort avst�ndsm�tare 1 gett
	volatile int short_ir_2_value;//V�rdet p� den analoga sp�nning som kort avst�ndsm�tare 2 gett
	volatile int short_ir_3_value;//V�rdet p� den analoga sp�nning som kort avst�ndsm�tare 3 gett
	volatile int gyro_value; //V�rdet p� den analoga sp�nning som gyrot gett
	volatile int itr_long_ir_1 = 0;
	volatile int itr_long_ir_2 = 0;
	volatile int itr_short_ir_1 = 0;
	volatile int itr_short_ir_2 = 0;
	volatile int itr_short_ir_3 = 0;
	volatile int long_ir_1_values[4];
	volatile int long_ir_2_values[4];
	volatile int short_ir_1_values[4];
	volatile int short_ir_2_values[4];
	volatile int short_ir_3_values[4];

	volatile uint8_t global_tape = 0;
	volatile uint8_t tape_counter = 0;
	volatile uint8_t timer = 0;


//AD-omvandling klar. 
ISR(ADC_vect){

	switch(i){
		case 0: tape_value = ADCH;
		break;
		case 1:
		if (itr_long_ir_1 >= 3){
		long_ir_1_values[itr_long_ir_1]= ADCH;
		itr_long_ir_1 = 0;
		}
		else {
		long_ir_1_values[itr_long_ir_1]= ADCH;
		itr_long_ir_1++;
		}
		break;
		case 2: 		
		if (itr_long_ir_2 >= 3){
		long_ir_2_values[itr_long_ir_2]= ADCH;
		itr_long_ir_2 = 0;
		}
		else {
		long_ir_2_values[itr_long_ir_2]= ADCH;
		itr_long_ir_2++;
		}
		break;
		case 3:
		if (itr_short_ir_1 >= 3){
		short_ir_1_values[itr_short_ir_1]= ADCH;
		itr_short_ir_1 = 0;
		}
		else {
		short_ir_1_values[itr_short_ir_1]= ADCH;
		itr_short_ir_1++;
		}
		break;
		case 4: 
		if (itr_short_ir_2 >= 3){
		short_ir_2_values[itr_short_ir_2]= ADCH;
		itr_short_ir_2 = 0;
		}
		else {
		short_ir_2_values[itr_short_ir_2]= ADCH;
		itr_short_ir_2�++;
		}
		break;
		case 5: 
		if (itr_short_ir_3 >= 3){
		short_ir_3_values[itr_short_ir_3]= ADCH;
		itr_short_ir_3 = 0;
		}
		else {
		short_ir_3_values[itr_short_ir_3]= ADCH;
		itr_short_ir_3++;
		}
		break;
//		case 6: gyro_value = ADCH;
//		break;
	}

	i++;
	if(i == 6){
	i = 0;}

	ADMUX = (ADMUX & 0xE0) | (i & 0x1F); //Byter insignal.
	ADCSRA = 0xCB;//Interrupt-bit nollst�lls
}


//Subrutin f�r att plocka ut det minsta v�rdet av tv� stycken
	unit8_t min(unit8_t value_one, unit8_t value_two)
	if(value_one < value_two)
	{
	return value_one;
	}
	else 
	{ 
	return value_two;
	}

//Subrutin som plockar ut det minsta v�rdet i arrayen
	unit8_t lowest_value(uint8_t *list)
	{
	int minimum = list[0];
	int itr=1;
	for(itr=1,irt<4,itr++)
		{
		if(minimum>list[itr])
		{
		minimum=list[itr];
		}
	}
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
	ADMUX = 0x20;
	ADCSRA = 0xCB; 
	sei(); //till�t interrupt

	while(c) {
		if(tape_value > high_threshold){
			tape_detected(1);}
		else if (tape_value < low_threshold){
			tape_detected(0);}
	}

	return 0;
}
