//Constants
#define CROSSING 0x00 //on�dig? v�ntl�ge tom mitten?
#define CROSSING_LEFT 0x01
#define CROSSING_RIGHT 0x02
#define CROSSING_FORWARD 0x03

#define STRAIGHT 0x04

#define TURN 0x05 // on�dig?
#define TURN_LEFT 0x06
#define TURN_RIGHT 0x07
#define TURN_FORWARD 0x08

#define MANUAL 0x09 // on�dig?
#define MANUAL_LEFT 0x0A
#define MANUAL_RIGHT 0x0B
#define MANUAL_FORWARD 0x0C
#define MANUAL_BACK 0x0D
#define MANUAL_ROTATE_LEFT 0x0E
#define MANUAL_ROTATE_RIGHT 0x0F
#define MANUAL_STOP 0x10

#define ROTATION_COMPLETE 0x11 //on�dig?

void griparm(void);
void setup_motor(void);
void rotate_right(void);
void rotate_left(void);
void drive_forward(void);
void turn_left(void);
void turn_right(void);
void turn_forward(void);
void manual_left(void);
void manual_right(void);
void manual_forward(void);
void manual_stop(void);
void manual_reverse(void);

