#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>

int main(){
PORTB = 1;

while(1) {
	PORTB = PORTB << 1;
	
	if(PORTB == 0)
	PORTB = 1;
}

return 0;
}
