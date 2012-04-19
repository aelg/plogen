#include "../TWI/TWI.h"
#include "../commands.h"

void send_tape(uint8_t tape){
	uint8_t s[4];
	s[0] = CMD_SENSOR_DATA;
	s[1] = 2;
	s[2] = TAPE;
	s[3] = tape;

	TWI_write(COMM_ADDRESS, s, 4);
}

void send_tape_value(uint8_t value){
	uint8_t s[4];
	s[0] = CMD_SENSOR_DATA;
	s[1] = 2;
	s[2] = TAPE_VALUE;
	s[3] = value;

	TWI_write(COMM_ADDRESS, s, 4);
}

void send_sensor_values(uint8_t ll, uint8_t lr, 
						uint8_t sl, uint8_t sr, 
						uint8_t sb){
	uint8_t s[12];
	s[0] = CMD_SENSOR_DATA;
	s[1] = 10;
	s[2] = IR_LONG_LEFT;
	s[3] = ll;
	s[4] = IR_LONG_RIGHT;
	s[5] = lr;
	s[6] = IR_SHORT_LEFT;
	s[7] = sl;
	s[8] = IR_SHORT_RIGHT;
	s[9] = sr;
	s[10] = IR_SHORT_BACK;
	s[11] = sb;

	TWI_write(COMM_ADDRESS, s, 12);
}


//Funktion för att skicka differensen
void send_differences(uint8_t diff, uint8_t rot){
	uint8_t s[6];
	s[0] = CMD_SENSOR_DATA;
	s[1] = 4;
	s[2] = IRDIFF;
	s[3] = diff;
	s[4] = IRANGLE;
	s[5] = rot;

	TWI_write(CONTROL_ADDRESS, s, 6);
}

void send_line_pos(uint8_t pos){
	uint8_t s[4];
	s[0] = CMD_SENSOR_DATA;
	s[1] = 2;
	s[2] = LINE_POSITION;
	s[3] = pos;

	TWI_write(COMM_ADDRESS, s, 4);

}

