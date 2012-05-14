/**
 * \addtogroup komm
 * @{
 */

/** @file
 * H�r finns kod som hanterar USART bussen som bl�tandsenheten �r kopplad till. Denna hanteras genom tv� arrayer som agerar
 * cirkul�ra listor f�r att spara data som tagits emot men inte hunnit behandlas alternativt
 * ska skickas men inte hunnit iv�g �nnu. Eftersom dessa listor �r begr�nsade kr�ver detta att
 * den inkommande listan t�ms regelbundet av resten av programmet. 
 *
 * Funktionerna TWI_read() och TWI_write() �r de som ska anv�ndas utanf�r den h�r filen.
 * Skrivs paket med dessa s� ska de flyga iv�g p� bussen helt automatiskt.
 */

#include <avr/interrupt.h>
#include <avr/io.h>

#include "../error/error.h"
#include "UART.h"

/* @name Buffertar
 * Buffertar och pekare till dem. 
 */
///@{
// Dessa �ndras av b�de interrupts och vanlig kod och borde vara volatile.
// Tror egentligen inte att det �r n�dv�ndigt.
volatile uint8_t uread_buff[TWI_BUFFER_SIZE]; ///< Inkommande buffer.
volatile uint8_t uwrite_buff[TWI_BUFFER_SIZE]; ///< Utg�ende buffer.
volatile uint8_t uread_end; ///< Pekare till elementet efter det sista som anv�nds i inkommande bufferten.
volatile uint8_t uwrite_start; ///< Pekare till f�rsta elementet i utg�ende bufferten. Det som ska skickas.


uint8_t uread_start; ///< Pekare till f�rsta elementet i inkommande bufferten. Det som ska l�sas.
uint8_t uwrite_end; ///< Pekare till elementet efter det sista som anv�nds i utg�ende bufferten.
///@}

/** UART calculate address.
 * Hanterar cirkulariteten och ser till att pekaren inte hamnar utanf�r listan.
 */
inline uint8_t UARTca(uint8_t);
uint8_t UARTca(uint8_t addr){
	return addr & 0x1f;
}

/** Interrupt f�r UDR Empty.
 * UDR tom och redo att ta emot mer data.
 * Skicka n�sta byte i paketk�n.
 */
ISR(USART_UDRE_vect){
  // Finns mer att skicka?
	if (uwrite_start != uwrite_end){
    // Skriv data till UDR (RXE) f�r att skicka.
		UDR = uwrite_buff[uwrite_start];
    // Uppdatera buffertpekaren.
		uwrite_start = UARTca(uwrite_start+1);
	}
	else {
      // Avaktivera UDRIE f�r att sluta skicka data.
		UCSRB = UCSRB & 0xdf;
	}
}

/** Interrupt f�r Rx Complete. 
 * UDR inneh�ller den senast mottagna byte och �r redo att l�sas.
 * Kopiera UDR till inkommande k�n.
 */
ISR(USART_RXC_vect){
	// �r bufferten full?
	if(uread_start == UARTca(uread_end+1)){
		error(UART_RXE_BUFFER_FULL);
    // L�s UDR f�r att rensa interruptflaggan.
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

/** Initiera USART h�rdvara.
 * Nollst�ller buffertpekare och st�ller in register s� h�rdvaran �r redo.
 */
void UART_init(){
  // S�tt bufferpekare p� b�rjan av bufferten.
	uread_start = uread_end = uwrite_start = uwrite_end = 0;
	// Init UCSRA just to make sure.
  // F�r s�kerhets skull.
	UCSRA = 0x0;
	// Skriv UCSRA; l�ge: Asynkron, ingen paritet, 8 databitar.
	UCSRC = 0x86;
	// Baudrate 115200
	UBRRL = 0x09;
	// Aktivera interrupt p� RX Complete.
	UCSRB = 0x98;
}

/** Starta USART-�verf�ringar.
 * N�r interruptet aktiveras kommer UDR Empty k�ras och paketen kommer b�rja flyga iv�g.
 */
void UART_start(){
    // Enable interrupt on UDRE
    UCSRB = UCSRB | 0x20;
}

/** Skriv paket till UART-k�n.
 * Parametrar:
 * uint8_t* s paket som ska skrivas.
 * Returv�rde: 1 om paketet, fick plats, 0 annars.
 */
uint8_t UART_write(uint8_t *s, uint8_t len){
  // Tempor�r end f�r att hantera cirkul�r buffer.
	uint8_t end;
  // Se till att end alltid �r st�rre �n start.
	if(uwrite_start > uwrite_end){
		end = uwrite_end + UART_BUFFER_SIZE;
	}
	else end = uwrite_end;
  // Returnera false om buffertern �r full.
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

/** L�s f�rsta paketet fr�n inkommande paketk�n.
 * Parametrar:
 * uint8_t* s buffer f�r att ta emot paket.
 * Returv�rde: Paketl�ngd, 0 om inget helt paket finns i bufferten.
 */
uint8_t UART_read(uint8_t* s){
	if(uread_start == uread_end) return 0;
  // Tempor�r variabel f�r att hanter cirkul�ra listan.
	uint8_t end;
	uint8_t start = uread_start;
  // R�kna ut l�ngden p� listan
	if(uread_end < start)
		end = uread_end + UART_BUFFER_SIZE;
	else end = uread_end;
  // Kolla om uread_buff inneh�ller korrekt antal byte.
	if(end - start > 1){
    // L�s l�ngden fr�n paketet.
		uint8_t len = uread_buff[UARTca(start+1)]+2;
      // Kolla om det finns nog med byte i read_buff
			if(len <= end - start){
				// Kopiera till s.
				for(uint8_t i = 0; i < len; i++){
					s[i]=uread_buff[start];
					start =UARTca(start + 1);
					}
				uread_start = start;
        // Returnera l�ngden.
				return len;
			}
			else return 0;
		}
		else return 0;
}

/*
 * @}
 */
