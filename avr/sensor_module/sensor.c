#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
//#include <avr/sleep.h>
//#include <stdlib.h>

	uint8_t i = 0;
	uint8_t tape_value = 0; //Värdet på den analoga spänning som tejpdetektorn gett
//	int long_ir_1_value;//Värdet på den analoga spänning som lång avståndsmätare 1 gett
//	int long_ir_2_value;//Värdet på den analoga spänning som lång avståndsmätare 2 gett
//	int short_ir_1_value;//Värdet på den analoga spänning som kort avståndsmätare 1 gett
//	int short_ir_2_value;//Värdet på den analoga spänning som kort avståndsmätare 2 gett
//	int short_ir_3_value;//Värdet på den analoga spänning som kort avståndsmätare 3 gett
//	int gyro_value; //Värdet på den analoga spänning som gyrot gett
	uint8_t global_tape = 0;
	uint8_t tape_counter = 0;
	uint8_t timer = 0;


//AD-omvandling klar. 
ISR(ADC_vect){

	switch(i){
		case 0: tape_value = ADCH;
		break;
//		case 1: long_ir_1_value = ADCH;
//		break;
//		case 2: long_ir_2_value = ADCH;
//		break;
//		case 3: short_ir_1_value = ADCH;
//		break;
//		case 4: short_ir_2_value = ADCH;
//		break;
//		case 5: short_ir_3_value = ADCH;
//		break;
//		case 6: gyro_value = ADCH;
//		break;
	}

//	i++;
//	if(i == 6){
//	i = 0;}

//	ADMUX = (ADMUX & 0xE0) | (i & 0x1F);
	ADCSRA = 0xCB;
}

//Timern har räknat klart, interrupt skickas, nu kommer ingen mer tejp.
ISR (TIMER1_COMPA_vect){

	uint8_t tape = tape_counter/2; //Ger antalet tejpar

	switch(tape){
		case 1: PORTB = (PORTB & 0b11110011); //Nollställ PB4 och PB5
		break;
		case 2: PORTB = (0b00001000 | (PORTB & 0b11110011)); //Ettställ PB4 och nollställ PB5
		break;
		case 3: PORTB = (0b00001100 | (PORTB & 0b11110011)); //Ettställ PB4 och PB5
		break;
	}
}





//Subrutin för tejpdetektering
void tape_detected(int tape){

	if(tape ^ global_tape){
	global_tape = tape;
	tape_counter++;
	TCNT1 = 0x0000;} //Nollställ timer


	if(tape_counter == 7){
		//Målområdeskörning
	}
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
	OCR1A = 0x4444; //sätt in värde som ska trigga avbrott gammal värde = 210



	uint8_t threshold = 0xCC;//Tröskelvärde som vid jämförelse ger tejp/inte tejp
	uint8_t diod = 0b00001000;//Anger vilken diod som vi skriver/läser till i diodbryggan	
	PORTB = diod; //tänd diod

	volatile char c;
	c=1;

	//Starta AD-omvandling av insignalen på PA0 
	ADMUX = 0x20;
	ADCSRA = 0xCB; 
	sei(); //tillåt interrupt
	
	while(c) {
		if(tape_value > threshold){
			tape_detected(1);}
		else tape_detected(0);
	}
	return 0;
}

//Subrutin för att se om tejp eller inte
//int tape_detection(){

//	int tape;//Tejp eller inte. tape=1 => tejp. tape=0 => inte tejp.
	


//	if (analog_value > threshold){
//	tape = 1;} //tejp detekterad

//	else {
//	tape = 0;} //ingen tejp detekterad

//	return tape; //returnera 1 om tejp. 0 om inte tejp.
//}
