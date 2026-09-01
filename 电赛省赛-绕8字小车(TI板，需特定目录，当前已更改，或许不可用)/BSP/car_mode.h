#ifndef __CAR_MODE_H
#define __CAR_MODE_H 	



void wheel1_move(int speed);
void wheel2_move(int speed);

void car_mode1(void);
void car_mode2(void);
void car_mode3(void);
void car_mode4(void);
void car_mode5(void);

void buzzer_on(void);
void buzzer_off(void);
void led_on(void);
void led_off(void);

void car_go_straight(float target_speed,float target_angle);
void car_go_circle(float target_speed);
void car_go_turn(float target_angle);
void stop_car(void);
void angle_init(void);
void test(void);
void car_go_circle_x(int target_circle_speed,int pwm1_max,int pwm2_max);

void deal_data();
void car_move(int car_move_state,float car_target_angle,float car_target_speed,int car_circle_speed,int wheel1_pwm,int wheel2_pwm,int car_on_line_state);



#endif



