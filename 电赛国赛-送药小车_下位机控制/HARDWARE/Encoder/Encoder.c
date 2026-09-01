#include <stm32f4xx.h>
#include "stm32f4xx_tim.h"
#include "Encoder.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <Encoder.h>
#include "lcd.h"

void TIM3_Encoder_Init() 
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE); 

//定时器设置-------------------------------------------------------------	
  TIM_TimeBaseInitStructure.TIM_Period = 65535; 	//重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=0x0;  //预分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; //时钟分割
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//初始化TIM3

//编码器模式设置--------------------------------------------------------------			  		

	
	TIM_ICStructInit(&TIM_ICInitStructure); 
  TIM_ICInitStructure.TIM_ICFilter = 0xF;//滤波器值
	TIM_ICInitStructure.TIM_Channel=TIM_Channel_1;
  TIM_ICInit(TIM3, &TIM_ICInitStructure);
	
	TIM_ICInitStructure.TIM_ICFilter = 0xF;//滤波器值
	TIM_ICInitStructure.TIM_Channel=TIM_Channel_2;
  TIM_ICInit(TIM3, &TIM_ICInitStructure);

	TIM_EncoderInterfaceConfig(TIM3,TIM_EncoderMode_TI12,TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);//计数模式3
  
 //Reset counter-----------------------------------------------
  TIM_SetCounter(TIM3,0); //TIM3->CNT=0
  TIM_Cmd(TIM3, ENABLE); 
}


void TIM4_Encoder_Init() 
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE); 

//定时器设置-------------------------------------------------------------	
  TIM_TimeBaseInitStructure.TIM_Period = 65535; 	//重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=0x0;  //预分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; //时钟分割
	
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);//初始化TIM2

//编码器模式设置--------------------------------------------------------------			  		

	
	TIM_ICStructInit(&TIM_ICInitStructure); 
  TIM_ICInitStructure.TIM_ICFilter = 0xF;//滤波器值
	TIM_ICInitStructure.TIM_Channel=TIM_Channel_1;
  TIM_ICInit(TIM4, &TIM_ICInitStructure);
	
	TIM_ICInitStructure.TIM_ICFilter = 0xF;//滤波器值
	TIM_ICInitStructure.TIM_Channel=TIM_Channel_2;
  TIM_ICInit(TIM4, &TIM_ICInitStructure);

	TIM_EncoderInterfaceConfig(TIM4,TIM_EncoderMode_TI12,TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);//计数模式3
  
 //Reset counter-----------------------------------------------
  TIM_SetCounter(TIM4,0); //TIM3->CNT=0
  TIM_Cmd(TIM4, ENABLE); 
}




/*相关GPIO初始化*/
void Related_GPIO_Pin_Init()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOE,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;

	//TIM4
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_TIM4);


	//TIM3
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource4,GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource5,GPIO_AF_TIM3);

}
/*----------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************************************************************/
/*----------------------------------------------------------------------------------------------------------------------------*/
/*
*车辆参数
*单位：cm
*/
typedef struct
{
	double car_wheel_d;/*车轮的直径*/
	double car_width;/*车的宽度*/
	double car_length;/*车的长度*/
	double one_cirle_encouder;/*轮子转一圈的编码器计数*/
	double reduction_ratio;/*电机的减速比*/
}car_arg;

car_arg My_car=
{
	.car_wheel_d=8.0,
	.car_width=1.0,/*当前未使用*/
	.car_length=1.0,/*当前未使用*/
	.one_cirle_encouder=44.0,/*一圈44个计数*/
	.reduction_ratio=90.0/*减速比1:131*/
};
/*----------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************************************************************/
/*----------------------------------------------------------------------------------------------------------------------------*/

/*这个PID结构体的定义我已经放到Encoder.h里了*/

/*----------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************************************************************/
/*----------------------------------------------------------------------------------------------------------------------------*/


/*到时候可能要更改轮子顺序*/
void Get_Encoder_Straight(TIM_TypeDef* TIMx,double*current_sum_straight,PID_InitTypeDef*My_PID)
{
	int16_t encouder_counter_pack=TIM_GetCounter(TIMx);
	
	TIM_SetCounter(TIMx,0);

	//LCD_ShowNum(100,100,encouder_counter_pack,4,16);
	/*调试LCD显示*/
//	if(encouder_counter_pack<0)
//		LCD_ShowNum(100,100,4,1,16);
//	else 
//		LCD_ShowNum(100,100,2,1,16);
//	LCD_ShowNum(1,1,(u32)abs(encouder_counter_pack),4,16);

	
		My_PID->Velocity_Actual_Val=(encouder_counter_pack/(My_car.one_cirle_encouder*My_car.reduction_ratio))*My_car.car_wheel_d*pi*25;//单位：cm/s
		*current_sum_straight+=((encouder_counter_pack/(My_car.one_cirle_encouder*My_car.reduction_ratio))*My_car.car_wheel_d*pi);//计算现在已经走的直线总距离
		My_PID->Location_Actual_Val=*current_sum_straight;
}
/*复位直线距离*/
void Reset_Straight_Length(double*current_sum_straigh)
{
	*current_sum_straigh=0.0;
}
/*----------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************************************************************/
/*----------------------------------------------------------------------------------------------------------------------------*/
void Get_Encoder_Turn(TIM_TypeDef* TIMx,double*current_sum_turn_spin,PID_InitTypeDef*My_PID)
{
	int16_t encouder_counter_pack=TIM_GetCounter(TIMx);
	TIM_SetCounter(TIMx,0);


		My_PID->Velocity_Actual_Val=(encouder_counter_pack/(My_car.one_cirle_encouder*My_car.reduction_ratio))*My_car.car_wheel_d*pi*25;//单位：cm/s
		*current_sum_turn_spin+=(encouder_counter_pack/(My_car.one_cirle_encouder*My_car.reduction_ratio))*My_car.car_wheel_d*pi;//计算现在已经走的直线总距离
		My_PID->Location_Actual_Val=*current_sum_turn_spin;	
}

void Reset_Turn_Spin_Length(double*current_sum_turn_spin)
{
	*current_sum_turn_spin=0.0;
}


