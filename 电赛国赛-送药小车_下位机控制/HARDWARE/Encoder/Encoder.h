#ifndef _ENCODER_H
#define _ENCODER_H
#include "sys.h"
#define pi 3.1415926
typedef struct
{
	double Location_Kp;
	double Location_Ki;
	double Location_Kd;
	double Location_Error;
	double Location_Last_Error;
	double Location_Integral;
	double Location_Target_Val;
	double Location_Actual_Val;
	double Location_Out;
	
	
	double Location_Out_Limit;
	double Location_Int_Limit;
	/*这里是我们的位置环*/

	
	double Velocity_Kp;
	double Velocity_Ki;
	double Velocity_Kd;
	double Velocity_Error;
	double Velocity_Last_Error;
	double Velocity_Integral;
	double Velocity_Target_Val;
	double Velocity_Actual_Val;
	double Velocity_Out;
	
	double Velocity_Int_Limit;
	double Velocity_Out_Limit;
	/*这里是我们的速度环*/
	
}PID_InitTypeDef;


typedef struct
{
	double Kp;
	double Ki;
	double Kd;
	double Error;
	double Last_Error;
	double Integral;
	double Target_Val;
	double Actual_Val;
	double Out;
	
	
	double Out_Limit;
	double Int_Limit;
}PID_Mini;

void TIM3_Encoder_Init(void); 
void TIM4_Encoder_Init(void);
void Related_GPIO_Pin_Init(void);
double Get_Encoder(void);
void Get_Encoder_Straight(TIM_TypeDef* TIMx,double*current_sum_straight,PID_InitTypeDef*My_PID);
void Reset_Straight_Length(double*current_sum_straight);
void Get_Encoder_Turn(TIM_TypeDef* TIMx,double*current_sum_turn_spin,PID_InitTypeDef*My_PID);
void Reset_Turn_Spin_Length(double*current_sum_turn_spin);


/*
*编码器引脚(总共四个编码器)
*PB4
*PB5
*PB6
*PB7
*/
#endif

