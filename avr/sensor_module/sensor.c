#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
//#include <avr/sleep.h>
//#include <stdlib.h>

	int i = 0;
	int tape_value; //Värdet på den analoga spänning som tejpdetektorn gett
//	int long_ir_1_value;//Värdet på den analoga spänning som lång avståndsmätare 1 gett
//	int long_ir_2_value;//Värdet på den analoga spänning som lång avståndsmätare 2 gett
//	int short_ir_1_value;//Värdet på den analoga spänning som kort avståndsmätare 1 gett
//	int short_ir_2_value;//Värdet på den analoga spänning som kort avståndsmätare 2 gett
//	int short_ir_3_value;//Värdet på den analoga spänning som kort avståndsmätare 3 gett
//	int gyro_value; //Värdet på den analoga spänning som gyrot gett

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

uint8_t global_tape = 0;
uint8_t tape_counter = 0;
uint8_t timer = 0;

//Subrutin för tejpdetektering
int tape_detected(int tape){
	if(tape ^ global_tape){
	global_tape = tape;
	tape_counter++;
	timer = 0;}

	if(tape_counter == 4){
		//Målområdeskörning
	}

	if(timer > 

	return 0;
}
		




//Huvudprogram
int main()
{
	//Initiering
	MCUCR = 0x03;
	GICR = 0x40;
	DDRB= 0xFF;
	DDRD= 0xFF; //PORTD som utgångar för timern. 

	//Initiera timer
	TCCR1A = 0x88; 
	TCCR1B = 0x4D;
	TIMSK = 0x30;
	OCR1AH = //sätt in högt värde i timern
	OCR1AL = //sätt in lågt värde i timern


	uint8_t threshold = 0xCC;//Tröskelvärde som vid jämförelse ger tejp/inte tejp
	uint8_t diod = 0x06;//Anger vilken diod som vi skriver/läser till i diodbryggan	
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
