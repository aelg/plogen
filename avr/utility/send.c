/**
 * \addtogroup buss
 * @{
 */

/** @file
 * Samlar alla funktioner som skickar data på bussen.
 * I den här filen göms all kod som formaterar paketen på olika sätt.
 * Inget sånt ska behöva göras i programlogiken.
 */
#include "../TWI/TWI.h"
#include "../commands.h"

/** Skickar antal upptäckta tejpar i en tejpmarkering.
 * Data skickas både till datorn och styrenheten.
 */
void send_tape(uint8_t tape){
	uint8_t s[4];
	s[0] = CMD_SENSOR_DATA;
	s[1] = 2;
	s[2] = TAPE;
	s[3] = tape;

	TWI_write(GENERAL_CALL, s, 4);
}

/** Skickar rådata från tejpsensorn.
 * Detta är alltså data från sensorn som upptäcker tejpmarkering.
 * Data skickas både till datorn och till styrenheten.
 */
void send_tape_value(uint8_t value){
	uint8_t s[4];
	s[0] = CMD_SENSOR_DATA;
	s[1] = 2;
	s[2] = TAPE_VALUE;
	s[3] = value;

	TWI_write(COMM_ADDRESS, s, 4);
}

/** Skickar rådata från IR-sensorerna till datorn.
 * - ll Long left
 * - lr Long right
 * - sl Short left
 * - sr Short right
 * - sb Short back
 */
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

/** Skickar antal dioder som upptäcker tejp under linjeföljning.
 * Används för att avgöra om roboten ser någon tejp alls, eller om 
 * den är framme vid slutet på målområdet.
 * Data skickas till styrenheten.
 */
void send_number_of_diods(uint8_t diod){
	uint8_t s[4];
	s[0]= CMD_SENSOR_DATA;
	s[1]= 2;
	s[2]= DIOD;
	s[3]= diod;

	TWI_write(CONTROL_ADDRESS,s,4);
}


/** Skickar reglerdata till styrenheten.
 * Det som skickas är differensen av avståndet till väggarna, diff, och
 * differensen av de bakre och främre högra sensorerna, rot.
 */
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

/** Skickar numret på den diod som starkast markerar tejp.
 * Observera att bara för att en markerar starkast behöver inte tejp finnas alls.
 * Data skickas till styrenheten.
 */
void send_line_pos(uint8_t pos){
	uint8_t s[4];
	s[0] = CMD_SENSOR_DATA;
	s[1] = 2;
	s[2] = LINE_POSITION;
	s[3] = pos;

	TWI_write(GENERAL_CALL, s, 4);

}

/** Skickar data från de långa IR-sensorerna till styrenheten.
 * Denna funktion används nog inte längre.
 */
void send_long_ir_data(uint8_t long1, uint8_t long2){

	uint8_t s[6];
	s[0] = CMD_SENSOR_DATA;
	s[1] = 4;
	s[2] = IR_LONG_LEFT;
	s[3] = long1;
	s[4] = IR_LONG_RIGHT;
	s[5] = long2;

	TWI_write(CONTROL_ADDRESS, s, 6);
	
}

/** Skickas av styrenheten till sensorenheten för att byte mode i denna.
 * Används för att växla mellan STRAIGHT, LINE_FOLLOW och GYRO moderna i 
 * sensorenheten.
 */
void send_sensor_mode(uint8_t mode){
	uint8_t s[3];
	s[0] = CMD_SENSOR_MODE;
	s[1] = 1;
	s[2] = mode;

	TWI_write(SENSOR_ADDRESS, s, 3);

}

/** Skickar reglerparametrar till datorn från styrenheten.
 * Skickar de värden som räknats fram av regleralgoritmen och 
 * som används för att reglera roboten. Det är dessa som plottas i 
 * datorgränsnittet.
 */
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

/*
 * @}
 */
