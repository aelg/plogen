/** @file
 * Programfil för kommunikationsenheten.
 *
 * Kommunikationsenheten gör att roboten kan kommunicera, via blåtand, med en dator. 
 * Detta sker genom busskontakt med de andra två enheterna, samt vi en blåtandsmodul
 * inkopplad via ett UART-interface.
 *
 * Det som görs i kommunikationsenheten är att kopiera meddelanden från bussen till UART-interfacet.
 * Data som kommer från UART-interfacet måste filtreras lite för att kunna skickas till rätt enhet.
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
 * Här sker allt i som kommunikationsenheten gör. Läs in data från den ena kanalen och skicka på den andra.
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
