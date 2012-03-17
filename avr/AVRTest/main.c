#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/sleep.h>

int main(){
	while(1){
	PORTB = 0x01;
	PORTB = 0x02;
	PORTB = 0x03;
	PORTB = 0x04;
	PORTB = 0x05;
	}
	return 0;
}
