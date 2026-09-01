/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "board.h"
#include "stdio.h"
#include "oled.h"
#include "ti_msp_dl_config.h"
#include "encoder.h"
#include "icm20602.h"
#include "math.h"
#include "car_mode.h"
#include "xunji.h"
#include "pid.h"




extern int16_t icm_gyro_x,icm_gyro_y,icm_gyro_z;
extern int16_t icm_acc_x,icm_acc_y,icm_acc_z;

extern quater_param_t Q_info;   //四元数
extern euler_param_t eulerAngle;//欧拉角
extern icm_param_t icm_data;    //六轴数值
extern gyro_param_t GyroOffset; //陀螺仪校准




int on_line_state;
extern int move_state;
extern int final_state;
extern int buzzer_led_state;
float target_speed;
float target_angle;
float target_turn_angle;

float target_circle_speed;


int target_circle_speed_x;
int pwm1_max_circle;
int pwm2_max_circle; 




int count=0;
int key_num;
int key_count=0;
int car_move_mode;


#define PI 3.1415926
#define turn_angle_target   45


void get_data(void);
void show_angle(int yaw);
void key_select(void);
void show_data(void);
void key_scan(void);



int main(void)
{
	//开发板初始化
	
	
	SYSCFG_DL_init();
	board_init();
        
    OLED_Init();     //初始化OLED
    OLED_Clear();
	
	icm20602_init_spi();
	//gyroOffsetInit();	
	
	
	
	//清除定时器中断标志
    NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);
    //使能定时器中断
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);

//  test();
	   
//	car_mode3(); 
	   
	   

    while(1) 
    {
		
		while(1)
		{
		   key_scan();
		   key_select();		
		   show_data();
			if(key_num==2)
			{
				key_num=0;
				OLED_ShowString(0,48,(uint8_t *)"is ok",16,1);
				OLED_Refresh();
                break;				
			}
				
		}
		if(car_move_mode==1)
		{
			 car_mode1(); 
		}
		else if(car_move_mode==2)
		{
			 car_mode2();
		
		}
		else if(car_move_mode==3)
		{
		     car_mode3();
		}
		else if(car_move_mode==4)
		{
		     car_mode4();
		}
		else if(car_move_mode==5)
		{
			car_mode5();		
		}
				
    }
		
}




//串口中断
void UART_0_INST_IRQHandler(void)
{
    //如果产生了串口中断
	
	uint8_t uart_data;
    switch(DL_UART_getPendingInterrupt(UART_0_INST))
    {		
        case DL_UART_IIDX_RX://如果是接收中断
            //接发送过来的数据保存在变量中
        //uart_data = DL_UART_Main_receiveData(UART_0_INST);
		
            break;

        default://其他的串口中断
            break;
    }
	
	
}
//
int count_led_buzzer;
int speed_count;
int circle_speed_count;
void TIMER_0_INST_IRQHandler(void)
{
	float temp;
    //如果产生了定时器中断
	int count;
	
    switch( DL_TimerG_getPendingInterrupt(TIMER_0_INST))
    {
        case DL_TIMER_IIDX_ZERO://如果是0溢出中断
		
		
		
		get_data();	
		//走直线
		if(move_state==1)
		{
			speed_count++;
			if(speed_count>=250&&speed_count<=380)
			{
				
			  car_go_straight(target_speed,target_angle);
			}
			else if(speed_count>380)
			{
				
				if(target_angle>0)
				{
					target_angle-=40;			
				}
				else if(target_angle<0)
				{
					target_angle+=40;			
				}
				
				car_go_straight(target_speed,target_angle);
				
				 //car_go_turn(target_angle);
				
			}
			else 
			{
				car_go_straight(90,target_angle);		
			}
			
			
			
			if(on_line_state==0)
			{
			   if(on_line()==1)
			   {
				speed_count=0;
				final_state=1;
			   }
		    }
			else if(on_line_state==1)
			{
				if(on_line_x()==1)
			   {
				speed_count=0;  
				final_state=1;
			   }
			}
			else if(on_line_state==2)
			{
			   if(on_line_y()==1)
			   {
				   
				speed_count=0;  
				final_state=1;
			   }
			   
			}
			
		}
		//圆弧循迹
		else if(move_state==2)
		{
			
			car_go_circle(target_circle_speed);
			
			if(on_line()==0)
			{
				final_state=1;
			}
			
		}	
		//转弯
		else if(move_state==3)
		{
			
			car_go_turn(target_turn_angle);
			
			if(fabs(eulerAngle.yaw-target_turn_angle)<3.0)
			{	
			    final_state=1;
			}
		
		}

		else if(move_state==4)
		{
			circle_speed_count++;
			if(circle_speed_count>=240)//240
			{
				car_go_circle_x(target_circle_speed_x-20,pwm1_max_circle-20,pwm2_max_circle-20);
			}
			else 
			{
			    car_go_circle_x(target_circle_speed_x,pwm1_max_circle,pwm2_max_circle);
				
			}
			
		     if(on_line()==0)
			{
				circle_speed_count=0;
				final_state=1;
			}	
		
		}
		
		if(buzzer_led_state==1)
		{
			count_led_buzzer++;
			if(count_led_buzzer>=80)
			{
				led_off();
				buzzer_off();	
				count_led_buzzer=0;
				buzzer_led_state=0;
			}
			
		}
		
		
         break;

        default://其他的定时器中断
            break;
    }
}



void show_angle(int yaw)
{
	if(yaw>=0)
	{	
	   OLED_ShowString(0,32,(uint8_t *)"+",16,1);//6*8 “ABC”
	   OLED_ShowNum(8,32,yaw,4,16,1);
	}
	else if(yaw<0)
	{
	   yaw=-yaw;
	   OLED_ShowString(0,32,(uint8_t *)"-",16,1);//6*8 “ABC”
	   OLED_ShowNum(8,32,yaw,4,16,1);	
	}
    OLED_Refresh();

}

void get_data(void)
{
	
	int yaw;
    get_icm20602_accdata_spi();
	get_icm20602_gyro_spi();
	icmGetValues();
		
    icmAHRSupdate(&icm_data);
		
    float q0 = Q_info.q0;
    float q1 = Q_info.q1;
    float q2 = Q_info.q2;
    float q3 = Q_info.q3;
    // atan2返回输入坐标点与坐标原点连线与X轴正方形夹角的弧度值
    eulerAngle.pitch = asin(2 * q0 * q2 - 2 * q1 * q3) * 180 / PI; 
    eulerAngle.roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 180 / PI; 
    eulerAngle.yaw = atan2(2 * q1 * q2 + 2 * q0 * q3, -2 * q2 * q2 - 2 * q3 * q3 + 1) * 180 / PI;
	
//	printf("%f,%d,%d,%d,%d,%d,%d\n",eulerAngle.yaw,icm_gyro_x,icm_gyro_y,icm_gyro_z,icm_acc_x,icm_acc_y,icm_acc_z);
	
}

void key_select(void)
{
	if(key_num==1)
	{
		key_num=0;
		key_count++;
		
		if(key_count>5)
		{
			key_count=0;		
		}
	}
	car_move_mode=key_count;
	
	OLED_ShowString(0,48,(uint8_t *)"mode:",16,1);
	OLED_ShowNum(48,48,car_move_mode,4,16,1);
	OLED_Refresh();

}

void show_data(void)
{

	    int x1,x2,x3,x4,x5,x6,x7,x8;
	
	    track_deal_four(&x1,&x2,&x3,&x4,&x5,&x6,&x7,&x8);
	
		OLED_ShowNum(0,0,x1,1,16,1);
		OLED_ShowNum(16,0,x2,1,16,1);
		OLED_ShowNum(32,0,x3,1,16,1);
		OLED_ShowNum(48,0,x4,1,16,1);
		OLED_ShowNum(64,0,x5,1,16,1);
		OLED_ShowNum(80,0,x6,1,16,1);
		OLED_ShowNum(96,0,x7,1,16,1);
		OLED_ShowNum(112,0,x8,1,16,1);
	
	    show_angle((int)eulerAngle.yaw);	

}


void key_scan(void)
{
	static int key_up=1;
	int key1=DL_GPIO_readPins(KEY_PORT, KEY_PIN_23_PIN);
	int key2=DL_GPIO_readPins(KEY_PORT, KEY_PIN_24_PIN);
	if(key_up==1&&(key1==0||key2==0))
	{
		key_up=0;
		if(key1==0)
		{
			key_num=1;
		}
		if(key2==0)
		{
			key_num=2;
		}
	}
	else if(key1>0&&key2>0)
	{
		key_up=1;	
	}

}
	
	
	








