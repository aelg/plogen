#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include "../TWI/TWI.h"
#include "../utility/send.h"
#include "../commands.h"
//#include <avr/sleep.h>
//#include <stdlib.h>

#define GYRO_TURN_LEFT 950000
#define GYRO_TURN_RIGHT -700000 //Tolkas de decimalt??
#define GYRO_TURN_AROUND 2700000
#define TURN_TRESHOLD 20
#define SHORT_TRESHOLD 30
#define MIDDLE_SENSOR_VALUE 48

#define SEND_DATA 0x0040
#define SEND_COMPUTER_DATA 0x2000

#define SENSOR_LIST_LENGTH 8


uint8_t mode = MODE_STRAIGHT;

uint8_t turn_around = 0;

uint8_t interrupt_sent = 0;

uint8_t high_threshold = 170;//Tr�skelv�rde som vid j�mf�relse ger tejp/inte tejp
uint8_t low_threshold = 140;//Tr�skelv�rde som vid j�mf�relse ger tejp/inte

volatile uint16_t temp_count = 0; // Temporar fullosning
volatile uint16_t send_to_computer = 0;
uint16_t temp_ir_count = 0; // Temporar fullosning
volatile uint8_t i = 2;
volatile uint8_t tape_value = 0; //V�rdet p� den analoga sp�nning som tejpdetektorn gett
volatile int32_t gyro_value; //V�rdet p� den analoga sp�nning som gyrot gett
volatile int32_t gyro_sum; //Summan av gyrov�rden. Anv�nds som integral. 
volatile uint8_t global_tape = 0;
volatile uint8_t tape_counter = 0;
volatile uint8_t timer = 0;

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
uint8_t long_ir_1_values[SENSOR_LIST_LENGTH];
uint8_t long_ir_2_values[SENSOR_LIST_LENGTH];
uint8_t short_ir_1_values[SENSOR_LIST_LENGTH];
uint8_t short_ir_2_values[SENSOR_LIST_LENGTH];
uint8_t short_ir_3_values[SENSOR_LIST_LENGTH];

uint8_t test_pos;


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


ISR(BADISR_vect){ // F�nga felaktiga interrupt om n�got g�r snett.
	volatile uint8_t c;
	while(1) ++c;
}

//Subrutin som plockar ut det minsta v�rdet i arrayen
uint8_t lowest_value(uint8_t *list)
{
  uint8_t min1, min2, min3, itr;
	min1 = list[0];
  min2 = min3 = 255;
	for(itr=1;itr < SENSOR_LIST_LENGTH;itr++){
		if(list[itr] < min1){
		  min3 = min2;
      min2 = min1;
      min1 = list[itr];
		}
    else if(list[itr] < min2){
		  min3 = min2;
      min2 = list[itr];
		}
    else if(list[itr] < min3){
		  min3 = list[itr];
		}
	}
	return min3;
}

//Subrutin som plockar ut det h�gsta v�rdet i arrayen
uint8_t highest_value(uint8_t *list)
{
	int maximum = list[0];
	int itr;
	for(itr=1;itr<SENSOR_LIST_LENGTH;itr++){
		if(maximum<list[itr]){
		maximum=list[itr];
		}
	}
	return maximum;
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
  else if(low_short1 < SHORT_TRESHOLD)
    low_short1 = MIDDLE_SENSOR_VALUE;
	if(low_short2 > 117)
		low_short2 = 117;
  else if(low_short2 < SHORT_TRESHOLD)
    low_short2 = MIDDLE_SENSOR_VALUE;

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

	if (rot > 160 || rot < 80) return 127;

	return rot - 7;
}


//Skickar interrupt till styrenheten
void send_interrupt(uint8_t mode){

	PORTD = 0b00010000 | mode;
	PORTD = mode;
}


//AD-omvandling klar. 
ISR(ADC_vect){

	switch(mode){
	case MODE_GYRO:
		gyro_sum += (int8_t)ADCH;
		if(turn_around == 1 && (gyro_sum >= GYRO_TURN_AROUND)){
			send_interrupt(MODE_GYRO_COMPLETE);
			gyro_sum = 0;
			turn_around = 0;
			}
		else if(turn_around == 0 && ((gyro_sum <= GYRO_TURN_RIGHT) || (gyro_sum >= GYRO_TURN_LEFT))){ //V�rde f�r fullbordad sv�ng
			send_interrupt(MODE_GYRO_COMPLETE);
			gyro_sum = 0;
		}
	
		ADCSRA = 0xCB;
		break;
	case MODE_LINE_FOLLOW:
		turn_around = 1;
		if(diod_iterator == 0) diod[10] = ADCH;
		else diod[diod_iterator-1] = ADCH;// l�gg ADCH i arrayen
		ADCSRA = 0xCB;//Interrupt-bit nollst�lls
		if(diod_iterator < 10) ++diod_iterator; // r�kna upp iteratorn
		else diod_iterator = 0;
		PORTB = (PORTB & 0xf0) | (10-diod_iterator);
		break;
	case MODE_STRAIGHT:
		switch(i){
		case 2:
			// Spara v�rde fr�n ad-omvandligen.
			long_ir_2_values[itr_long_ir_2]= ADCH;
			// R�kna upp iteratorn.
      		if(++itr_long_ir_2 >= SENSOR_LIST_LENGTH) itr_long_ir_2 = 0;
			break;
		case 5:
			// Spara v�rde fr�n ad-omvandligen.
			short_ir_1_values[itr_short_ir_1]= ADCH;
			// R�kna upp iteratorn.
			if(++itr_short_ir_1 >= SENSOR_LIST_LENGTH) itr_short_ir_1 = 0;
			break;
		case 4:
			// Spara v�rde fr�n ad-omvandligen.
			short_ir_2_values[itr_short_ir_2]= ADCH;
			// R�kna upp iteratorn.
			if(++itr_short_ir_2 >= SENSOR_LIST_LENGTH) itr_short_ir_2 = 0;
			break;
		case 3:
			// Spara v�rde fr�n ad-omvandligen.
			short_ir_3_values[itr_short_ir_3]= ADCH;
			// R�kna upp iteratorn.
			if(++itr_short_ir_3 >= SENSOR_LIST_LENGTH) itr_short_ir_3 = 0;
			break;
		case 6:
			// Spara v�rde fr�n ad-omvandligen.
			long_ir_1_values[itr_long_ir_1]= ADCH;
			// R�kna upp iteratorn.
			if(++itr_long_ir_1 >= SENSOR_LIST_LENGTH) itr_long_ir_1 = 0;
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
		break;
	}
}

//Timern har r�knat klart, interrupt skickas, nu kommer ingen mer tejp.
ISR (TIMER1_COMPA_vect){

	volatile uint8_t tape = tape_counter >> 1; //Ger antalet tejpar

	/*switch(tape){
	case 1:
		send_interrupt(MODE_TURN_FORWARD);
		break;
	case 2: 
		send_interrupt(MODE_TURN_RIGHT);
		break;
	case 3: 
		send_interrupt(MODE_TURN_LEFT);
		break;
	}*/

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

//Antalet dioder som ser en tejp vid linjef�ljningen
uint8_t tape_detections(){
uint8_t number_of_diods = 0;

	for(i=0;i<10;i++){ 

		if(diod[i] > high_threshold){
			number_of_diods++;
		}
	}
	return number_of_diods;
}

//Subrutin f�r tejpdetektering
void tape_detected(int tape){

	if(tape ^ global_tape){
	global_tape = tape;
	tape_counter++;
	TCNT1 = 0x0000;} //Nollst�ll timer


	//Till M�lomr�desk�rning
	if(tape_counter == 7){
		send_interrupt(MODE_LINE_FOLLOW);
		tape_counter = 0;
	}
}
	


void init_gyro(){

	ADMUX = 0b00110000;
	TIMSK = 0b00000000; //Disable interrupt on Output compare A
	gyro_sum = 0;
}

void init_straight(void){

	ADMUX = (ADMUX & 0xE0) | (i & 0x1F); //Byter insignal. //ska det vara i h�r?
	TIFR = 0b00010000; //Nollst�ll timer-interrupt s� det inte blir interrupt direkt.
	TIMSK = 0b00010000; //Enable interrupt on Output compare A
}

void init_sensor_buffers(){
	itr_long_ir_1 = 0;
	itr_long_ir_2 = 0;
	itr_short_ir_1 = 0;
	itr_short_ir_2 = 0;
	itr_short_ir_3 = 0;
}


//Funktion som skickar all n�dv�ndig data vid PD-reglering
void send_straight_data(void){

	if (++temp_count > SEND_DATA){

		send_differences(difference(), rotation());
		temp_count = 0;
	}

	if(++send_to_computer > SEND_COMPUTER_DATA){	
	
	send_tape_value(tape_value);
	send_sensor_values(lowest_value(long_ir_1_values),
					  lowest_value(long_ir_2_values),
					  lowest_value(short_ir_1_values),
					  lowest_value(short_ir_2_values),
					  lowest_value(short_ir_3_values));
	send_to_computer = 0;
	}
}

void init_line_follow(){
	mode = MODE_LINE_FOLLOW;
	temp_count = 0;
	ADMUX = (ADMUX & 0xE0) | (7 & 0x1F); //St�ll in interna muxen att l�sa fr�n externa muxen.
}

// Kontrollera meddelanden.
void check_TWI(){
  uint8_t s[16];
  uint8_t len;
  len = TWI_read(s);
  if(len){
    switch(s[0]){
    case CMD_SENSOR_MODE:
		mode = s[2];

		if(mode == MODE_GYRO) init_gyro();
		else if(mode == MODE_LINE_FOLLOW) init_line_follow();
		else init_straight();
		break;

    }
  }
}

//Initera uppstart, datariktningar
void init(void){
	MCUCR = 0x03;
	DDRA = 0x00;
	DDRB = 0xFF; //utg�ngar, styr mux 
	DDRD = 0xFF; //utg�ngar, styr interruptsignaler till styr

}

//Initiera timer f�r tejpdetektorn
void init_timer(void){
	
	TCCR1A = 0b00000000; //Eventuellt 00001000 
	TCCR1B = 0b00001101; // gammalt: 4D;
	TIMSK = 0b00010000; //Enable interrupt on Output compare A
	TCNT1 = 0; //Nollst�ll timer
	OCR1A = 0x0600; //s�tt in v�rde som ska trigga avbrott (Utr�knat v�rde = 0x0194)
}

//Huvudprogram
int main()
{
	TWI_init(SENSOR_ADDRESS);
	init_sensor_buffers();
	//Initiera interrupt, datariktningar 
	init();
	
	//Initiera timer
	init_timer();


	uint8_t diod = 0b00001000;//Anger vilken diod som vi skriver/l�ser till i diodbryggan	
	PORTB = diod; //t�nd diod

	//Starta AD-omvandling av insignalen p� PA0 
	ADMUX = 0x27;
	ADCSRA = 0xCB; 
	sei(); //till�t interrupt
	
	while(1) {
			
		check_TWI();

		switch(mode){
			case MODE_STRAIGHT:
				if((lowest_value(short_ir_1_values) < SHORT_TRESHOLD) || (lowest_value(short_ir_2_values) < SHORT_TRESHOLD)){
					if (!interrupt_sent){
						send_interrupt(MODE_CROSSING);
						interrupt_sent = 1;
					}
				
					if(lowest_value(long_ir_1_values) < TURN_TRESHOLD){
						PORTD = MODE_CROSSING_LEFT;
					}
					else if(lowest_value(long_ir_2_values) < TURN_TRESHOLD){
						PORTD = MODE_CROSSING_RIGHT;
					}
					else PORTD = MODE_CROSSING_FORWARD;
				}
				else{
					interrupt_sent = 0;
					PORTD = PORTD & 0xe0;
				}
				send_straight_data();
				if(tape_value > high_threshold){
					tape_detected(1);
				}
				else if (tape_value < low_threshold){
					tape_detected(0);
				}
				break;					
			case MODE_LINE_FOLLOW:
				if(++temp_count > SEND_DATA){
					temp_count = 0;
					uint8_t pos = find_max();
					send_line_pos(pos);
					uint8_t num_diods = tape_detections();
					send_number_of_diods(num_diods);
				}		
				break;
			case MODE_GYRO:
				break;
			}
	}
	return 0;
}
