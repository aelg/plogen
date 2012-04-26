#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include "../TWI/TWI.h"
#include "../utility/send.h"
#include "../commands.h"
//#include <avr/sleep.h>
//#include <stdlib.h>

#define GYRO_TURN_LEFT -750000
#define GYRO_TURN_RIGHT 750000 //Tolkas de decimalt??
#define TURN_TRESHOLD 20

#define SEND_DATA 0x0100
#define SEND_COMPUTER_DATA 0x2000

#define SENSOR_LIST_LENGTH 16


uint8_t mode = MODE_STRAIGHT;

uint8_t interrupt_sent = 0;

uint8_t high_threshold = 100;//Tröskelvärde som vid jämförelse ger tejp/inte tejp
uint8_t low_threshold = 60;//Tröskelvärde som vid jämförelse ger tejp/inte

volatile uint16_t temp_count = 0; // Temporar fullosning
volatile uint16_t send_to_computer = 0;
uint16_t temp_ir_count = 0; // Temporar fullosning
volatile uint8_t i = 2;
volatile uint8_t tape_value = 0; //Värdet på den analoga spänning som tejpdetektorn gett
volatile int32_t gyro_value; //Värdet på den analoga spänning som gyrot gett
volatile int32_t gyro_sum; //Summan av gyrovärden. Används som integral. 
volatile uint8_t global_tape = 0;
volatile uint8_t tape_counter = 0;
volatile uint8_t timer = 0;

int diod_iterator = 0;
uint8_t diod[11];

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
uint8_t long_ir_1_values[SENSOR_LIST_LENGTH];
uint8_t long_ir_2_values[SENSOR_LIST_LENGTH];
uint8_t short_ir_1_values[SENSOR_LIST_LENGTH];
uint8_t short_ir_2_values[SENSOR_LIST_LENGTH];
uint8_t short_ir_3_values[SENSOR_LIST_LENGTH];

uint8_t test_pos;

//Referensevärden

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
//volatile uint8_t voltage_ref_long1[]; // Behöver endast en bransch!
//volatile uint8_t voltage_ref_long2[]; // Behöver endast en bransch!


ISR(BADISR_vect){ // Fånga felaktiga interrupt om något går snett.
	volatile uint8_t c;
	while(1) ++c;
}

//Subrutin som plockar ut det minsta värdet i arrayen
uint8_t lowest_value(uint8_t *list)
{
	int minimum = list[0];
	int itr;
	for(itr=1;itr < SENSOR_LIST_LENGTH;itr++){
		if(minimum>list[itr]){
		minimum=list[itr];
		}
	}
	return minimum;
}

//Subrutin som plockar ut det högsta värdet i arrayen
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

	if (rot > 160 || rot < 80) return 127;

	return rot;
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

		if((gyro_sum >= GYRO_TURN_RIGHT) || (gyro_sum <= GYRO_TURN_LEFT)){ //Värde för fullbordad sväng
			send_interrupt(MODE_GYRO_COMPLETE);
			gyro_sum = 0;
		}
	
		ADCSRA = 0xCB;
		break;
	case MODE_LINE_FOLLOW:
		if(diod_iterator == 0) diod[10] = ADCH;
		else diod[diod_iterator-1] = ADCH;// lägg ADCH i arrayen
		ADCSRA = 0xCB;//Interrupt-bit nollställs
		if(diod_iterator < 10) ++diod_iterator; // räkna upp iteratorn
		else diod_iterator = 0;
		PORTB = (PORTB & 0xf0) | (10-diod_iterator);
		break;
	case MODE_STRAIGHT:	
		switch(i){
		case 2:
			// Spara värde från ad-omvandligen.
			long_ir_2_values[itr_long_ir_2]= ADCH;
			// Räkna upp iteratorn.
      		if(++itr_long_ir_2 >= SENSOR_LIST_LENGTH) itr_long_ir_2 = 0;
			break;
		case 3:
			// Spara värde från ad-omvandligen.
			short_ir_1_values[itr_short_ir_1]= ADCH;
			// Räkna upp iteratorn.
			if(++itr_short_ir_1 >= SENSOR_LIST_LENGTH) itr_short_ir_1 = 0;
			break;
		case 4:
			// Spara värde från ad-omvandligen.
			short_ir_2_values[itr_short_ir_2]= ADCH;
			// Räkna upp iteratorn.
			if(++itr_short_ir_2 >= SENSOR_LIST_LENGTH) itr_short_ir_2 = 0;
			break;
		case 5:
			// Spara värde från ad-omvandligen.
			short_ir_3_values[itr_short_ir_3]= ADCH;
			// Räkna upp iteratorn.
			if(++itr_short_ir_3 >= SENSOR_LIST_LENGTH) itr_short_ir_3 = 0;
			break;
		case 6:
			// Spara värde från ad-omvandligen.
			long_ir_1_values[itr_long_ir_1]= ADCH;
			// Räkna upp iteratorn.
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
		ADCSRA = 0xCB;//Interrupt-bit nollställs
		break;
	}
}

//Timern har räknat klart, interrupt skickas, nu kommer ingen mer tejp.
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



//Hittar positionen för dioden med högsta värdet 
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

//Antalet dioder som ser en tejp vid linjeföljningen
uint8_t tape_detections(){
uint8_t number_of_diods = 0;

	for(i=0;i<10;i++){ 

		if(diod[i] > high_threshold){
			number_of_diods++;
		}
	}
	return number_of_diods;
}

//Subrutin för tejpdetektering
void tape_detected(int tape){

	if(tape ^ global_tape){
	global_tape = tape;
	tape_counter++;
	TCNT1 = 0x0000;} //Nollställ timer


	//Till Målområdeskörning
	if(tape_counter == 7){
		send_interrupt(MODE_LINE_FOLLOW);
	}
}
	


void init_gyro(){

	ADMUX = 0b00110000;
	TIMSK = 0b00000000; //Disable interrupt on Output compare A
	gyro_sum = 0;
}

void init_straight(void){

	ADMUX = (ADMUX & 0xE0) | (i & 0x1F); //Byter insignal. //ska det vara i här?
	TIFR = 0b00010000; //Nollställ timer-interrupt så det inte blir interrupt direkt.
	TIMSK = 0b00010000; //Enable interrupt on Output compare A
}

void init_sensor_buffers(){
	itr_long_ir_1 = 0;
	itr_long_ir_2 = 0;
	itr_short_ir_1 = 0;
	itr_short_ir_2 = 0;
	itr_short_ir_3 = 0;
}


//Funktion som skickar all nödvändig data vid PD-reglering
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
		else init_straight();
		break;

    }
  }
}

//Initera uppstart, datariktningar
void init(void){
	MCUCR = 0x03;
	DDRA = 0x00;
	DDRB = 0xFF; //utgångar, styr mux 
	DDRD = 0xFF; //utgångar, styr interruptsignaler till styr

}

//Initiera timer för tejpdetektorn
void init_timer(void){
	
	TCCR1A = 0b00000000; //Eventuellt 00001000 
	TCCR1B = 0b00001101; // gammalt: 4D;
	TIMSK = 0b00010000; //Enable interrupt on Output compare A
	TCNT1 = 0; //Nollställ timer
	OCR1A = 0x0600; //sätt in värde som ska trigga avbrott (Uträknat värde = 0x0194)
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


	uint8_t diod = 0b00001000;//Anger vilken diod som vi skriver/läser till i diodbryggan	
	PORTB = diod; //tänd diod

	//Starta AD-omvandling av insignalen på PA0 
	ADMUX = 0x27;
	ADCSRA = 0xCB; 
	sei(); //tillåt interrupt

	while(1) {
			
		check_TWI();

		switch(mode){
			case MODE_STRAIGHT:
				if((lowest_value(short_ir_1_values) < 30) || (lowest_value(short_ir_2_values) < 30)){
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
			case MODE_FINISH:
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
