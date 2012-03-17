#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/sleep.h>

int main(){
	while(1){
	PORTB = 0x01;
	PORTB = 0x01;
	PORTB = 0x01;
	PORTB = 0x01;
	PORTB = 0x01;
	}
	return 0;
}
