/**
 * Signal error. Call to signal errors.
 * TODO: Implement some sort of signaling, LED or bluetooth.
 */

#include "error.h"
#include "../styr_module/motor.h"

void error(uint8_t e){
	volatile int c = e;
	if(c==2) return;
	while(1){
		stop();
	}
	return;
}
