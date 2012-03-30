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
