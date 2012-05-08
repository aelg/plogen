/** @file
 * Programfil f�r sensorenheten.
 * Hanterar alla sensorer och behandling av dem. En stor del av jobbet sker med hj�lp av AD-omvandlarinterruptet.
 * Detta k�rs d� en AD-omvandling �r klar. Interruptet sparar undan den uppm�tta sp�nningen p� r�tt st�lle och 
 * startar sedan m�tningen av n�sta v�rde. AD-omvandlingen sker s� fort som m�jligt, data skickas dock bara med
 * lagom intervall f�r att hindra att bussen blir �verbelastad. H�r g�rs en avv�gning desto mer data som skickas
 * desto b�ttre blir regleringen, � andra sidan blir risken st�rre att det blir fel p� bussen.
 *
 * Denna kod g�r ocks� behandling av alla uppm�tta sp�nningar. Inga r�data skickas till styrenheten. D�remot skickas
 * en del r�data till kommunikationsenheten f�r att vidarebefodras till dator.
 *
 * Sensorn k�rs i tre moder:
 *
 * -# STRAIGHT: Detta l�ge m�ter sp�nningar p� IR-sensorerna och p� en av reflexsensorerna. Sen g�rs linj�risering 
 *    innan data om robotens l�ge mellan v�ggarna och hur den �r roterad mot h�gerv�ggen. Om en korsning uppt�cks 
 *    skickas interrupt till styrenheten. Dessutom skickas hela tiden data om hur labyrinten ser ut runt roboten
 *    via kablar fr�n PORTD, allts� inte via I2c. Reflexsensorn r�knar ocks� tejpar 1, 2 eller 3 tejpar uppt�ckta skickas
 *    via I2C. Uppt�cks 4 tejpar skickas interrupt till styrenheten.
 * -# GYRO: H�r g�rs AD-omvandling bara p� gyrot. N�r tillr�ckligt stora v�rden uppm�ts �t n�got h�ll skickas 
 *    interrupt till styrenheten.
 * -# LINE_FOLLOW: I detta l�ge k�rs IR-sensorerna och alla dioder reflexsensorn. D� skickas linj�riserade l�gesdata,
 *    antalet dioder med tejp under och vilken diod som tydligast visar tejp.
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include "../TWI/TWI.h"
#include "../utility/send.h"
#include "../commands.h"

/** @name Tr�skelkonstanter 
 * Konstanter f�r att hantera olika fast gr�nser f�r sensorer.
 */
///@{
#define GYRO_TURN_LEFT 800000
#define GYRO_TURN_RIGHT -600000 
#define GYRO_TURN_AROUND 2100000
#define TURN_TRESHOLD 20
#define SHORT_TRESHOLD 30
#define MIDDLE_SENSOR_VALUE 48
///@}

/** @name Maxv�rden f�r r�knare. 
 * Konstanter f�r att hantera maxv�rden f�r olika timers.
 */
///@{
#define SEND_DATA 0x0100 
#define SEND_COMPUTER_DATA 0x2000
///@}

/// Definerar l�ngden p� listan d�r sensorv�rden sparas.
#define SENSOR_LIST_LENGTH 8

/// Sparar sensorns mode.
uint8_t mode = MODE_STRAIGHT;
/// Anv�nds f�r att st�lla om gyrointerruptet n�r roboten v�nder i m�lomr�det.
uint8_t turn_around = 0;
/// Anv�nds f�r att markera att interruptet f�r korsning har skickats i den h�r korsningen och inte ska skickas igen.
uint8_t interrupt_sent = 0;

/** @name Tr�skelvariabler 
 * Tr�skelvariabler f�r reflexsensorn.
 */
///@{
uint8_t high_threshold_line_follow = 160; ///< Tr�skelv�rde som vid j�mf�relse ger tejp/inte tejp vid linjef�ljning
uint8_t high_threshold = 130; ///< Tr�skelv�rde som vid j�mf�relse ger tejp/inte tejp
uint8_t low_threshold = 60; ///< Tr�skelv�rde som vid j�mf�relse ger tejp/inte
///@}

/** @name Timervariabler.
 *  R�knare f�r att inte skicka data p� bussen f�r ofta.
 */
///@{
volatile uint16_t send_data_count = 0; ///< R�knare f�r att skicka data till styrenheten.
volatile uint16_t send_to_computer = 0; ///< R�knare f�r att skicka data till datorn.
///@}

/** @name AD-omvandlarvariabler.
 * Variabler f�r att styra AD-omvandlaren och spara v�rden fr�n den.
 */
///@{
volatile uint8_t i = 2; ///< Anger vilken pinne den interna AD-omvandlarmuxen �r inst�lld p�.
volatile uint8_t tape_value = 0; ///< V�rdet p� sp�nningen som tejpdetektorn gett.
volatile int32_t gyro_value; ///< V�rdet p� sp�nningen som gyrot gett
volatile int32_t gyro_sum; ///< Summan av gyrov�rden. Anv�nds som integral.
///@}

/** @name Tejpvariabler
 * Variabler f�r att r�knar tejpar.
 */
///@{
volatile uint8_t global_tape = 0; ///< Variabel som sparar om vi ser en tejp, f�r att kunna r�kna f�r�ndringar.
volatile uint8_t tape_counter = 0; ///< R�knas upp varje g�ng vi antigen b�rjar eller slutar se tejp.
///@}

/** @name Reflexsensorvariabler.
 * Variabler f�r reflexsensorn.
 */
///@{
int diod_iterator = 0; ///< Diod som m�ts just nu.
uint8_t diod[11]; ///< Sparar v�rden fr�n reflexsensorm�tningar.
///@}

/** @name IR-sensorvariabler.
 * Variabler f�r att spara v�rden fr�n IR-sensorerna.
 * Dessa spara i arrayer f�r att kunna g�ra mjukvarufiltrering. Den tredje l�gsta m�tningen av de sparade anv�nds.
 */
///@{
uint8_t long_ir_1_value = 48;//V�rdet p� den analoga sp�nning som l�ng avst�ndsm�tare 1 gett(pinne 34/PA6)
uint8_t long_ir_2_value = 48;//V�rdet p� den analoga sp�nning som l�ng avst�ndsm�tare 2 gett(pinne 38/PA2)
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

/// Hashtabell f�r linjarisering av IR-sensorerna.
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
 * Funktioner f�r att behandla data fr�n IR-sensorerna.
 */
///@{

///Subrutin som plockar ut det tredje l�gsta v�rdet fr�n en array.
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

/// Subrutin som plockar ut det h�gsta v�rdet i en array.
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

/** Differensber�kning.
 * Tar det filtrerade v�rdet fr�n de tv� fr�mre IR-sensorerna, linj�riserar det och r�knar ut skillnaden mellan dem.
 * Dessutom adderas 127 till v�rdet s� att 127 betyder mitt mellan v�ggarna.
 * Om sensorerna visar orimliga v�rden tas ett standardv�rde som motsvarar mitt mellan v�ggarna. 
 * Bra om endast en av sensorerna uppt�ckar en v�gg.
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


/** Rotationsber�kning.
 * Tar det filtrerade v�rdet fr�n de tv� h�gra IR-sensorerna, linj�riserar det och r�knar ut skillnaden mellan dem.
 * Dessutom adderas 127 till v�rdet s� att 127 betyder mitt mellan v�ggarna.
 * Om sensorerna visar orimliga v�rden tas ett standardv�rde som motsvarar att roboten st�r korrekt.
 * Bra om endast en av sensorerna uppt�ckar en v�gg. Det sker ocks� en korrigering f�r att sensorerna ger olika v�rden.
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
 * Skickar interrupt via kablarna kopplade till styrenheten. V�ntar ocks� s� att styrenheten hinner ta emot interruptet.
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
/** F�ngar felaktiga interrupt om n�got g�r snett. T�nkt att anv�ndas under debuggning med JTAG s� man vet vad som p�g�r
 */
ISR(BADISR_vect){ // F�nga felaktiga interrupt om n�got g�r snett.
	volatile uint8_t c;
	while(1) ++c;
}

/** AD-omvandlingen �r klar.
 * L�ser in data och startar ny AD-omvandling. Vad som ber�knas beror p� vilket mode styrenheten �r i.
 * Observera att MODE_LINE_FOLLOW �verlappar med MODE_STRAIGHT. Inte helt tydligt hur det fungerar, 
 * men fungerar g�r det.
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
		else if(turn_around == 0 && ((gyro_sum <= GYRO_TURN_RIGHT) || (gyro_sum >= GYRO_TURN_LEFT))){ //V�rde f�r fullbordad sv�ng
			send_interrupt(MODE_GYRO_COMPLETE);
			gyro_sum = 0;
		}
	
		ADCSRA = 0xCB;
		break;
	case MODE_LINE_FOLLOW:
		turn_around = 1;
		if(i == 7){
			if(diod_iterator == 1) diod[9] = ADCH;
			else diod[diod_iterator-1] = ADCH;// l�gg ADCH i arrayen
			ADCSRA = 0xCB;//Interrupt-bit nollst�lls
			if(diod_iterator < 9) ++diod_iterator; // r�kna upp iteratorn
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
		if(mode == MODE_STRAIGHT || i != 7){
			if(++i > 7){
				i = 2;
			}
		}

		ADMUX = (ADMUX & 0xE0) | (i & 0x1F); //Byter insignal.
		ADCSRA = 0xCB;//Interrupt-bit nollst�lls
		break;
	}
}

/** Timern har r�knat klart.
 * Interrupt skickas, nu kommer ingen mer tejp.
 * Denna timer anv�nds f�r att avg�ra n�r en tejpmarkering �r slut.
 * Den nollst�lls d� roboten detekterar att en tejp b�rjar eller slutar.
 * Om den r�knat klart har tillr�cklig g�tt sedan den senaste tejpf�r�ndringen,
 * tejpmarkeringen m�ste vara slut.
 * Antalet tejpar som uppt�ckts �r antalet f�r�ndringar delat med tv�.
 */
ISR (TIMER1_COMPA_vect){

	volatile uint8_t tape = tape_counter >> 1; //Ger antalet tejpar

	if (tape) send_tape(tape);
	tape_counter = 0; //Nollst�ll tape_counter d� timern g�tt.
}

/** Interrupt som tar hand om overflow i timer1. Borde inte n�nsin hamna h�r.
 */
ISR(TIMER1_OVF_vect){

	TCNT1 = 0; //Nollst�ll r�knaren.
}
///@}



/** Plockar ut det minsta v�rdet av tv� stycken.
 */
uint8_t min(uint8_t value_one, uint8_t value_two){
	if(value_one < value_two)
		return value_one;
	else 
		return value_two;
}

/** Hittar positionen f�r dioden som tydligast ser tejp.
 * Diod 0 och 10 fungerar inte och anv�nds inte.
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

/** R�knar ut antalet dioder som ser en tejp vid linjef�ljningen.
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

/** Kontrollerar och hanterar om n�gon tejpkant uppt�ckts.
 */
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

/** Initierar register och muxar f�r gyrot.
 */
void init_gyro(){

	ADMUX = 0b00110000;
	TIMSK = 0b00000000; //Disable interrupt on Output compare A
	gyro_sum = 0;
}

/** Initierar register och muxar f�r att l�sa IR-sensorer.
 */
void init_straight(void){

	ADMUX = (ADMUX & 0xE0) | (i & 0x1F); //Byter insignal. //ska det vara i h�r?
	TIFR = 0b00010000; //Nollst�ll timer-interrupt s� det inte blir interrupt direkt.
	TIMSK = 0b00010000; //Enable interrupt on Output compare A
}

/** Nollst�ller iteratorerna f�r IR-sensorarrayerna.
 */
void init_sensor_buffers(){
	itr_long_ir_1 = 0;
	itr_long_ir_2 = 0;
	itr_short_ir_1 = 0;
	itr_short_ir_2 = 0;
	itr_short_ir_3 = 0;
}

/** Initierar register och muxar f�r att k�ra linjef�ljningen.
 */
void init_line_follow(){
	mode = MODE_LINE_FOLLOW;
	send_data_count = 0;
	ADMUX = (ADMUX & 0xE0) | (7 & 0x1F); //St�ll in interna muxen att l�sa fr�n externa muxen.
}

/** Skicka data f�r PD-reglering
 * Funktion som skickar all n�dv�ndig data vid PD-reglering.
 * Om send_to_computer �r tillr�ckligt h�g s� skickas ocks� r�data till datorn.
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
 *  Hanterar bussen, l�ser in fr�n bussk�n och hanterar. S�tter globala variabler som sedan anv�nds av andra funktioner.
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
	DDRB = 0xFF; //utg�ngar, styr mux 
	DDRD = 0xFF; //utg�ngar, styr interruptsignaler till styr

}

/** Initiera timer f�r tejpdetektorn
 * Timern ger avbrott d� tillr�ckligt l�ng tid g�tt sedan senaste tejpen uppt�cktes.
 */
void init_timer(void){
	
	TCCR1A = 0b00000000; //Eventuellt 00001000 
	TCCR1B = 0b00001101; // gammalt: 4D;
	TIMSK = 0b00010000; //Enable interrupt on Output compare A
	TCNT1 = 0; //Nollst�ll timer
	OCR1A = 0x0600; //s�tt in v�rde som ska trigga avbrott (Utr�knat v�rde = 0x0194)
}


/** Mainloop
 *  K�r alla initieringar och sedan g�r in i huvudloopen.
 *  K�r check_TWI() och sedan beroende p� vilket l�ge sensorenheten �r s� 
 *  k�rs r�tt behandling och data skickas.
 *
 *  H�r k�rs ocks� lite mer filtrering.
 *  De sensorv�rden som anv�nds f�r att avg�ra om roboten �r i en korsning och 
 *  de som avg�r hur labyrinten ser ut, k�rs genom en succesiv approximerings-algoritm.
 *  Varje varv i loopen r�knas de upp respektive ner med 1 om de uppm�tta v�rdena �r 
 *  h�gre respektive l�gre �n de gamla. Detta �r f�r att undvika enstaka spikar.
 */
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
