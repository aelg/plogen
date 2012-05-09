/** @file
 * Programfil f�r kommunikationsenheten.
 *
 * Kommunikationsenheten g�r att roboten kan kommunicera, via bl�tand, med en dator. 
 * Detta sker genom busskontakt med de andra tv� enheterna, samt vi en bl�tandsmodul
 * inkopplad via ett UART-interface.
 *
 * Det som g�rs i kommunikationsenheten �r att kopiera meddelanden fr�n bussen till UART-interfacet.
 * Data som kommer fr�n UART-interfacet m�ste filtreras lite f�r att kunna skickas till r�tt enhet.
 * En del ska till styrenheten och en del till sensorenheten.
 *
 *  Created on: 15 mar 2012
 *      Author: aelg
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>

#include "../UART/UART.h"
#include "../TWI/TWI.h"
#include "../commands.h"

/** Mainloop.
 * H�r sker allt i som kommunikationsenheten g�r. L�s in data fr�n den ena kanalen och skicka p� den andra.
 */
int main(void)
{
	UART_init();
	TWI_init(COMM_ADDRESS);
	sei();

	uint8_t end[2] = {CMD_END, 0};
	

	uint8_t buff[16];

	while (1){
		uint8_t len;
		len = UART_read(buff);
		if(len){
			switch(buff[0]){
			case CMD_SEND_NEXT:
				UART_write(end, 2);
				break;
			case CMD_MANUAL:
			case CMD_SET_REG_PARAMS:
            case CMD_AUTO_ON:
				TWI_write(CONTROL_ADDRESS, buff, len);
				break;
			}
		}
		len = TWI_read(buff);
 		if(len){
			switch(buff[0]){
			case CMD_SENSOR_DATA:
      case CMD_REG_PARAMS:
				UART_write(buff, len);
				break;
			}
		}
	}
}
