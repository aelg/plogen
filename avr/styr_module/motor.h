/**
 * \addtogroup styr Styrenhet
 * @{
 */

/** @file
 * Definerar konstanter och funktioner som definerats i Motor.c
 */

#ifndef MOTOR_H
#define MOTOR_H

/** @name Returv�rden fr�n run_line_follow().
 * Returnerar vad linjef�ljningsfunktion ser.
 */
///@{
#define NO_TAPE 2
#define END_TAPE 1
#define TAPE_DETECTED 0
///@}

/** @name Sanningskonstanter
 * F�r l�sbarhet.
 */
///@{
#define FALSE 0
#define TRUE 1
///@}

/** @name Regulatorkonstanter
 * Defaultv�rden f�r regulatorn.
 */
///@{
#define K_P 3 ///< P-konstant i PD-regulator
#define K_D 14 ///< D-konstant i PD-regulator
/// Antalet steg man shiftar resultatet i P- resp. D-regleringen.
/// S� att det fortfarande g�r att skicka 8 bitars v�rden p� bussen.
#define REGULATOR_CORR 4
///@}


void griparm(uint8_t);
void setup_motor(void);
void rotate_right(void);
void rotate_left(void);
void forward(void);
void turn_left(void);
void turn_right(void);
void stop(void);
void reverse(void);
void run_straight(uint8_t diff, uint8_t rot, uint8_t k_p, uint8_t k_d, uint8_t run);
uint8_t run_line_follow(uint8_t num_diods, uint8_t tape_position);
void set_speed(int16_t max, int16_t turn, int16_t stop);

#endif

/*
 * @}
 */
