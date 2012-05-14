/**
 * \defgroup buss I2C-buss och protokoll
 * @{
 */

/** @file
 * Här finns kod som hanterar I2C bussen. Denna hanteras genom två arrayer som agerar
 * cirkulära listor för att spara data som tagits emot men inte hunnit behandlas alternativt
 * ska skickas men inte hunnit iväg ännu. Eftersom dessa listor är begränsade kräver detta att
 * den inkommande listan töms regelbundet av resten av programmet. 
 * Skulle bussen bli upptagen för länge kommer också den utgående listan bli full.
 *
 * I2C har 4 lägen
 * - MR Master Reciever
 * - MT Master Transmitter
 * - SR Slave Reciever
 * - ST Slave Transmitter
 *
 * De lägen som implementerats är MT och SR data skickas alltså som Master och tas emot som 
 * Slave. Eftersom alla enheter använder denna kod betyder det att roboten har flera Masters
 * på bussen. Fördelen är att det blir mindre lägen att implementera och samma kod körs överallt.
 * Nackdelen är att flera Masters kan skriva på bussen samtidigt och alltså måste hantering
 * för arbitrationfel implementeras. MR och ST är inte implementerade överhuvudtaget.
 *
 * Funktionerna TWI_read() och TWI_write() är de som ska användas utanför den här filen.
 * Skrivs paket till dessa så ska de flyga iväg på bussen helt automatiskt.
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "../error/error.h"
#include "TWI.h"

/** @name TWCR-konstanter.
 * Namngivna konstanter för TWCR i I2C-interruptet.
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
// Dessa ändras av både interrupts och vanlig kod och borde vara volatile.
// Tror egentligen inte att det är nödvändigt.
volatile uint8_t tread_buff[TWI_BUFFER_SIZE]; ///< Inkommande buffer.
volatile uint8_t twrite_buff[TWI_BUFFER_SIZE]; ///< Utgående buffer.
volatile uint8_t tread_end; ///< Pekare till elementet efter det sista som används i inkommande bufferten.
volatile uint8_t twrite_start; ///< Pekare till första elementet i utgående bufferten. Det som ska skickas.


uint8_t tread_start; ///< Pekare till första elementet i inkommande bufferten. Det som ska läsas.
uint8_t twrite_end; ///< Pekare till elementet efter det sista som används i utgående bufferten.
///@}

/** @name Interruptpekare
 * Pekare som används i interrupten för att inte tappa bort saker om fel skulle uppstå under överföring.
 */
///@{
uint8_t twrite_current;
uint8_t tread_current;
uint8_t tremaining_bytes;
///@}

/** TWI calculate address.
 * Hanterar cirkulariteten och ser till att pekaren inte hamnar utanför listan.
 */
inline uint8_t TWIca(uint8_t);
uint8_t TWIca(uint8_t addr){
	return addr & 0x1f;
}

/** Hantera TWI-interruptet.
 *  Läser in SR som säger vilket läge TWI-hårdvaran är och gör det som behövs. (Use the source, Luke).
 *  
 *  TWI behöver veta vem den ska skicka paketet till så före varje paket i den utgående bufferten ska
 *  adressen på mottagaren ligga.
 */

ISR(TWI_vect){
	uint8_t sr = TWSR & 0xf8;
	switch(sr){
	case 0x8: // Start har skickats.
	case 0x10: // Repeated start har skickats.
		// Kopiera twrite_start till current_start så vi vet vad som ska skickas nu,
		// låt twrite_start vara, om arbitration error.
		twrite_current = twrite_start;
		// Läs antalet byte i paketet, utgå ifrån att det ligger ett helt paket i kön (TWI_write är snabbare än bussen).
		// Lägg till 2 för att få med kommando och längdbyterna.
		tremaining_bytes = twrite_buff[TWIca(twrite_start+2)]+2;
		// Skicka adress, tömm W/R för att gå in i MT.
		TWDR = twrite_buff[twrite_current] << 1;
		TWCR = SEND;
		twrite_current = TWIca(twrite_current + 1);
		break;
	case 0x18: // SLA+W skickat, ACK mottagen.
	case 0x28: // Data skickat, ACK mottagen.
		TWDR = twrite_buff[twrite_current];
		// Finns något kvar att skicka?
		if(tremaining_bytes == 0){
			// Finns fler paket att skicka efter detta?
			if(twrite_current == twrite_end){
				// Slut på paket skicka STOP;
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
		// We vill fortfarande skicka det som vi höll på med, skicka START när bussen blir ledig.
		TWCR = START;
		break;
	case 0x20: // Not ACK mottagen efter SLA+W, okänd adress.
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
			// Fler paket i kön, skicka REPEATED START.
			TWCR = START;
		}
		break;
	case 0x30: // Not ACK mottagen efter data.
		// Signalera fel.
		error(TWI_NOT_ACK_RECEIVED);
		// Försök igen och hoppas att mottagaren har rensat bufferten.
		TWCR = START;
		break;
	case 0x60: // Egen SLA+W mottagen, ACK skickad tillbaka.
	case 0x68: // Arbitration fel i MR mode, egen SLA+W mottagen, ACK skickad tillbaka.
	case 0x70: // General call adress mottagen.
	case 0x78: // Arbitration fel i MR mode, general call adress mottagen, ACK skickad tillbaka.
		// Läs byte
		//tread_buff[tread_current] = TWDR;
		tread_current = TWIca(tread_end);
		// Kolla om bufferten är full.
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
		// Läs byte
		tread_buff[tread_current] = TWDR;
		tread_current = TWIca(tread_current+1);
		// Kolla om bufferten är full.
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
		// Reset och hoppas att det finns plats i nästa gång.
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
		// Skicka STOP för att återställa hårdvara, inget STOP kommer skickas.
		error(TWI_BUS_ERROR);
		TWCR = STOP;
		break;
	default: // Ska inte hända.
		error(TWI_UNKNOWN_STATUS);
		break;
	}
}

/** Sätter igång bussen. Skickar ett första START-condition för att dra igång alltihop.
 * Efter det så kommer allt skötas av interruptrutinen.
 */
void TWI_start(){
	TWCR = START;
}

/** Initierar I2C-hårdvaran.
 * Sätter adressen och bitrate så att bussen fungerar.
 */
void TWI_init(uint8_t sla){
	tread_start = tread_end = twrite_start = twrite_end = 0;
	// Sätt bitrate.
	TWBR = 0x80;//0x0c;
	// Sätt slave adress och ta emot general calls.
	TWAR = (sla << 1) | 0x01;
	// Säkrast så.
	TWSR = 0;
	// Reset för att vara redo.
	TWCR = RESET;
}
	
/** Lägger till paket i utgående buffert.
 * Tar ett paket och lägger till paketet i den utgående buffertern. 
 * Kontrollerar så att det får plats och sätter igång bussen så det skickas.
 */
uint8_t TWI_write(uint8_t addr, uint8_t *s, uint8_t len){
  // Spara start lokalt ifall den skulle ändras av ett interrupt.
  uint8_t start = twrite_start;
  // Temporär end för att hantera cirkulär buffer.
	uint8_t end;
  // Se till att end alltid är större än start.
	if(start > twrite_end){
	  end = twrite_end + TWI_BUFFER_SIZE;
	}
	else end = twrite_end;
  // Returnera false om buffertern är full.
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

/** Läser ett paket från den inkommande buffertern.
 * Tar ett paket från den inkommande buffertern och lägger paketet i parametern s.
 * Returnerar längden på paketet. 0 om inget helt paket finns.
 */
uint8_t TWI_read(uint8_t* s){
  // Temporär variabel för att hanter cirkulära listan.
	uint8_t end;
  // Räkna ut längden på listan
  // Atomic ifall tread_end skulle ändras av interrupt.
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
	  if(tread_end < tread_start)
		  end = tread_end + TWI_BUFFER_SIZE;
	  else end = tread_end;
  }
  // Kolla om tread_buff innehåller korrekt antal byte.
	if(end-tread_start >= 2){
    // Läs längden från paketet.
		uint8_t len = tread_buff[TWIca(tread_start+1)]+2;
      // Kolla om det finns nog med byte i read_buff
			if(len <= end-tread_start){
				// Kopiera till s.
				for(uint8_t i = 0; i < len; i++){
					s[i]=tread_buff[tread_start];
					tread_start =TWIca(tread_start + 1);
					}
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
