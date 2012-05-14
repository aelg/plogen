/**
 * \defgroup buss I2C-buss och protokoll
 * @{
 */

/** @file
 * H�r finns kod som hanterar I2C bussen. Denna hanteras genom tv� arrayer som agerar
 * cirkul�ra listor f�r att spara data som tagits emot men inte hunnit behandlas alternativt
 * ska skickas men inte hunnit iv�g �nnu. Eftersom dessa listor �r begr�nsade kr�ver detta att
 * den inkommande listan t�ms regelbundet av resten av programmet. 
 * Skulle bussen bli upptagen f�r l�nge kommer ocks� den utg�ende listan bli full.
 *
 * I2C har 4 l�gen
 * - MR Master Reciever
 * - MT Master Transmitter
 * - SR Slave Reciever
 * - ST Slave Transmitter
 *
 * De l�gen som implementerats �r MT och SR data skickas allts� som Master och tas emot som 
 * Slave. Eftersom alla enheter anv�nder denna kod betyder det att roboten har flera Masters
 * p� bussen. F�rdelen �r att det blir mindre l�gen att implementera och samma kod k�rs �verallt.
 * Nackdelen �r att flera Masters kan skriva p� bussen samtidigt och allts� m�ste hantering
 * f�r arbitrationfel implementeras. MR och ST �r inte implementerade �verhuvudtaget.
 *
 * Funktionerna TWI_read() och TWI_write() �r de som ska anv�ndas utanf�r den h�r filen.
 * Skrivs paket till dessa s� ska de flyga iv�g p� bussen helt automatiskt.
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "../error/error.h"
#include "TWI.h"

/** @name TWCR-konstanter.
 * Namngivna konstanter f�r TWCR i I2C-interruptet.
 */
///@{ 
//TWCR BITS(TWxx):   INT EA STA STO WC EN Res IE
#define SEND 0xc5 // 1   1  0   0   0  1  0   1
#define STOP 0xd5 // 1   1  0   1   0  1  0   1
#define START 0xe5// 1   1  1   0   0  1  0   1
#define ACK 0xc5  // 1   1  0   0   0  1  0   1
#define NACK 0x85 // 1   0  0   0   0  1  0   1
#define RESET 0xc5// 1   1  0   0   0  1  0   1
///@}

/* @name Buffertar
 * Buffertar och pekare till dem. 
 */
///@{
// Dessa �ndras av b�de interrupts och vanlig kod och borde vara volatile.
// Tror egentligen inte att det �r n�dv�ndigt.
volatile uint8_t tread_buff[TWI_BUFFER_SIZE]; ///< Inkommande buffer.
volatile uint8_t twrite_buff[TWI_BUFFER_SIZE]; ///< Utg�ende buffer.
volatile uint8_t tread_end; ///< Pekare till elementet efter det sista som anv�nds i inkommande bufferten.
volatile uint8_t twrite_start; ///< Pekare till f�rsta elementet i utg�ende bufferten. Det som ska skickas.


uint8_t tread_start; ///< Pekare till f�rsta elementet i inkommande bufferten. Det som ska l�sas.
uint8_t twrite_end; ///< Pekare till elementet efter det sista som anv�nds i utg�ende bufferten.
///@}

/** @name Interruptpekare
 * Pekare som anv�nds i interrupten f�r att inte tappa bort saker om fel skulle uppst� under �verf�ring.
 */
///@{
uint8_t twrite_current;
uint8_t tread_current;
uint8_t tremaining_bytes;
///@}

/** TWI calculate address.
 * Hanterar cirkulariteten och ser till att pekaren inte hamnar utanf�r listan.
 */
inline uint8_t TWIca(uint8_t);
uint8_t TWIca(uint8_t addr){
	return addr & 0x1f;
}

/** Hantera TWI-interruptet.
 *  L�ser in SR som s�ger vilket l�ge TWI-h�rdvaran �r och g�r det som beh�vs. (Use the source, Luke).
 *  
 *  TWI beh�ver veta vem den ska skicka paketet till s� f�re varje paket i den utg�ende bufferten ska
 *  adressen p� mottagaren ligga.
 */

ISR(TWI_vect){
	uint8_t sr = TWSR & 0xf8;
	switch(sr){
	case 0x8: // Start har skickats.
	case 0x10: // Repeated start har skickats.
		// Kopiera twrite_start till current_start s� vi vet vad som ska skickas nu,
		// l�t twrite_start vara, om arbitration error.
		twrite_current = twrite_start;
		// L�s antalet byte i paketet, utg� ifr�n att det ligger ett helt paket i k�n (TWI_write �r snabbare �n bussen).
		// L�gg till 2 f�r att f� med kommando och l�ngdbyterna.
		tremaining_bytes = twrite_buff[TWIca(twrite_start+2)]+2;
		// Skicka adress, t�mm W/R f�r att g� in i MT.
		TWDR = twrite_buff[twrite_current] << 1;
		TWCR = SEND;
		twrite_current = TWIca(twrite_current + 1);
		break;
	case 0x18: // SLA+W skickat, ACK mottagen.
	case 0x28: // Data skickat, ACK mottagen.
		TWDR = twrite_buff[twrite_current];
		// Finns n�got kvar att skicka?
		if(tremaining_bytes == 0){
			// Finns fler paket att skicka efter detta?
			if(twrite_current == twrite_end){
				// Slut p� paket skicka STOP;
				TWCR = STOP;
			}
			else {
				// Finns mer, skicka START
				TWCR = START;
			}
			// Uppdatera start_twrite, inga arbitrationfel.
			twrite_start = TWIca(twrite_current);
		}
		else {
			// Skicka en byte.
			--tremaining_bytes;
			TWCR = SEND;
			twrite_current = TWIca(twrite_current+1);
		}
		break;
	case 0x38: // Arbitration fel.
		// We vill fortfarande skicka det som vi h�ll p� med, skicka START n�r bussen blir ledig.
		TWCR = START;
		break;
	case 0x20: // Not ACK mottagen efter SLA+W, ok�nd adress.
		error(TWI_UNKNOWN_ADDRESS);
		// Ta bort paketet.
		while(tremaining_bytes--)
			++twrite_current;

		twrite_start = TWIca(twrite_current);

		// Kolla om det finn fler paket att skicka.
		if(twrite_start == twrite_end){
			// Inget mer, skicka STOP.
			TWCR = STOP;
		}
		else{
			// Fler paket i k�n, skicka REPEATED START.
			TWCR = START;
		}
		break;
	case 0x30: // Not ACK mottagen efter data.
		// Signalera fel.
		error(TWI_NOT_ACK_RECEIVED);
		// F�rs�k igen och hoppas att mottagaren har rensat bufferten.
		TWCR = START;
		break;
	case 0x60: // Egen SLA+W mottagen, ACK skickad tillbaka.
	case 0x68: // Arbitration fel i MR mode, egen SLA+W mottagen, ACK skickad tillbaka.
	case 0x70: // General call adress mottagen.
	case 0x78: // Arbitration fel i MR mode, general call adress mottagen, ACK skickad tillbaka.
		// L�s byte
		//tread_buff[tread_current] = TWDR;
		tread_current = TWIca(tread_end);
		// Kolla om bufferten �r full.
		if(TWIca(tread_current+1) == tread_start){
			// Skicka NOT ACK.
			TWCR = NACK;
		}
		else { // Skicka ACK
			TWCR = ACK;
		}
		break;
	case 0x80: // Data mottagen, ACK sent. (Adresserad)
	case 0x90: // Data mottage, ACK sent. (General call)
		// L�s byte
		tread_buff[tread_current] = TWDR;
		tread_current = TWIca(tread_current+1);
		// Kolla om bufferten �r full.
		if(tread_current == tread_start){
			// Skicka NOT ACK.
			TWCR = NACK;
		}
		else { // Skicka ACK
			TWCR = ACK;
		}
		break;
	case 0x88: // Data mottagen, NACK skickad. (Adresserad)
	case 0x98: // Data received, NACK skickad. (General call)
		// Reset och hoppas att det finns plats i n�sta g�ng.
		TWCR = RESET;
		error(TWI_READ_BUFFER_FULL);
		break;
	case 0xA0: // STOP mottagen.
		tread_end = tread_current;
		//Kolla om vi vill skicka data.
		if(twrite_start != twrite_end)
			TWCR = START;
		else
			TWCR = RESET;
		break;
	case 0x00: // Bus fel pga felaktig START or STOP
		// Skicka STOP f�r att �terst�lla h�rdvara, inget STOP kommer skickas.
		error(TWI_BUS_ERROR);
		TWCR = STOP;
		break;
	default: // Ska inte h�nda.
		error(TWI_UNKNOWN_STATUS);
		break;
	}
}

/** S�tter ig�ng bussen. Skickar ett f�rsta START-condition f�r att dra ig�ng alltihop.
 * Efter det s� kommer allt sk�tas av interruptrutinen.
 */
void TWI_start(){
	TWCR = START;
}

/** Initierar I2C-h�rdvaran.
 * S�tter adressen och bitrate s� att bussen fungerar.
 */
void TWI_init(uint8_t sla){
	tread_start = tread_end = twrite_start = twrite_end = 0;
	// S�tt bitrate.
	TWBR = 0x80;//0x0c;
	// S�tt slave adress och ta emot general calls.
	TWAR = (sla << 1) | 0x01;
	// S�krast s�.
	TWSR = 0;
	// Reset f�r att vara redo.
	TWCR = RESET;
}
	
/** L�gger till paket i utg�ende buffert.
 * Tar ett paket och l�gger till paketet i den utg�ende buffertern. 
 * Kontrollerar s� att det f�r plats och s�tter ig�ng bussen s� det skickas.
 */
uint8_t TWI_write(uint8_t addr, uint8_t *s, uint8_t len){
  // Spara start lokalt ifall den skulle �ndras av ett interrupt.
  uint8_t start = twrite_start;
  // Tempor�r end f�r att hantera cirkul�r buffer.
	uint8_t end;
  // Se till att end alltid �r st�rre �n start.
	if(start > twrite_end){
	  end = twrite_end + TWI_BUFFER_SIZE;
	}
	else end = twrite_end;
  // Returnera false om buffertern �r full.
	if(len + 1> TWI_BUFFER_SIZE - 1 -(end - start)){
		// TODO: signalera buffer fel.
		return 0;
	}
	twrite_buff[twrite_end] = addr;
	twrite_end = TWIca(twrite_end+1);
  // Kopiera meddelande till bufferten.
	for(int i = 0; i < len; ++i){
		twrite_buff[twrite_end] = s[i];
		twrite_end = TWIca(twrite_end+1);
	}
	TWI_start();
	return 1;
}

/** L�ser ett paket fr�n den inkommande buffertern.
 * Tar ett paket fr�n den inkommande buffertern och l�gger paketet i parametern s.
 * Returnerar l�ngden p� paketet. 0 om inget helt paket finns.
 */
uint8_t TWI_read(uint8_t* s){
  // Tempor�r variabel f�r att hanter cirkul�ra listan.
	uint8_t end;
  // R�kna ut l�ngden p� listan
  // Atomic ifall tread_end skulle �ndras av interrupt.
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
	  if(tread_end < tread_start)
		  end = tread_end + TWI_BUFFER_SIZE;
	  else end = tread_end;
  }
  // Kolla om tread_buff inneh�ller korrekt antal byte.
	if(end-tread_start >= 2){
    // L�s l�ngden fr�n paketet.
		uint8_t len = tread_buff[TWIca(tread_start+1)]+2;
      // Kolla om det finns nog med byte i read_buff
			if(len <= end-tread_start){
				// Kopiera till s.
				for(uint8_t i = 0; i < len; i++){
					s[i]=tread_buff[tread_start];
					tread_start =TWIca(tread_start + 1);
					}
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
