#include "ti_msp_dl_config.h"
#include "pid.h"
#include "xunji.h"
#include "icm20602.h"
#include "board.h"
#include "car_mode.h"



//小车以固定速度前进
extern quater_param_t Q_info;   //四元数
extern euler_param_t eulerAngle;//欧拉角
extern icm_param_t icm_data;    //六轴数值
extern gyro_param_t GyroOffset; //陀螺仪校准


int final_state;
int move_state;
int buzzer_led_state;

extern float target_speed;
extern float target_angle;
float angle_error;

extern float target_turn_angle;

extern float target_circle_speed;

extern int target_circle_speed_x;
extern int pwm1_max_circle;
extern int pwm2_max_circle;

extern int on_line_state;


#define   angle_kp            4.5
#define   circle_kp           0.8
#define   pwm_max             99
#define   pwm_min             70



#define   pwm_turn_max        70
#define   pwm_turn_min        60



void buzzer_on(void)
{
	DL_GPIO_setPins(buzzer_PORT,buzzer_PIN_22_PIN);
	
}

void buzzer_off(void)
{
	DL_GPIO_clearPins(buzzer_PORT,buzzer_PIN_22_PIN);
}

void led_on(void)
{	
	DL_GPIO_clearPins(LED1_PORT,LED1_PIN_21_PIN);
}

void led_off(void)
{
	DL_GPIO_setPins(LED1_PORT,LED1_PIN_21_PIN);
}



//PA8 PA9 
void wheel1_move(int speed)
{
	
	if(speed>=0)
	{
		DL_TimerG_setCaptureCompareValue(wheel_INST,speed,GPIO_wheel_C0_IDX);
		DL_TimerG_setCaptureCompareValue(wheel_INST,-1,GPIO_wheel_C1_IDX);	
	}
	else 
	{
		DL_TimerG_setCaptureCompareValue(wheel_INST,-1,GPIO_wheel_C0_IDX);
		DL_TimerG_setCaptureCompareValue(wheel_INST,-speed,GPIO_wheel_C1_IDX);
	}	
	
}



//PA7 PA12
void wheel2_move(int speed)
{
	
	if(speed>=0)
	{
		DL_TimerG_setCaptureCompareValue(wheel_INST,speed,GPIO_wheel_C2_IDX);
		DL_TimerG_setCaptureCompareValue(wheel_INST,-1,GPIO_wheel_C3_IDX );	
	}
	else 
	{
		DL_TimerG_setCaptureCompareValue(wheel_INST,-1,GPIO_wheel_C2_IDX);
		DL_TimerG_setCaptureCompareValue(wheel_INST,-speed,GPIO_wheel_C3_IDX );
	}	
	
}

void stop_car(void)
{
	DL_TimerG_setCaptureCompareValue(wheel_INST,99,GPIO_wheel_C0_IDX);
	DL_TimerG_setCaptureCompareValue(wheel_INST,99,GPIO_wheel_C1_IDX);
	DL_TimerG_setCaptureCompareValue(wheel_INST,99,GPIO_wheel_C2_IDX);
	DL_TimerG_setCaptureCompareValue(wheel_INST,99,GPIO_wheel_C3_IDX);
}




//小车走圆弧时
//固定速度
//外环速度比内环大
//循迹不采用角度传感器

//小车走直线
//在中断里面不断调用这个函数
//wheel1是左轮
//wheel2是右轮	
//pwm最多为99  速度偏差最多为20



//最低50  
void car_go_circle_x(int target_circle_speed,int pwm1_max,int pwm2_max)
{
	int error_x;
	int pwm1;
	int pwm2;
	
	error_x=car_track();
	
	pwm1=target_circle_speed-4.5*error_x;
	pwm2=target_circle_speed+4.5*error_x;
	
	
	if(pwm1<50)
	{
		pwm1=50;		
	}
	else if(pwm1>pwm1_max)
	{
		pwm1=pwm1_max;	
	}
	if(pwm2<50)
	{
		pwm2=50;	
	}
	else if(pwm2>pwm2_max)
	{
		pwm2=pwm2_max;	
	}
	
	wheel1_move(pwm1);
	wheel2_move(pwm2);
	
	
	
}




void car_go_straight(float target_speed,float target_angle)
{
	
	float current_angle;
	float error_angle;
	int pwm1;
	int pwm2;
	
	current_angle=eulerAngle.yaw;
	error_angle=current_angle-target_angle;
	
	if(error_angle>20)
	{
		error_angle=20;	
	}
	else if(error_angle<-20)
	{
		error_angle=-20;	
	}
	
	//如果左边往前走时角度增加
	
	pwm1=target_speed+error_angle*angle_kp;
	pwm2=target_speed-error_angle*angle_kp;
	
	
//对pwm进行限幅处理	
	if(pwm1<pwm_min)
	{
		pwm1=pwm_min;		
	}
	else if(pwm1>pwm_max)
	{
		pwm1=pwm_max;	
	}
	if(pwm2<pwm_min)
	{
		pwm2=pwm_min;	
	}
	else if(pwm2>pwm_max)
	{
		pwm2=pwm_max;	
	}
	wheel1_move(pwm1);
	wheel2_move(pwm2);
	
	
	
	
}

//走圆弧时要循迹
void car_go_circle(float target_speed)
{
	int error_x;
	int pwm1;
	int pwm2;
	
	error_x=car_track();
	
	
	pwm1=target_speed-circle_kp*error_x;
	pwm2=target_speed+circle_kp*error_x;
	
	
	
	if(pwm1<pwm_min)
	{
		pwm1=pwm_min;		
	}
	else if(pwm1>pwm_max)
	{
		pwm1=pwm_max;	
	}
	if(pwm2<pwm_min)
	{
		pwm2=pwm_min;	
	}
	else if(pwm2>pwm_max)
	{
		pwm2=pwm_max;	
	}
	
	wheel1_move(pwm1);
	wheel2_move(pwm2);
	
}

//转一定的角度
void car_go_turn(float target_angle) 
{
	
	float current_angle;
	float error_angle;
	int pwm1;
	int pwm2;
	
	
	current_angle=eulerAngle.yaw;
	error_angle=current_angle-target_angle;
	
	if(error_angle>20)
	{
		error_angle=20;	
	}
	else if(error_angle<-20)
	{
		error_angle=-20;	
	}
	pwm1=error_angle*angle_kp;
	pwm2=(-error_angle)*angle_kp;
	
	

	if(pwm1<pwm_turn_min&&pwm1>0)
	{
		pwm1=pwm_turn_min;		
	}
	else if(pwm1>pwm_turn_max)
	{
		pwm1=pwm_turn_max;	
	}
	if(pwm2<pwm_turn_min&&pwm2>0)
	{
		pwm2=pwm_turn_min;	
	}
	else if(pwm2>pwm_turn_max&&pwm2>0)
	{
		pwm2=pwm_turn_max;	
	}
	
	if(pwm1<0&&pwm1>-pwm_turn_min)
	{
		pwm1=-pwm_turn_min;
	}
	else if(pwm1<-pwm_turn_max)
	{
		pwm1=-pwm_turn_max;	
	}
	
	if(pwm2<0&&pwm2>-pwm_turn_min)
	{
		pwm2=-pwm_turn_min;
	}
	else if(pwm2<-pwm_turn_max)
	{
		pwm2=-pwm_turn_max;	
	}
		
	wheel1_move(pwm1);
	wheel2_move(pwm2);

}

void angle_init(void)
{
	Q_info.q0=1;
	Q_info.q1=0;
	Q_info.q2=0;
	Q_info.q3=0;
}
void show_data();


void test(void)
{	
//	
     car_move(1,-36,60,0,0,0,1);
	 
	 car_move(4,0,0,85,90,99,0);
	
	 stop_car();
	
	 delay_ms(50);
	
	 angle_init();
	
	 car_move(1,50,60,0,0,0,2);
	 
	 car_move(4,0,0,85,99,90,0);
		
	 stop_car();

	 delay_ms(50);
	
	 angle_init();
	 
//	 	 
	
	 car_move(1,-50,60,0,0,0,1);
	
	 car_move(4,0,0,85,90,99,0);
	
	 stop_car();
	
	 delay_ms(50);
	
	 angle_init();
	 
	 car_move(1,50,60,0,0,0,2);
	 
	 car_move(4,0,0,85,99,90,0);
	 	 
	 stop_car();

	 delay_ms(50);
	
	 angle_init();
	 
//	 
	 car_move(1,-50,60,0,0,0,1);
	
	 car_move(4,0,0,85,90,99,0);
	
	 stop_car();
	
	 delay_ms(50);
	
	 angle_init();
	 
	 car_move(1,50,60,0,0,0,2);
	 
	 car_move(4,0,0,85,99,90,0);
	 	 
	 stop_car();

	 delay_ms(50);
	
	 angle_init();
	 
//	 
	 car_move(1,-50,60,0,0,0,1);
	
	 car_move(4,0,0,85,90,99,0);
	
	 stop_car();
	
	 delay_ms(50);
	
	 angle_init();
	 
	 car_move(1,50,60,0,0,0,2);
	 
	 car_move(4,0,0,85,99,90,0);
	 	 
	 stop_car();
//
	
	 

}





void car_mode1(void)
{
	move_state=1;
	target_angle=0;
	target_speed=90;
	
	while(1)
	{
		show_data();
		if(final_state==1)
		{
			move_state=0;
			final_state=0;
			//进行声光提示
			buzzer_on();
		    led_on();
			buzzer_led_state=1;
			break;		    
		}
		
	}
	
	stop_car();

} 

void car_mode2(void)
{
//	on_line_state=0;
//	move_state=1;
//	target_angle=0;
//	target_speed=90;
//		
//	while(1)
//	{
//		
//		show_data();
//		if(final_state==1)
//		{
//			move_state=0;
//			final_state=0;
//			//进行声光提示
//			buzzer_on();
//		    led_on();
//			buzzer_led_state=1;
//			break;		    
//		}
//	}
//	
//	
//	target_circle_speed=80;
//	move_state=2;
//	
//	while(1)
//	{
//		
//		show_data();
//		if(final_state==1)
//		{
//			move_state=0;
//			final_state=0;
//			buzzer_on();
//		    led_on();
//		    buzzer_led_state=1;
//			break;	
//			
//		}
//	
//	}
//	
//	
//	
//	if(eulerAngle.yaw<0)
//	{
//		angle_error=-180-eulerAngle.yaw;
//	}
//	if(eulerAngle.yaw>0)
//	{
//		angle_error=180-eulerAngle.yaw;	
//	}
//	
//	angle_init();
//	
//	target_angle=angle_error;
//	target_speed=90;	
//	move_state=1;

//	while(1)
//	{
//		
//		show_data();
//		if(final_state==1)
//		{
//			move_state=0;
//			final_state=0;
//			buzzer_on();
//		    led_on();
//		    buzzer_led_state=1;
//			break;	
//			
//		}
//	}
//	
//	target_circle_speed=80;
//	move_state=2;
//	while(1)
//	{
//		
//		show_data();
//		if(final_state==1)
//		{
//			move_state=0;
//			final_state=0;
//			buzzer_on();
//		    led_on();
//		    buzzer_led_state=1;
//			
//			break;		
//		}
//	
//	}
//	stop_car();


    car_move(1,0,90,0,0,0,2);

	target_circle_speed=80;
	move_state=2;
	
	while(1)
	{
		
		show_data();
		if(final_state==1)
		{
			move_state=0;
			final_state=0;
			buzzer_on();
		    led_on();
		    buzzer_led_state=1;
			break;	
			
		}
	
	}


	if(eulerAngle.yaw<0)
	{
		angle_error=-180-eulerAngle.yaw;
	}
	if(eulerAngle.yaw>0)
	{
		angle_error=180-eulerAngle.yaw;	
	}
	
	angle_init();
	
	
car_move(1,angle_error,90,0,0,0,2);		
	
	target_circle_speed=80;
	move_state=2;
	while(1)
	{
		
		show_data();
		if(final_state==1)
		{
			move_state=0;
			final_state=0;
			buzzer_on();
		    led_on();
		    buzzer_led_state=1;
			
			break;		
		}
	
	}
	stop_car();
	
}


//40度




//对左轮右轮进行限速处理  在循迹时增加限速操作

void car_move(int car_move_state,float car_target_angle,float car_target_speed,int car_circle_speed,int wheel1_pwm,int wheel2_pwm,int car_on_line_state)
{
	
	if(car_move_state==1)
	{
		on_line_state=car_on_line_state;
		move_state=1;
		target_angle=car_target_angle;
		target_speed=car_target_speed;
		
	 while(1)
	{		
		show_data();
		if(final_state==1)
		{
			move_state=0;
			final_state=0;
			//进行声光提示
			buzzer_on();
		    led_on();
			buzzer_led_state=1;
		    stop_car();
	        delay_ms(50);
			break;		    
		}
		
	}
	
	
	}
	
	
	else if(car_move_state==4)
	{
		move_state=4;
		target_circle_speed_x=car_circle_speed;
		pwm1_max_circle=wheel1_pwm;
		pwm2_max_circle=wheel2_pwm;

		
	while(1)
	{
		
		show_data();
		if(final_state==1)
		{
			move_state=0;
			final_state=0;
			buzzer_on();
		    led_on();
		    buzzer_led_state=1;
			break;	
			
		}
	
	}
	
	
	}

}


void deal_data()
{
	if(eulerAngle.yaw<0)
	{
		target_turn_angle=eulerAngle.yaw+25;
	}
	else if(eulerAngle.yaw>0)
	{
		target_turn_angle=eulerAngle.yaw-25;
	}
}

float yaw;

void car_turn(void)
{
	deal_data();
	move_state=3;
	
	while(1)
	{
		show_data();
		if(final_state==1)
		{
			move_state=0;
			final_state=0;
			break;	
			
		}
	
	}
	
}	



void car_mode3(void)
{
		
	 car_move(1,-36,60,0,0,0,1);
	 
	 car_move(4,0,0,80,90,99,0);
	
	 stop_car();
	
	 delay_ms(60);
	
	 angle_init();
	
	 car_move(1,53,60,0,0,0,2);
	 
	 car_move(4,0,0,80,99,90,0);
		
	 stop_car();
	
}


void car_mode4(void)
{
	 car_move(1,-36,60,0,0,0,1);
	 
	 car_move(4,0,0,80,90,99,0);
	
	 stop_car();
	
	 delay_ms(60);
	
	 angle_init();
	
	 car_move(1,53,60,0,0,0,2);
	 
	 car_move(4,0,0,80,99,90,0);
		
	 stop_car();

	 delay_ms(60);
	
	 angle_init();
	 
//	 	 
	
	 car_move(1,-50,60,0,0,0,1);
	
	 car_move(4,0,0,80,90,99,0);
	
	 stop_car();
	
	 delay_ms(60);
	
	 angle_init();
	 
	 car_move(1,53,60,0,0,0,2);
	 
	 car_move(4,0,0,80,99,90,0);
	 	 
	 stop_car();

	 delay_ms(60);
	
	 angle_init();
	 
//	 
	 car_move(1,-50,60,0,0,0,1);
	
	 car_move(4,0,0,80,90,99,0);
	
	 stop_car();
	
	 delay_ms(60);
	
	 angle_init();
	 
	 car_move(1,53,60,0,0,0,2);
	 
	 car_move(4,0,0,80,99,90,0);
	 	 
	 stop_car();

	 delay_ms(60);
	
	 angle_init();
	 
//	 
	 car_move(1,-50,60,0,0,0,1);
	
	 car_move(4,0,0,80,90,99,0);
	
	 stop_car();
	
	 delay_ms(60);
	
	 angle_init();
	 
	 car_move(1,53,60,0,0,0,2);
	 
	 car_move(4,0,0,80,99,90,0);
	 	 
	 stop_car();
//
	 
	 
}
void car_mode5(void)
{
	 car_move(1,-36,60,0,0,0,1);
	 
	 car_move(4,0,0,80,90,99,0);
	
	 stop_car();
	
	 delay_ms(60);
	
	 angle_init();
	
	 car_move(1,53,60,0,0,0,2);
	 
	 car_move(4,0,0,80,99,90,0);
		
	 stop_car();

	 delay_ms(60);
	
	 angle_init();
	
	
	while(1)
	{
	 car_move(1,-50,60,0,0,0,1);
	
	 car_move(4,0,0,80,90,99,0);
	
	 stop_car();
	
	 delay_ms(60);
	
	 angle_init();
	 
	 car_move(1,53,60,0,0,0,2);
	 
	 car_move(4,0,0,80,99,90,0);
	 	 
	 stop_car();
		
	 delay_ms(60);
	
	 angle_init();
		
		
		
		
	}
	
}











