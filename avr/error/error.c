/** @file
 * Kod som anropas när något går fel. Framförallt bussen.
 * Denna används för debuggning. Så att det finns något att anropa och leta i 
 * om något går fel. Bra ställe att sätta breakpoint på.
 */

#include "error.h"
//#include "../styr_module/motor.h"

/** Anropas då något som inte borde hända händer.
 * Bra ställe att sätta breakpoint eller köra kod som gör något som går att upptäcka.
 * Verkar som att det skickas en del fel under uppstart. Då någon av enheterna inte 
 * börjat ta emot busskomm ännu och man får okänd-adress-NACK.
 */
void error(uint8_t e){
	volatile int c = e;
/*	if(c==2) return;
	while(1){
		stop();
	}*/
	return;
}
