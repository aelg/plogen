/**
 * \addtogroup komm
 * @{
 */

/** @file
 * Här finns kod som hanterar USART bussen som blåtandsenheten är kopplad till. Denna hanteras genom två arrayer som agerar
 * cirkulära listor för att spara data som tagits emot men inte hunnit behandlas alternativt
 * ska skickas men inte hunnit iväg ännu. Eftersom dessa listor är begränsade kräver detta att
 * den inkommande listan töms regelbundet av resten av programmet. 
 *
 * Funktionerna TWI_read() och TWI_write() är de som ska användas utanför den här filen.
 * Skrivs paket med dessa så ska de flyga iväg på bussen helt automatiskt.
 */

#include <avr/interrupt.h>
#include <avr/io.h>

#include "../error/error.h"
#include "UART.h"

/* @name Buffertar
 * Buffertar och pekare till dem. 
 */
///@{
// Dessa ändras av både interrupts och vanlig kod och borde vara volatile.
// Tror egentligen inte att det är nödvändigt.
volatile uint8_t uread_buff[TWI_BUFFER_SIZE]; ///< Inkommande buffer.
volatile uint8_t uwrite_buff[TWI_BUFFER_SIZE]; ///< Utgående buffer.
volatile uint8_t uread_end; ///< Pekare till elementet efter det sista som används i inkommande bufferten.
volatile uint8_t uwrite_start; ///< Pekare till första elementet i utgående bufferten. Det som ska skickas.


uint8_t uread_start; ///< Pekare till första elementet i inkommande bufferten. Det som ska läsas.
uint8_t uwrite_end; ///< Pekare till elementet efter det sista som används i utgående bufferten.
///@}

/** UART calculate address.
 * Hanterar cirkulariteten och ser till att pekaren inte hamnar utanför listan.
 */
inline uint8_t UARTca(uint8_t);
uint8_t UARTca(uint8_t addr){
	return addr & 0x1f;
}

/** Interrupt för UDR Empty.
 * UDR tom och redo att ta emot mer data.
 * Skicka nästa byte i paketkön.
 */
ISR(USART_UDRE_vect){
  // Finns mer att skicka?
	if (uwrite_start != uwrite_end){
    // Skriv data till UDR (RXE) för att skicka.
		UDR = uwrite_buff[uwrite_start];
    // Uppdatera buffertpekaren.
		uwrite_start = UARTca(uwrite_start+1);
	}
	else {
      // Avaktivera UDRIE för att sluta skicka data.
		UCSRB = UCSRB & 0xdf;
	}
}

/** Interrupt för Rx Complete. 
 * UDR innehåller den senast mottagna byte och är redo att läsas.
 * Kopiera UDR till inkommande kön.
 */
ISR(USART_RXC_vect){
	// Är bufferten full?
	if(uread_start == UARTca(uread_end+1)){
		error(UART_RXE_BUFFER_FULL);
    // Läs UDR för att rensa interruptflaggan.
    // FIXME: DENNA BYTE KOMMER TAPPAS OCH VI KOMMER VAR UR SYNC MED PAKETEN
		asm volatile("ldi r24, 0x2c" : : : "r24");
	}
	else{
    // Kopiera UDR till uread_buff.
		uread_buff[uread_end] = UDR;
		// Uppdatera pekaren.
		uread_end = UARTca(uread_end+1);
	}
}

/** Initiera USART hårdvara.
 * Nollställer buffertpekare och ställer in register så hårdvaran är redo.
 */
void UART_init(){
  // Sätt bufferpekare på början av bufferten.
	uread_start = uread_end = uwrite_start = uwrite_end = 0;
	// Init UCSRA just to make sure.
  // För säkerhets skull.
	UCSRA = 0x0;
	// Skriv UCSRA; läge: Asynkron, ingen paritet, 8 databitar.
	UCSRC = 0x86;
	// Baudrate 115200
	UBRRL = 0x09;
	// Aktivera interrupt på RX Complete.
	UCSRB = 0x98;
}

/** Starta USART-överföringar.
 * När interruptet aktiveras kommer UDR Empty köras och paketen kommer börja flyga iväg.
 */
void UART_start(){
    // Enable interrupt on UDRE
    UCSRB = UCSRB | 0x20;
}

/** Skriv paket till UART-kön.
 * Parametrar:
 * uint8_t* s paket som ska skrivas.
 * Returvärde: 1 om paketet, fick plats, 0 annars.
 */
uint8_t UART_write(uint8_t *s, uint8_t len){
  // Temporär end för att hantera cirkulär buffer.
	uint8_t end;
  // Se till att end alltid är större än start.
	if(uwrite_start > uwrite_end){
		end = uwrite_end + UART_BUFFER_SIZE;
	}
	else end = uwrite_end;
  // Returnera false om buffertern är full.
	if(len > UART_BUFFER_SIZE - 2 - (end - uwrite_start)){
		// TODO: signalera buffer fel.
		return 0;
	}

  // Kopiera meddelande till bufferten.
	for(int i = 0; i < len; ++i){
		uwrite_buff[uwrite_end] = s[i];
		uwrite_end = UARTca(uwrite_end+1);
	}
	UART_start();
	return 1;
}

/** Läs första paketet från inkommande paketkön.
 * Parametrar:
 * uint8_t* s buffer för att ta emot paket.
 * Returvärde: Paketlängd, 0 om inget helt paket finns i bufferten.
 */
uint8_t UART_read(uint8_t* s){
	if(uread_start == uread_end) return 0;
  // Temporär variabel för att hanter cirkulära listan.
	uint8_t end;
	uint8_t start = uread_start;
  // Räkna ut längden på listan
	if(uread_end < start)
		end = uread_end + UART_BUFFER_SIZE;
	else end = uread_end;
  // Kolla om uread_buff innehåller korrekt antal byte.
	if(end - start > 1){
    // Läs längden från paketet.
		uint8_t len = uread_buff[UARTca(start+1)]+2;
      // Kolla om det finns nog med byte i read_buff
			if(len <= end - start){
				// Kopiera till s.
				for(uint8_t i = 0; i < len; i++){
					s[i]=uread_buff[start];
					start =UARTca(start + 1);
					}
				uread_start = start;
        // Returnera längden.
				return len;
			}
			else return 0;
		}
		else return 0;
}

/*
 * @}
 */
