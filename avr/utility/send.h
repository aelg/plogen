void send_tape_value(uint8_t value);
void send_tape(uint8_t tape);
void send_sensor_values(uint8_t ll, uint8_t lr, 
						uint8_t sl, uint8_t sr, 
						uint8_t sb);
void send_line_pos(uint8_t pos);
void send_differences(uint8_t diff, uint8_t rot);
void send_reg_params(uint16_t p, uint16_t d);

