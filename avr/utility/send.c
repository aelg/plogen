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
