/** @file
 * Programfil för sensorenheten.
 * Hanterar alla sensorer och behandling av dem. En stor del av jobbet sker med hjälp av AD-omvandlarinterruptet.
 * Detta körs då en AD-omvandling är klar. Interruptet sparar undan den uppmätta spänningen på rätt ställe och 
 * startar sedan mätningen av nästa värde. AD-omvandlingen sker så fort som möjligt, data skickas dock bara med
 * lagom intervall för att hindra att bussen blir överbelastad. Här görs en avvägning desto mer data som skickas
 * desto bättre blir regleringen, å andra sidan blir risken större att det blir fel på bussen.
 *
 * Denna kod gör också behandling av alla uppmätta spänningar. Inga rådata skickas till styrenheten. Däremot skickas
 * en del rådata till kommunikationsenheten för att vidarebefodras till dator.
 *
 * Sensorn körs i tre moder:
 *
 * -# STRAIGHT: Detta läge mäter spänningar på IR-sensorerna och på en av reflexsensorerna. Sen görs linjärisering 
 *    innan data om robotens läge mellan väggarna och hur den är roterad mot högerväggen. Om en korsning upptäcks 
 *    skickas interrupt till styrenheten. Dessutom skickas hela tiden data om hur labyrinten ser ut runt roboten
 *    via kablar från PORTD, alltså inte via I2c. Reflexsensorn räknar också tejpar 1, 2 eller 3 tejpar upptäckta skickas
 *    via I2C. Upptäcks 4 tejpar skickas interrupt till styrenheten.
 * -# GYRO: Här görs AD-omvandling bara på gyrot. När tillräckligt stora värden uppmäts åt något håll skickas 
 *    interrupt till styrenheten.
 * -# LINE_FOLLOW: I detta läge körs IR-sensorerna och alla dioder reflexsensorn. Då skickas linjäriserade lägesdata,
 *    antalet dioder med tejp under och vilken diod som tydligast visar tejp.
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include "../TWI/TWI.h"
#include "../utility/send.h"
#include "../commands.h"

/** @name Tröskelkonstanter 
 * Konstanter för att hantera olika fast gränser för sensorer.
 */
///@{
#define GYRO_TURN_LEFT 800000
#define GYRO_TURN_RIGHT -600000 
#define GYRO_TURN_AROUND 2100000
#define TURN_TRESHOLD 20
#define SHORT_TRESHOLD 30
#define MIDDLE_SENSOR_VALUE 48
///@}

/** @name Maxvärden för räknare. 
 * Konstanter för att hantera maxvärden för olika timers.
 */
///@{
#define SEND_DATA 0x0100 
#define SEND_COMPUTER_DATA 0x2000
///@}

/// Definerar längden på listan där sensorvärden sparas.
#define SENSOR_LIST_LENGTH 8

/// Sparar sensorns mode.
uint8_t mode = MODE_STRAIGHT;
/// Används för att ställa om gyrointerruptet när roboten vänder i målområdet.
uint8_t turn_around = 0;
/// Används för att markera att interruptet för korsning har skickats i den här korsningen och inte ska skickas igen.
uint8_t interrupt_sent = 0;

/** @name Tröskelvariabler 
 * Tröskelvariabler för reflexsensorn.
 */
///@{
uint8_t high_threshold_line_follow = 160; ///< Tröskelvärde som vid jämförelse ger tejp/inte tejp vid linjeföljning
uint8_t high_threshold = 130; ///< Tröskelvärde som vid jämförelse ger tejp/inte tejp
uint8_t low_threshold = 60; ///< Tröskelvärde som vid jämförelse ger tejp/inte
///@}

/** @name Timervariabler.
 *  Räknare för att inte skicka data på bussen för ofta.
 */
///@{
volatile uint16_t send_data_count = 0; ///< Räknare för att skicka data till styrenheten.
volatile uint16_t send_to_computer = 0; ///< Räknare för att skicka data till datorn.
///@}

/** @name AD-omvandlarvariabler.
 * Variabler för att styra AD-omvandlaren och spara värden från den.
 */
///@{
volatile uint8_t i = 2; ///< Anger vilken pinne den interna AD-omvandlarmuxen är inställd på.
volatile uint8_t tape_value = 0; ///< Värdet på spänningen som tejpdetektorn gett.
volatile int32_t gyro_value; ///< Värdet på spänningen som gyrot gett
volatile int32_t gyro_sum; ///< Summan av gyrovärden. Används som integral.
///@}

/** @name Tejpvariabler
 * Variabler för att räknar tejpar.
 */
///@{
volatile uint8_t global_tape = 0; ///< Variabel som sparar om vi ser en tejp, för att kunna räkna förändringar.
volatile uint8_t tape_counter = 0; ///< Räknas upp varje gång vi antigen börjar eller slutar se tejp.
///@}

/** @name Reflexsensorvariabler.
 * Variabler för reflexsensorn.
 */
///@{
int diod_iterator = 0; ///< Diod som mäts just nu.
uint8_t diod[11]; ///< Sparar värden från reflexsensormätningar.
///@}

/** @name IR-sensorvariabler.
 * Variabler för att spara värden från IR-sensorerna.
 * Dessa spara i arrayer för att kunna göra mjukvarufiltrering. Den tredje lägsta mätningen av de sparade används.
 */
///@{
uint8_t long_ir_1_value = 48;//Värdet på den analoga spänning som lång avståndsmätare 1 gett(pinne 34/PA6)
uint8_t long_ir_2_value = 48;//Värdet på den analoga spänning som lång avståndsmätare 2 gett(pinne 38/PA2)
uint8_t short_ir_1_value = 48;
uint8_t short_ir_2_value = 48;
uint8_t short_ir_3_value = 48;
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
///@}

/// Hashtabell för linjarisering av IR-sensorerna.
const uint8_t distance_ref_short[118] = 
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

/** @name IR-sensorbehandling.
 * Funktioner för att behandla data från IR-sensorerna.
 */
///@{

///Subrutin som plockar ut det tredje lägsta värdet från en array.
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

/// Subrutin som plockar ut det högsta värdet i en array.
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

/** Differensberäkning.
 * Tar det filtrerade värdet från de två främre IR-sensorerna, linjäriserar det och räknar ut skillnaden mellan dem.
 * Dessutom adderas 127 till värdet så att 127 betyder mitt mellan väggarna.
 * Om sensorerna visar orimliga värden tas ett standardvärde som motsvarar mitt mellan väggarna. 
 * Bra om endast en av sensorerna upptäckar en vägg.
 */
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

	short1 = distance_ref_short[low_short1];
	short2 = distance_ref_short[low_short2];

	diff = 127 + short1 - short2;

	

	return diff;
}


/** Rotationsberäkning.
 * Tar det filtrerade värdet från de två högra IR-sensorerna, linjäriserar det och räknar ut skillnaden mellan dem.
 * Dessutom adderas 127 till värdet så att 127 betyder mitt mellan väggarna.
 * Om sensorerna visar orimliga värden tas ett standardvärde som motsvarar att roboten står korrekt.
 * Bra om endast en av sensorerna upptäckar en vägg. Det sker också en korrigering för att sensorerna ger olika värden.
 */
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
///@}

/** Skickar interrupt till styrenheten.
 * Skickar interrupt via kablarna kopplade till styrenheten. Väntar också så att styrenheten hinner ta emot interruptet.
 */
void send_interrupt(uint8_t mode){
	PORTD = 0;
	for(uint8_t i = 0; i < 50; ++i){}
	PORTD = 0b00010000 | mode;
	for(uint8_t i = 0; i < 50; ++i){}
	PORTD = mode;
}


/** @name Interruptrutiner
 * Interruptrutiner som hanterar avbrott i styrenheten. Hanterar knappar och sensorsignaler.
 */
///@{
/** Fångar felaktiga interrupt om något går snett. Tänkt att användas under debuggning med JTAG så man vet vad som pågår
 */
ISR(BADISR_vect){ // Fånga felaktiga interrupt om något går snett.
	volatile uint8_t c;
	while(1) ++c;
}

/** AD-omvandlingen är klar.
 * Läser in data och startar ny AD-omvandling. Vad som beräknas beror på vilket mode styrenheten är i.
 * Observera att MODE_LINE_FOLLOW överlappar med MODE_STRAIGHT. Inte helt tydligt hur det fungerar, 
 * men fungerar gör det.
 */
ISR(ADC_vect){

	switch(mode){
	case MODE_GYRO:
		gyro_sum += (int8_t)ADCH;
		if(turn_around == 1 && (gyro_sum >= GYRO_TURN_AROUND)){
			send_interrupt(MODE_GYRO_COMPLETE);
			gyro_sum = 0;
			turn_around = 0;
			}
		else if(turn_around == 0 && ((gyro_sum <= GYRO_TURN_RIGHT) || (gyro_sum >= GYRO_TURN_LEFT))){ //Värde för fullbordad sväng
			send_interrupt(MODE_GYRO_COMPLETE);
			gyro_sum = 0;
		}
	
		ADCSRA = 0xCB;
		break;
	case MODE_LINE_FOLLOW:
		turn_around = 1;
		if(i == 7){
			if(diod_iterator == 1) diod[9] = ADCH;
			else diod[diod_iterator-1] = ADCH;// lägg ADCH i arrayen
			ADCSRA = 0xCB;//Interrupt-bit nollställs
			if(diod_iterator < 9) ++diod_iterator; // räkna upp iteratorn
			else diod_iterator = 1;
			PORTB = (PORTB & 0xf0) | (9-diod_iterator);
			if(diod_iterator == 1){
				i = 2;
			}
			break;
		}
	case MODE_STRAIGHT:
		switch(i){
		case 2:
			// Spara värde från ad-omvandligen.
			long_ir_2_values[itr_long_ir_2]= ADCH;
			// Räkna upp iteratorn.
      		if(++itr_long_ir_2 >= SENSOR_LIST_LENGTH) itr_long_ir_2 = 0;
			break;
		case 5:
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
		case 3:
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
		if(mode == MODE_STRAIGHT || i != 7){
			if(++i > 7){
				i = 2;
			}
		}

		ADMUX = (ADMUX & 0xE0) | (i & 0x1F); //Byter insignal.
		ADCSRA = 0xCB;//Interrupt-bit nollställs
		break;
	}
}

/** Timern har räknat klart.
 * Interrupt skickas, nu kommer ingen mer tejp.
 * Denna timer används för att avgöra när en tejpmarkering är slut.
 * Den nollställs då roboten detekterar att en tejp börjar eller slutar.
 * Om den räknat klart har tillräcklig gått sedan den senaste tejpförändringen,
 * tejpmarkeringen måste vara slut.
 * Antalet tejpar som upptäckts är antalet förändringar delat med två.
 */
ISR (TIMER1_COMPA_vect){

	volatile uint8_t tape = tape_counter >> 1; //Ger antalet tejpar

	if (tape) send_tape(tape);
	tape_counter = 0; //Nollställ tape_counter då timern gått.
}

/** Interrupt som tar hand om overflow i timer1. Borde inte nånsin hamna här.
 */
ISR(TIMER1_OVF_vect){

	TCNT1 = 0; //Nollställ räknaren.
}
///@}



/** Plockar ut det minsta värdet av två stycken.
 */
uint8_t min(uint8_t value_one, uint8_t value_two){
	if(value_one < value_two)
		return value_one;
	else 
		return value_two;
}

/** Hittar positionen för dioden som tydligast ser tejp.
 * Diod 0 och 10 fungerar inte och används inte.
 */
uint8_t find_max(){
	uint8_t max_value = 0, max_pos = 0;
	for(uint8_t i = 1; i < 10; ++i){
		if (max_value < diod[i]){
			max_pos = i;
			max_value = diod[i];
		}
	}
	return max_pos;
}

/** Räknar ut antalet dioder som ser en tejp vid linjeföljningen.
 */
uint8_t tape_detections(){
	uint8_t number_of_diods = 0;

	for(uint8_t i = 1; i < 10;++i){ 

		if(diod[i] > high_threshold_line_follow){
			++number_of_diods;
		}
	}
	return number_of_diods;
}

/** Kontrollerar och hanterar om någon tejpkant upptäckts.
 */
void tape_detected(int tape){

	if(tape ^ global_tape){
	global_tape = tape;
	tape_counter++;
	TCNT1 = 0x0000;} //Nollställ timer


	//Till Målområdeskörning
	if(tape_counter == 7){
		send_interrupt(MODE_LINE_FOLLOW);
		tape_counter = 0;
	}
}

/** Initierar register och muxar för gyrot.
 */
void init_gyro(){

	ADMUX = 0b00110000;
	TIMSK = 0b00000000; //Disable interrupt on Output compare A
	gyro_sum = 0;
}

/** Initierar register och muxar för att läsa IR-sensorer.
 */
void init_straight(void){

	ADMUX = (ADMUX & 0xE0) | (i & 0x1F); //Byter insignal. //ska det vara i här?
	TIFR = 0b00010000; //Nollställ timer-interrupt så det inte blir interrupt direkt.
	TIMSK = 0b00010000; //Enable interrupt on Output compare A
}

/** Nollställer iteratorerna för IR-sensorarrayerna.
 */
void init_sensor_buffers(){
	itr_long_ir_1 = 0;
	itr_long_ir_2 = 0;
	itr_short_ir_1 = 0;
	itr_short_ir_2 = 0;
	itr_short_ir_3 = 0;
}

/** Initierar register och muxar för att köra linjeföljningen.
 */
void init_line_follow(){
	mode = MODE_LINE_FOLLOW;
	send_data_count = 0;
	ADMUX = (ADMUX & 0xE0) | (7 & 0x1F); //Ställ in interna muxen att läsa från externa muxen.
}

/** Skicka data för PD-reglering
 * Funktion som skickar all nödvändig data vid PD-reglering.
 * Om send_to_computer är tillräckligt hög så skickas också rådata till datorn.
 */
void send_straight_data(void){

	if (++send_data_count > SEND_DATA){

		send_differences(difference(), rotation());
		send_data_count = 0;
	}

	if(++send_to_computer > SEND_COMPUTER_DATA){	
	
	send_tape_value(tape_value);
	send_sensor_values(long_ir_1_value,
					  long_ir_2_value,
					  lowest_value(short_ir_1_values),
					  lowest_value(short_ir_2_values),
					  lowest_value(short_ir_3_values));
	send_to_computer = 0;
	}
}

/** Kontrollera busmeddelanden.
 *  Hanterar bussen, läser in från busskön och hanterar. Sätter globala variabler som sedan används av andra funktioner.
 */
uint8_t check_TWI(){
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
		return 1;
  	}
  	else return 0;
}

/** Initera uppstart, datariktningar
 */
void init(void){
	MCUCR = 0x03;
	DDRA = 0x00;
	DDRB = 0xFF; //utgångar, styr mux 
	DDRD = 0xFF; //utgångar, styr interruptsignaler till styr

}

/** Initiera timer för tejpdetektorn
 * Timern ger avbrott då tillräckligt lång tid gått sedan senaste tejpen upptäcktes.
 */
void init_timer(void){
	
	TCCR1A = 0b00000000; //Eventuellt 00001000 
	TCCR1B = 0b00001101; // gammalt: 4D;
	TIMSK = 0b00010000; //Enable interrupt on Output compare A
	TCNT1 = 0; //Nollställ timer
	OCR1A = 0x0600; //sätt in värde som ska trigga avbrott (Uträknat värde = 0x0194)
}


/** Mainloop
 *  Kör alla initieringar och sedan går in i huvudloopen.
 *  Kör check_TWI() och sedan beroende på vilket läge sensorenheten är så 
 *  körs rätt behandling och data skickas.
 *
 *  Här körs också lite mer filtrering.
 *  De sensorvärden som används för att avgöra om roboten är i en korsning och 
 *  de som avgör hur labyrinten ser ut, körs genom en succesiv approximerings-algoritm.
 *  Varje varv i loopen räknas de upp respektive ner med 1 om de uppmätta värdena är 
 *  högre respektive lägre än de gamla. Detta är för att undvika enstaka spikar.
 */
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
			
		while(check_TWI());

		switch(mode){
			case MODE_STRAIGHT:
				if(lowest_value(short_ir_1_values) < short_ir_1_value) --short_ir_1_value;
				else if(lowest_value(short_ir_1_values) > short_ir_1_value) ++short_ir_1_value;
				if(lowest_value(short_ir_2_values) < short_ir_2_value) --short_ir_2_value;
				else if(lowest_value(short_ir_2_values) > short_ir_2_value) ++short_ir_2_value;
				if(lowest_value(short_ir_3_values) < short_ir_3_value) --short_ir_3_value;
				else if(lowest_value(short_ir_3_values) > short_ir_3_value) ++short_ir_3_value;

				if((lowest_value(short_ir_1_values) < SHORT_TRESHOLD) || (lowest_value(short_ir_2_values) < SHORT_TRESHOLD)){
					if (!interrupt_sent){
						send_interrupt(MODE_CROSSING);
						interrupt_sent = 1;
					}
					if(lowest_value(long_ir_1_values) < long_ir_1_value) --long_ir_1_value;
					else if(lowest_value(long_ir_1_values) > long_ir_1_value) ++long_ir_1_value;
					if(lowest_value(long_ir_2_values) < long_ir_2_value) --long_ir_2_value;
					else if(lowest_value(long_ir_2_values) > long_ir_2_value) ++long_ir_2_value;
					if(long_ir_1_value < TURN_TRESHOLD){
						PORTD = MODE_CROSSING_LEFT;
					}
					else if(long_ir_2_value < TURN_TRESHOLD){
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
				if(++send_data_count > SEND_DATA){
					send_data_count = 0;
					uint8_t pos = find_max();
					send_line_pos(pos);
					uint8_t num_diods = tape_detections();
					send_number_of_diods(num_diods);
					send_differences(difference(), rotation());
				}		
				break;
			case MODE_GYRO:
				break;
			}
	}
	return 0;
}
