/** @file
 * Kod som anropas n�r n�got g�r fel. Framf�rallt bussen.
 * Denna anv�nds f�r debuggning. S� att det finns n�got att anropa och leta i 
 * om n�got g�r fel. Bra st�lle att s�tta breakpoint p�.
 */

#include "error.h"
//#include "../styr_module/motor.h"

/** Anropas d� n�got som inte borde h�nda h�nder.
 * Bra st�lle att s�tta breakpoint eller k�ra kod som g�r n�got som g�r att uppt�cka.
 * Verkar som att det skickas en del fel under uppstart. D� n�gon av enheterna inte 
 * b�rjat ta emot busskomm �nnu och man f�r ok�nd-adress-NACK.
 */
void error(uint8_t e){
	volatile int c = e;
/*	if(c==2) return;
	while(1){
		stop();
	}*/
	return;
}
