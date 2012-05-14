/** @file
 * Huvudprogram för styrenheten.
 * Här finns all logik för styrenheten. */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>

#include "../TWI/TWI.h"
#include "motor.h"
#include "../commands.h"
#include "../utility/send.h"


#define LINE_START_MAX 0x0f00 ///< Värde då line_start_timer slår över.
#define CROSSING_TIMER_MAX 0x4000 ///< Värde då crossing_timer slår över.

/// Används för att modedata från sensorn.
#define MODE_FROM_SENSOR (PINB & 0x0f)

void set_mode_turn_complete();

/** @name Räknare 
 *  Används då saker inte ska göras för ofta.
 */
///@{

/// Räknare så roboten inte detekterar sista tejpen i markeringen som slutmarkering.
uint16_t line_start_timer = 0;
/// Räknare för korsningar så vi väntar tillräckligt innan svängen.
uint32_t crossing_timer = 0;
/// Räknare för backningen efter att objektet är uppplockat.
uint32_t backwards_timer = 0;
///@}

/** @name Maxvärden 
 *  Maxvärden till räknare som innebär att åtgärden ska utföras.
 *  Dessa kan ändras via TWI så att det går att byta tidsfördröjningar från datorn.
 */
///@{

/// Maxvärde för crossing_timer
uint32_t crossing_timer_max = CROSSING_TIMER_MAX;
/// Maxvärde för line_start_timer
uint16_t line_start_timer_max = LINE_START_MAX;
/// Maxvärde för backwards_timer
uint32_t backwards_timer_max = CROSSING_TIMER_MAX;
///@}

/** @name Modevariabler
 * Variabler som definerar vilket läge styrenheten är i just nu.
 */
///@{

/// Mode-variabel som sätts då roboten är på väg tillbaka.
uint8_t driving_back = 0;
/// Mode-variabel som sätts då roboten är i autonomt läge.
uint8_t autonomous = 0;
/// Mode-variabel som kontrollerar vilket läge roboten är i.
uint8_t mode = MODE_STRAIGHT;
/// Mode-variabel som för att hantera vändningen.
uint8_t end_of_line_follow = 0;
///@}

/** @name Bussdata 
 *  Variabler där data mottagen från TWI sparas
 */
///@{

/// Senast mottagna manuella kommando.
uint8_t manual_command = STOP;
/// Diff mottagen från sensorenheten.
uint8_t diff = 127;
/// Rotation mottagen från sensorenheten.
uint8_t rot = 127;
/// Den diod där det är mest troligt att tejpen finns.
uint8_t tape_position = 5;
/// Hur många dioder som upptäcker tejp.
uint8_t num_diods = 0;
/// Sparar senaste tejpmarkering
uint8_t last_tape_detected = 0;
/// Sparar maxhastigheten
int16_t main_max_speed = 250;
///@}

/** @name Tillbakaväg
 *  Variabler som hanterar tillbakavägen.
 */
///@{

/// Sparar hur vi har kört på väg in i labyrinten
uint8_t way_home[20];
/// Pekare för way_home[].
uint8_t way_home_iterator = 0;
///@}

/** @name Regulatorparametrar.
 */
///@{
/// P-parameter
uint8_t k_p = K_P;
/// D-parameter
uint8_t k_d = K_D;
///@}

/** @name Manuella motorkommandon.
 */
///@{
/// Höger hjuls riktning.
rDir = 0;
/// Vänster hjuls riktning.
lDir = 0;
/// Höger hjuls fart.
rSpeed = 3;
/// Vänster hjuls fart.
lSpeed = 3;
///@}

/** Initiera interrupts.
 *  INT0 används för att upptäcka knapptryck på den blå knappen, denna byter mellan autonomt och manuellt läge.
 *  INT1 används för att ta emot korsnings och gyrodata från sensorn.*/
void init_interrupts(void){
  // Set interrupt on rising edge of INT0 and INT1 pin
  MCUCR = (MCUCR & 0xf0) | 0x0f;
  // Set INT0 and INT1 as input pins.
  DDRD = (DDRD & 0xf3);
  //Enable interrupts on INT0 and INT1
  GICR |= 0b11000000;
}
/** @name Lägesrutiner 
 *  Dessa rutiner anropas för att byta läge på sensorenheten på ett systematiskt sätt.
 *  Följande lägen finns:
 *
 *  1. MODE_STRAIGHT Detta läge är till för att köra rakt i raksträckor på labyrinten. PD-reglering körs med
 *     data från sensorenheten.
 *
 *  2. MODE_LINE_FOLLOW Detta läge följer linjen framför roboten, när roboten kommer fram till ett tvärstreck
 *     byts läget till "turn around", gripklon stängs och roboten påbörjar tilbakavägskörningen.
 *
 *  3. MODE_CROSSING Detta läge hanterar när roboten kommer in i en korsning. Den kör en timer som räknar upp så
 *     att roboten kommer till mitten av korsningen. När detta skett kontrollerar den vilket håll den ska svänga
 *     och byter till rätt mode.
 *
 *  4. MODE_FORWARD/LEFT/RIGHT_TURN Dessa lägen ser till att roboten gör 90 gradersvängar eller fortsätter rakt fram.
 *     Dessa funktioner kommer också att fixa sparning av tillbakvägsdata.
 *
 *  5. MODE_TURN_AROUND Vänder roboten och börjar sedan tillbakavägskörning.
 *
 *  6. MODE_TURN_COMPLETE Ser till att roboten kan ta sig ut ur en korsning efter att rätt sväng blivit utförd.
 *     Denna kör en timer och sätter sedan MODE_STRAIGHT.
 *
 *  7. MODE_FINISH Detta mode stoppar roboten och går över till manuellt mode.
 */

///@{

/** Byt till regleringsläge.
  *  Sätter roboten i läget där den reglerar efter väggarna i labyrinten.
  */
void set_mode_straight(){
  send_sensor_mode(MODE_STRAIGHT);
  mode = MODE_STRAIGHT;
}

/** Byt till linjeföljningsläge.
 *  Nollställ linjeföljningsvariabler så den börjar med att åka rakt, samt skicka MODE_LINE_FOLLOW till sensorenheten.
 */
void set_mode_line_follow(){
	set_speed(180, 3, 3);
	backwards_timer = 0;
	send_sensor_mode(MODE_LINE_FOLLOW);
  	num_diods = 1;
	tape_position = 5;
	line_start_timer = 0;
	mode = MODE_LINE_FOLLOW;
	griparm(OPEN);
}

/** Byt till korsningsläge.
 *  I detta läge väntar roboten på att crossing_timer ska räknas upp till crossing_timer_max.
 *  När detta skett ska vi vara i mitten på korsningen. Om sensorenheten anser att den se väggar igen 
 *  så avbryts räkningen och roboten återgår till att att reglera på väggarna.
 *  Roboten kör rakt fram i detta läge.
 */
void set_mode_crossing(){
	mode = MODE_CROSSING;
	crossing_timer = 0;
}

/** Byt till vänstersvängsläge.
 *  I detta läge roterar roboten 90 grader åt vänster. Detta sker genom att roboten säger åt sensorenheten att
 *  byta till gyroläge så att den skickar ett interrupt då den roterat klart.
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

/** Byt till högersvängsläge.
 *  I detta läge roterar roboten 90 grader åt höger. Detta sker genom att roboten säger åt sensorenheten att
 *  byta till gyroläge så att den skickar ett interrupt då den roterat klart.
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

/** Byt till raktframläge
 *  Denna funktion byter egentligen till "turn complete"-läget, 
 *  vi måste dock spara undan vägvalet för att hitta tillbaka.
 */
void set_mode_forward_turn(){
  if(driving_back)
    --way_home_iterator;
  else
		way_home[++way_home_iterator] = 2;
  set_mode_turn_complete();
}

/** Byt till läget "turn around"
 *  Detta är ett läge som gör en vänstersväng och sedan byter till det vanliga västersvängsläget.
 *  Här måste vi se upp med way_back så det inte händer några dumheter.
 *  Här krävs lite av en fullösning. Den här svängen och framförallt den efterföljande vanliga svängen 
 *  sker i med driving_back satt. Detta betyder att den vanliga svängrutinen kommer räkna ner way_home_iterator, 
 *  därför räknas den först upp här.
 */
void set_mode_turn_around(){
	set_speed(main_max_speed, 3, 3);
 	mode = MODE_TURN_AROUND;
	send_sensor_mode(MODE_GYRO);
 	driving_back = 1;
}

/** Byt till "turn complete"
 *  Detta läge markerar att svängen är klar. Detta innebär att roboten kör rakt fram till dess att den återigen
 *  ser väggar och kan börja reglera. Dessutom körs även här crossing_timer för att förhindra att roboten
 *  först av misstag tror att den ser väggar byter till reglerläget och sedan upptäcker (korrekt)
 *  att den är i en korsning och börjar svänga. På detta sätt måste crossing_timer hinna räknas upp två gånger
 *  innan roboten kan svänga igen (en gång i detta läge och sedan än gång i korsningsläget).
 */
void set_mode_turn_complete(){
  crossing_timer = 0; // För att hindra roboten att tro att den är ute ur svängen för tidigt.
	mode = MODE_CROSSING_FORWARD;
	send_sensor_mode(MODE_STRAIGHT);
  last_tape_detected = 0;
}

/** Byt till klar läget.
 *  Detta läge stoppar roboten och hänger sig sedan i en oändlig loop roboten är klar med uppdraget och ska inte göra mer.
 */
void set_mode_finish(){
  stop();
  autonomous = 0;
  mode = STOP;
}

///@}

/** @name Interruptrutiner
 * Interruptrutiner som hanterar avbrott i styrenheten. Hanterar knappar och sensorsignaler.
 */
///@{
/** Fångar felaktiga interrupt om något går snett. Tänkt att användas under debuggning med JTAG så man vet vad som pågår
 */
ISR(BADISR_vect){ 	
  volatile uint8_t c;
	while(1) ++c;
}

/** Interruptrutin för den blå knappen, togglar mellan autonomt och icke autonomt läge.
  */
ISR(INT0_vect){
	autonomous = 1 - autonomous;
	return;
}

/** Interruptrutin för att ta emot tidskritisk information från sensorenheten.
  * Interrupt skickas i följande fall:
  * 1. Sensorenheten har upptäckt att den är i en korsning. Då skickas också data 
  * om vilket håll sensorenheten tror att vi ska svänga på PINB.
  * 2. Sensorenheten är i gyroläge och anser att vi svängt klart. Då skickas också MODE_GYRO_COMPLETE på PINB.
  * 3. Sensorenheten har upptäckt 4 tejpar och vi måste byta till linjeföljningsläge snabbt. MODE_LINE_FOLLOW skickas på PINB.
  */
ISR(INT1_vect){
	if(autonomous){
		if (mode == MODE_CROSSING_FORWARD) return;
		uint8_t t_mode = MODE_FROM_SENSOR;
	
		switch(t_mode){
			case MODE_LINE_FOLLOW:
	      if(driving_back == 0) set_mode_line_follow();
				break;
			case MODE_GYRO_COMPLETE:
				if(mode == MODE_TURN_AROUND) set_mode_straight();
				else set_mode_turn_complete();
				break;
	    default:
	      set_mode_crossing();
				break;
		}
	}
}
///@}


/** @name Modehantering
  * Här följer några rutiner som hanterar vad som händer då vi är i något av moderna.
  * Där det behövs finns rutinen för moden, den anropas i auto_control.
  * Inget sker i auto_control för att få en konsekvent sätt att hantera moder.
 */

///@{

/** Rutin för att verifiera att roboten är i en korsning och sedan svänga.
 * Vänta först så att roboten hinner in till mitten av korsningen, om sensorerna ser väggar var det falskt alarm.
 * När roboten är i mitten så kolla sensorn för att avgöra vilket håll svängen ska vara.
 */
void crossing(void){
	forward(); // 
                                           // om de inte ser väggar och annars reglera efter väggen den ser.
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
 *  Räknar upp crossing_counter och byter sen till MODE_STRAIGHT.
 */
void crossing_forward(){
	forward(); // 
                                           // om de inte ser väggar och annars reglera efter väggen den ser.
	if (crossing_timer < (crossing_timer_max << 2)){
		++crossing_timer;
		return;
	}
	else if(MODE_FROM_SENSOR == MODE_STRAIGHT) set_mode_straight();
}

/** Hanterar MODE_LINE_FOLLOW
 *  Räkna upp line_start_timer så vi inte tror att den sista tejpen i markeringen är slutmarkering.
 *  Kör run_line_follow och byt mode om vi är framme.
 */
void line_follow(){

	if(line_start_timer < line_start_timer_max){
		++line_start_timer;
		forward();
		return;
	}
	if(!driving_back) end_of_line_follow = run_line_follow(num_diods, tape_position);
	
	if(end_of_line_follow == END_TAPE){
		driving_back = 1;
		backward();

		if(backwards_timer < backwards_timer_max){
			++backwards_timer;
			return;
		}
		else set_mode_turn_around();
			
	}
	else if(end_of_line_follow == NO_TAPE) run_straight(diff, rot, k_p, k_d, TRUE);
}
///@}

/** Manuell körning.
 *  Läser in senaste mottagna manuella kommando och utför.
 */
void manual_control(){
	run_straight(diff, rot, k_p, k_d, FALSE); // Kör PD-reglering utan att skicka något till hjulen (FALSE),
                                            // för att kunna se parametrar på datorn även i manuellt läge.
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
		case CMD_MANUAL_SPEED:
			manual_speed(rDir, lDir, rSpeed, lSpeed);
			break;
	}
}

/** Autonom körning.
 *  Läs vilket mode roboten är i och kör rätt funktion.
 */
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

/** Kontrollera busmeddelanden.
 *  Hanterar bussen, läser in från busskön och hanterar. Sätter globala variabler som sedan används av andra funktioner.
 */
uint8_t check_TWI(){
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
	  if(s[2] == OPEN) griparm(OPEN);
	  else if(s[2] == CLOSE) griparm(CLOSE);
      else manual_command = s[2];
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
			main_max_speed = s[i+1];
        	set_speed(s[i+1], 3, 3);
        }
		if(s[i] == REG_TIMER){
		  crossing_timer_max = ((uint32_t) s[i+1]) << 12;
		}
      }
      break;
	case CMD_MANUAL_SPEED:
          autonomous = 0;
	  manual_command = CMD_MANUAL_SPEED;
	  for(uint8_t i = 2; i < len; i = i+2){
            if(s[i] == RDIR){
              rDir = s[i+1];
        }
	    if(s[i] == LDIR){
              lDir = s[i+1];
        }
	    if(s[i] == RSPEED){
	      rSpeed = s[i+1];
        }
	    if(s[i] == LSPEED){
	      lSpeed = s[i+1];
	}
      }
      break;
	  case CMD_AUTO_ON:
      autonomous = 1;
      break;
    }
  	return 1;
  }
  else return 0;
}

/** Mainloop
 *  Kör alla initieringar och sedan går in i huvudloopen.
 *  Kör check_TWI() och sedan antingen auto_control eller manual_control.
 */
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
 	    while(check_TWI());

	    if(autonomous){
			auto_control();
	    }

		else {
			manual_control();
		}
	}
}
