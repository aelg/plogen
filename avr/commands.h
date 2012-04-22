//Commands

/* CMD_MANUAL skickas till styrenheten, datadelen av paketet
*  ska innehålla en av konstanterna under Manuell kontroll.*/
#define CMD_MANUAL 0x12

// CMD_SEND_NEXT och CMD_END Används inte just nu, kanske vill använda dessa senare.
#define CMD_SEND_NEXT 0x13
#define CMD_END 0x14

/* CMD_SENSOR_DATA skickas till datorinterfacet och till styrenheten.
 * Detta kommando används för att skicka sensordata.
 * Datadelen ska innehålla par av konstanter och data.
 * Först en konstant från de under Sensordata och sedan motsvarande data.
 * Efter det kan en till konstant komma med efterföljande data osv,
 * så länge inte begränsningen på 8 databytes överskrids.
 * Ex {CMD_SENSOR_DATA, 4, IRLEFT, 23, IRRIGHT, 19} motsvarar ett paket med 
 * värdet 23 i vänster irsensor och 19 i höger.*/
#define CMD_SENSOR_DATA 0x15

/* CMD_SENSOR_RAW anänds på samma sätt som CMD_SENSOR_DATA fast denna används
 * för att skicka obehandlad data till datorinterfacet. 
 * Tanken är att lätt kunna avgöra vilka data som styrenheten inte behöver bry sig om.*/
#define CMD_SENSOR_RAW 0x16

/* CMD_CONTROL_DATA används som CMD_SENSOR_DATA fast används för att skicka data 
 * från styrenheten till datorinterfacet. 
 * Här används konstanterna under Styrdatakonstanter */
#define CMD_CONTROL_DATA 0x17

/*CMD_MODE anges f�r att beteckna ett meddelande som inneh�ller ett model�ge
 *som anv�nds i sensorenheten.
 */
#define CMD_MODE 0x18


//Anv�nds f�r att identifiera att ett nytt sensormode kommer
#define SENSOR_MODE 0x21

//Olika modes f�r sensor och styrenhet
#define CROSSING_LEFT 0x00
#define CROSSING_RIGHT 0x01
#define CROSSING_FORWARD 0x02

#define STRAIGHT 0x03

#define TURN 0x04 // on�dig?
#define TURN_LEFT 0x05
#define TURN_RIGHT 0x06
#define TURN_FORWARD 0x07
#define CROSSING 0x08 //on�dig? v�ntl�ge tom mitten? Kan ju verifiera sv�ng

#define COMPLETING_CROSSING 0x19
#define FINISH 0x20


// Manuell kontroll
#define LEFT 0x0A
#define RIGHT 0x0B
#define FORWARD 0x0C
#define BACK 0x0D
#define ROTATE_LEFT 0x0E
#define ROTATE_RIGHT 0x0F
#define STOP 0x10


// Sensordata
#define IR_SHORT_LEFT 1
#define IR_SHORT_RIGHT 2
#define IRROT 3
#define IRDIFF 4
#define TAPE 6
#define TAPE_VALUE 7
#define IR_LONG_LEFT 8
#define IR_LONG_RIGHT 9
#define IR_SHORT_BACK 10
#define LINE_POSITION 11


// Styrdatakonstanter
#define AUTO_MODE 1 // Vilket läge är vi i manuell, raksträcka, tejpsväng osv.
#define MOTOR_LEFT 2
#define MOTOR_RIGHT 3

