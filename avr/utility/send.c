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

void send_number_of_diods(uint8_t diod){
	uint8_t s[4];
	s[0]= CMD_SENSOR_DATA;
	s[1]= 2;
	s[2]= DIOD;
	s[3]= diod;

	TWI_write(CONTROL_ADDRESS,s,4);
}


//Funktion för att skicka differensen
void send_differences(uint8_t diff, uint8_t rot){
	uint8_t s[6];
	s[0] = CMD_SENSOR_DATA;
	s[1] = 4;
	s[2] = IRDIFF;
	s[3] = diff;
	s[4] = IRROT;
	s[5] = rot;

	TWI_write(GENERAL_CALL, s, 6);
}

void send_line_pos(uint8_t pos){
	uint8_t s[4];
	s[0] = CMD_SENSOR_DATA;
	s[1] = 2;
	s[2] = LINE_POSITION;
	s[3] = pos;

	TWI_write(GENERAL_CALL, s, 4);

}

void send_reg_params(uint16_t p, uint16_t d){
	uint8_t s[8];
	s[0] = CMD_REG_PARAMS;
	s[1] = 6;
	s[2] = REG_P;
	s[3] = p >> 8;
	s[4] = p;
	s[5] = REG_D;
	s[6] = d >> 8;
	s[7] = d;

	TWI_write(COMM_ADDRESS, s, 8);
}

