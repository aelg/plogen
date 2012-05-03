//STYRENHET.C
#undef F_CPU 
#define F_CPU 18400000UL 

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>

#include "../TWI/TWI.h"
#include "motor.h"
#include "../commands.h"
#include "../utility/send.h"

#define LINE_START_MAX 0x0f00
#define CROSSING_TIMER_MAX 0x4000

#define MODE_FROM_SENSOR (PINB & 0x0f)

void set_mode_turn_complete();

// R�knare som ser anv�nds d� saker inte ska g�ras f�r ofta.
uint16_t line_start_timer = 0;
uint32_t crossing_timer = 0;

// V�rden p� r�knare som inneb�r att �tg�rden ska utf�ras.
// Dessa kan �ndras via TWI s� att det g�r att byta tidsf�rdr�jningar fr�n datorn
uint32_t crossing_timer_max = CROSSING_TIMER_MAX;
uint16_t line_start_timer_max = LINE_START_MAX;

// Variabler som definerar vilket l�ge styrenheten �r i just nu.
uint8_t driving_back = 0;
uint8_t autonomous = 0;
uint8_t mode = MODE_STRAIGHT;

// Variabler d�r data mottagen fr�n TWI sparas
uint8_t manual_command = STOP; // Senast mottagna manuella kommando.
uint8_t diff = 127; // Diff mottagen fr�n sensorenheten.
uint8_t rot = 5; // Rotation mottagen fr�n sensorenheten.
uint8_t tape_position = 5; // Den diod d�r det �r mest troligt att tejpen finns.
uint8_t num_diods = 0; // Hur m�nga dioder som uppt�cker tejp.
uint8_t last_tape_detected = 0; //Sparar senaste tejpmarkering

// Variabler som hanterar tillbakav�gen.
uint8_t way_home[20]; //h�r sparas hur vi har k�rt p� v�g in i labyrinten
uint8_t way_home_iterator = 0; //pekar p� hur vi ska sv�nga i n�sta kurva.

// Regulatorparametrar.
uint8_t k_p = K_P;
uint8_t k_d = K_D;

/** Initiera interrupts.
 *  INT0 anv�nds f�r att uppt�cka knapptryck p� den bl� knappen, denna byter mellan autonomt och manuellt l�ge.
 *  INT1 anv�nds f�r att ta emot korsnings och gyrodata fr�n sensorn.*/
void init_interrupts(void){
  // Set interrupt on rising edge of INT0 and INT1 pin
  MCUCR = (MCUCR & 0xf0) | 0x0f;
  // Set INT0 and INT1 as input pins.
  DDRD = (DDRD & 0xf3);
  //Enable interrupts on INT0 and INT1
  GICR |= 0b11000000;
}

/** L�gesrutiner, dessa rutiner anropas f�r att byta l�ge p� sensorenheten p� ett systematiskt s�tt.
 *  F�ljande l�gen finns:
 *  1. MODE_STRAIGHT Detta l�ge �r till f�r att k�ra rakt i rakstr�ckor p� labyrinten. PD-reglering k�rs med
 *  data fr�n sensorenheten.
 *  2. MODE_LINE_FOLLOW Detta l�ge f�ljer linjen framf�r roboten, n�r roboten kommer fram till ett tv�rstreck
 *  byts l�get till "turn around", gripklon st�ngs och roboten p�b�rjar tilbakav�gsk�rningen.
 *  3. MODE_CROSSING Detta l�ge hanterar n�r roboten kommer in i en korsning. Den k�r en timer som r�knar upp s�
 *  att roboten kommer till mitten av korsnningen. N�r detta skett kontrollerar den vilket h�ll den ska sv�nga
 *  och byter till r�tt mode.
 *  4. MODE_FORWARD/LEFT/RIGHT_TURN Dessa l�gen ser till att roboten g�r 90 gradersv�ngar eller forts�tter rakt fram.
 *  Dessa funktioner kommer ocks� att fixa sparning av tillbakv�gsdata.
 *  5. MODE_TURN_AROUND V�nder roboten och b�rjar sedan tillbakav�gsk�rning.
 *  6. MODE_TURN_COMPLETE Ser till att roboten kan ta sig ut ur en korsning efter att r�tt sv�ng blivit utf�rd.
 *  Denna k�r en timer och s�tter sedan MODE_STRAIGHT.
 *  7. MODE_FINISH Detta mode stoppar roboten och g�r �ver till manuellt mode.
 */

/** Byt till regleringsl�ge.
 *  S�tter roboten i l�get d�r den reglerar efter v�ggarna i labyrinten.
 */
void set_mode_straight(){
  send_sensor_mode(MODE_STRAIGHT);
  mode = MODE_STRAIGHT;
}

/** Byt till linjef�ljningsl�ge.
 *  Nollst�ll linjef�ljningsvariabler s� den b�rjar med att �ka rakt, samt skicka MODE_LINE_FOLLOW till sensorenheten.
 */
void set_mode_line_follow(){
	send_sensor_mode(MODE_LINE_FOLLOW);
  num_diods = 1;
	tape_position = 5;
	line_start_timer = 0;
	mode = MODE_LINE_FOLLOW;
}

/** Byt till korsningsl�ge.
 *  I detta l�ge v�ntar roboten p� att crossing_timer ska r�knas upp till crossing_timer_max.
 *  N�r detta skett ska vi vara i mitten p� korsningen. Om sensorenheten anser att den se v�ggar igen 
 *  s� avbryts r�kningen och roboten �terg�r till att att reglera p� v�ggarna.
 *  Roboten k�r rakt fram i detta l�ge.
 */
void set_mode_crossing(){
  mode = MODE_CROSSING;
	crossing_timer = 0;
}

/** Byt till v�nstersv�ngsl�ge.
 *  I detta l�ge roterar roboten 90 grader �t v�nster. Detta sker genom att roboten s�ger �t sensorenheten att
 *  byta till gyrol�ge s� att den skickar ett interrupt d� den roterat klart.
 *  Sedan roterar roboten till dess att interruptet kommer.
 */
void set_mode_left_turn(){
	mode = MODE_CROSSING_LEFT;
	send_sensor_mode(MODE_GYRO);
  if(driving_back)
    --way_home_iterator;
  else
		way_home[++way_home_iterator] = 3;
}

/** Byt till h�gersv�ngsl�ge.
 *  I detta l�ge roterar roboten 90 grader �t h�ger. Detta sker genom att roboten s�ger �t sensorenheten att
 *  byta till gyrol�ge s� att den skickar ett interrupt d� den roterat klart.
 *  Sedan roterar roboten till dess att interruptet kommer.
 */
void set_mode_right_turn(){
	mode = MODE_CROSSING_RIGHT;
	send_sensor_mode(MODE_GYRO);
  if(driving_back)
    --way_home_iterator;
  else
		way_home[++way_home_iterator] = 1;
}

/** Byt till raktframl�ge
 *  Denna funktion byter egentligen till "turn complete"-l�get, 
 *  vi m�ste dock spara undan v�gvalet f�r att hitta tillbaka.
 */
void set_mode_forward_turn(){
  if(driving_back)
    --way_home_iterator;
  else
		way_home[++way_home_iterator] = 2;
  set_mode_turn_complete();
}

/** Byt till l�get "turn around"
 *  Detta �r ett l�ge som g�r en v�nstersv�ng och sedan byter till det vanliga v�stersv�ngsl�get.
 *  H�r m�ste vi se upp med way_back s� det inte h�nder n�gra dumheter.
 *  H�r kr�vs lite av en full�sning. Den h�r sv�ngen och framf�rallt den efterf�ljande vanliga sv�ngen 
 *  sker i med driving_back satt. Detta betyder att den vanliga sv�ngrutinen kommer r�kna ner way_home_iterator, 
 *  d�rf�r r�knas den f�rst upp h�r.
 */
void set_mode_turn_around(){
 	mode = MODE_TURN_AROUND;
	send_sensor_mode(MODE_GYRO);
 	driving_back = 1;
}

/** Byt till "turn complete"
 *  Detta l�ge markerar att sv�ngen �r klar. Detta inneb�r att roboten k�r rakt fram till dess att den �terigen
 *  ser v�ggar och kan b�rja reglera. Dessutom k�rs �ven h�r crossing_timer f�r att f�rhindra att roboten
 *  f�rst av misstag tror att den ser v�ggar byter till reglerl�get och sedan uppt�cker (korrekt)
 *  att den �r i en korsning och b�rjar sv�nga. P� detta s�tt m�ste crossing_timer hinna r�knas upp tv� g�nger
 *  innan roboten kan sv�nga igen (en g�ng i detta l�ge och sedan �n g�ng i korsningsl�get).
 */
void set_mode_turn_complete(){
  crossing_timer = 0; // F�r att hindra roboten att tro att den �r ute ur sv�ngen f�r tidigt.
	mode = MODE_CROSSING_FORWARD;
	send_sensor_mode(MODE_STRAIGHT);
  last_tape_detected = 0;
}

/** Byt till klar l�get.
 *  Detta l�ge stoppar roboten och h�nger sig sedan i en o�ndlig loop roboten �r klar med uppdraget och ska inte g�ra mer.
 */
void set_mode_finish(){
  stop();
  autonomous = 0;
  mode = STOP;
}

// Interrupts

// F�nga felaktiga interrupt om n�got g�r snett. T�nkt att anv�ndas under debuggning med JTAG s� man vet vad som p�g�r
ISR(BADISR_vect){ 	
  volatile uint8_t c;
	while(1) ++c;
}

/* Interruptrutin f�r den bl� knappen, togglar mellan autonomt och icke autonomt l�ge.
 */
ISR(INT0_vect){
	autonomous = 1 - autonomous;
	return;
}

/* Interruptrutin f�r att ta emot tidskritisk information fr�n sensorenheten.
 * Interrupt skickas i f�ljande fall:
 * 1. Sensorenheten har uppt�ckt att den �r i en korsning. D� skickas ocks� data 
 * om vilket h�ll sensorenheten tror att vi ska sv�nga p� PINB.
 * 2. Sensorenheten �r i gyrol�ge och anser att vi sv�ngt klart. D� skickas ocks� MODE_GYRO_COMPLETE p� PINB.
 * 3. Sensorenheten har uppt�ckt 4 tejpar och vi m�ste byta till linjef�ljningsl�ge snabbt. MODE_LINE_FOLLOW skickas p� PINB.
 */
ISR(INT1_vect){
	if (mode == MODE_CROSSING_FORWARD) return;
	uint8_t t_mode = MODE_FROM_SENSOR;
	
	switch(t_mode){
		case MODE_LINE_FOLLOW:
      if(driving_back == 0) set_mode_line_follow();
			break;
		case MODE_GYRO_COMPLETE:
		   set_mode_turn_complete();
			break;
    default:
      set_mode_crossing();
			break;
	}
}

/** H�r f�ljer n�gra rutiner som hanterar vad som h�nder d� vi �r i n�got av moderna.
 *  D�r det beh�vs finns rutinen f�r moden, den anropas i auto_control.
 *  Inget sker i auto_control f�r att f� en konsekvent s�tt att hantera moder.
 */

//Routine to verify a crossing and decide which way to turn.
void crossing(void){
	forward(); // 
                                           // om de inte ser v�ggar och annars reglera efter v�ggen den ser.
	++crossing_timer;
	if(MODE_FROM_SENSOR == MODE_STRAIGHT){
    set_mode_straight();
		return;
	}
	if(crossing_timer > crossing_timer_max){
		if(driving_back){
			switch(way_home[way_home_iterator]){
			case 0:
				set_mode_finish();
				return;
			case 1:
        set_mode_left_turn();
				return;
			case 2:
        set_mode_forward_turn();
				return;
			case 3:
        set_mode_right_turn();
				return;
			}
		}
		switch(last_tape_detected){
		case 1:
      set_mode_forward_turn();
			return;
		case 2:
      set_mode_right_turn();
			return;
		case 3:
      set_mode_left_turn();
			return;
		}
		switch(MODE_FROM_SENSOR){
		case MODE_CROSSING_FORWARD: 
      set_mode_forward_turn();
			return;
		case MODE_CROSSING_RIGHT:
      set_mode_right_turn();
			return;
		case MODE_CROSSING_LEFT:
      set_mode_left_turn();
			return;
		}
	}		
}


/** Hanterar MODE_CROSSING_FORWARD
 *  R�knar upp crossing_counter och byter sen till MODE_STRAIGHT.
 */
void crossing_forward(){
	forward(); // 
                                           // om de inte ser v�ggar och annars reglera efter v�ggen den ser.
	if (crossing_timer < (crossing_timer_max << 1)){
		++crossing_timer;
		return;
	}
	else if(MODE_FROM_SENSOR == MODE_STRAIGHT) set_mode_straight();
}

/** Hanterar MODE_LINE_FOLLOW
 *  R�kna upp line_start_timer s� vi inte tror att den sista tejpen i markeringen �r slutmarkering.
 *  K�r run_line_follow och byt mode om vi �r framme.
 */
void line_follow(){
  uint8_t res;
  if(line_start_timer < line_start_timer_max){
	  ++line_start_timer;
		forward();
		return;
	}
	res = run_line_follow(num_diods, tape_position);
	if(res == END_TAPE){
    set_mode_turn_around();
		driving_back = 1;
	}
	else if(res == NO_TAPE) run_straight(diff, rot, k_p, k_d, TRUE);
}

//Manuell k�rning
void manual_control(){
	run_straight(diff, rot, k_p, k_d, FALSE); // K�r PD-reglering utan att skicka n�got till hjulen (FALSE),
                                            // f�r att kunna se parametrar p� datorn �ven i manuellt l�ge.
	switch(manual_command){
		case LEFT:
			turn_left();
			break;
		case RIGHT:
			turn_right();
			break;
		case FORWARD:
			forward();
			break;
		case REVERSE:
			reverse();
			break;
		case ROTATE_LEFT:
			rotate_left();
			break;
		case ROTATE_RIGHT:
			rotate_right();
			break;
		case STOP:
			stop();
			break;
		
	}
}

//Autonom k�rning
void auto_control(){

	switch(mode){
		case MODE_TURN_AROUND:
		case MODE_CROSSING_LEFT:
			rotate_left();
			break;
		case MODE_CROSSING_RIGHT:
			rotate_right();
			break;
		case MODE_CROSSING_FORWARD:
      crossing_forward();
			break;
		case MODE_STRAIGHT:
			run_straight(diff, rot, k_p, k_d, TRUE);
			break;
		case MODE_CROSSING:
			crossing();
			break;
    case MODE_LINE_FOLLOW:
      line_follow();
			break;
		case MODE_FINISH:
			set_mode_finish();
			break;
	}
}

// Kontrollera meddelanden.
void check_TWI(){
  uint8_t s[16];
  uint8_t len;
  len = TWI_read(s);
  if(len){
    switch(s[0]){
    case CMD_SENSOR_DATA:
      for(uint8_t i = 2; i < len; i = i+2){
        if(s[i] == IRDIFF){
          diff = s[i+1];
        }
		    if(s[i] == LINE_POSITION){
          tape_position = s[i+1];
        }
		    if(s[i] == DIOD){
          num_diods = s[i+1];
		    }
		    if(s[i] == IRROT){
          rot = s[i+1];
        }
		if(s[i] == TAPE){
		  	last_tape_detected = s[i+1];
		}
      }
      break;
    case CMD_MANUAL:
      autonomous = 0;
      manual_command = s[2];
      break;
	case CMD_SET_REG_PARAMS:
	  for(uint8_t i = 2; i < len; i = i+2){
        if(s[i] == REG_P){
          k_p = s[i+1];
        }
	    if(s[i] == REG_D){
          k_d = s[i+1];
        }
	    if(s[i] == REG_SPEED){
          set_speed(s[i+1], 3, 3);
        }
		if(s[i] == REG_TIMER){
		  crossing_timer_max = ((uint32_t) s[i+1]) << 12;
		}
      }
      break;
	  case CMD_AUTO_ON:
      autonomous = 1;
      break;
    }
  }
}

//MAIN
int main(void)
{
	init_interrupts();
	setup_motor();
	TWI_init(CONTROL_ADDRESS);
	sei();
	way_home[0] = 0;

	// Loop
	while (1){

	    // Check TWI bus.
 	    check_TWI();

	    if(autonomous){
			auto_control();
	    }

		else {
			manual_control();
		}
	}
}
