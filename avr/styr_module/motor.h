#ifndef MOTOR_H
#define MOTOR_H

//Constants, LIGGER NU I COMMANDS_H..
/*#define CROSSING 0x08 //onödig? väntläge tom mitten?
#define CROSSING_LEFT 0x00
#define CROSSING_RIGHT 0x01
#define CROSSING_FORWARD 0x03

#define STRAIGHT 0x04
#define LINE_FOLLOW 0x05

#define TURN 0x05 // onödig?
#define TURN_LEFT 0x06
#define TURN_RIGHT 0x07
#define TURN_FORWARD 0x08*/  

#define LEFT 0x0A
#define RIGHT 0x0B
#define FORWARD 0x0C
#define REVERSE 0x0D
#define ROTATE_LEFT 0x0E
#define ROTATE_RIGHT 0x0F
#define STOP 0x10

#define NO_TAPE 2
#define END_TAPE 1

#define FALSE 0
#define TRUE 1

#define ROTATION_COMPLETE 0x11 //onödig?

#define K_P 2 //P-konstant i PD-regulator
#define K_D 14 //D-konstant i PD-regulator

#define REGULATOR_CORR 4 // Antalet steg man shiftar resultatet i P- resp. D-regleringen. Vilket ger bra varlden.

void griparm(void);
void setup_motor(void);
void rotate_right(void);
void rotate_left(void);
void forward(void);
void turn_left(void);
void turn_right(void);
void stop(void);
void reverse(void);
void interrupts(void);
void run_straight(uint8_t diff, uint8_t rot, uint8_t k_p, uint8_t k_d, uint8_t run);
uint8_t run_line_follow(uint8_t num_diods, uint8_t tape_position);
void set_speed(int16_t max, int16_t turn, int16_t stop);

#endif
