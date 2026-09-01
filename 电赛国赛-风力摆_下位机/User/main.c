#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "PID.h"
#include "Motor.h"
#include "Serial.h"
#include "nrf24l01.h"
#include "inv_mpu.h"
#include "stdio.h"
#include "MPU6050.h"
#include "Function.h"


//Pitch  俯仰角
//Roll   翻滚角
//Yaw    偏航角

extern float Pitch,Roll,Yaw;
extern float sum_angle[3]; 
extern float gyro_real[3];
float Pitch_temp,Roll_temp;
float gyro_real_temp[3];


uint8_t Data_tx[32];
uint8_t Data_rx[32];
//参数 模式 开始信号 停止信号 以及手柄控制角度的信号
extern uint8_t flag;
extern PID_t Pitch_angle;
extern PID_t Roll_angle; 

extern PID_t Pitch_angularSpeed;
extern PID_t Roll_angularSpeed;



extern int deal_flag;
 
 
 
void Recieve_And_Send(void)
{
		flag=NRF24L01_GetRxBuf((uint8_t*) Data_rx);
		if(flag==0)
		{
			Delay_ms(1);
			NRF24L01_SendBuf((uint8_t*)Data_tx);			
		}
		
}

 
 



int main(void)
{
	
	OLED_Init();	
	mpu_dmp_init();
    NRF24L01_Init();
    Motor_PWM_Init();  
	

//	test_self_angle();
//	test_self_angularSpeed();	
	//PID_TIM_Init();
	
//		TIM_SetCompare1(TIM3,100);//右边
//		TIM_SetCompare2(TIM3,100);//前面
//		TIM_SetCompare3(TIM3,100);//后面
//     	TIM_SetCompare4(TIM3,100);//左边
	
//	 start_swing();
	
//	 Pitch_angle.OutMax=30;
//	 Pitch_angle.OutMin=-30;
//	 Pitch_angle.ErrIntMax=500;
//	 Pitch_angle.Kp=0.5;
//	 Pitch_angle.Ki=0;
//	 Pitch_angle.Kd=1.0;
	 
//   画直线的PID	 
	 Pitch_angularSpeed.Kp=2;
	 Pitch_angularSpeed.Ki=0.05;
	 Pitch_angularSpeed.Kd=0.01;
	 Pitch_angularSpeed.OutMax=70;
	 Pitch_angularSpeed.OutMin=-70;
	 Pitch_angularSpeed.ErrIntMax=300;
	 

//画圆的PID




	//start_swing(20);
	
	while (1)
	{
		
//		if(deal_flag==1)
//		{
//			deal_flag=0;			
//			draw_line(40,0);
//		}
		
		mpu_dmp_get_data(&Pitch,&Roll,&Yaw);
		
		//MPU6050_DMP_Get_Data(&Pitch,&Roll,&Yaw);
		
		send_tx();
		
		NRF24L01_SendBuf((uint8_t*)Data_tx);
		
		
//		MPU6050_DMP_Get_Data(&Pitch,&Roll,&Yaw);//获取当前的角度值
//			
//		Get_Gyro();
//		Pitch=Pitch-sum_angle[0];
//		Roll=Roll-sum_angle[1];
//		
//		
//		
//		OLED_ShowFloatNum(0, 0,Pitch, 3, 2, OLED_6X8);
//		OLED_ShowFloatNum(0,10,Roll , 3, 2, OLED_6X8);
////		OLED_ShowFloatNum(0,20,Yaw  , 3, 2, OLED_6X8);		
////		
////				
//		OLED_ShowFloatNum(60,  0,gyro_real[0], 3, 2, OLED_6X8); //俯仰角  上偏为正
//		OLED_ShowFloatNum(60, 10,gyro_real[1], 3, 2, OLED_6X8);//翻滚角  左偏为正		
////		OLED_ShowFloatNum(60, 20,gyro_real[2], 3, 2, OLED_6X8);//偏航角  
////		
////		
////		
////	   //OLED_ShowNum(0,30,i,6, OLED_6X8);
////		
////		
// //       OLED_Update();
////		
////		
//////	   NRF24L01_SendBuf((uint8_t*) Data_tx);
////		   flag=NRF24L01_GetRxBuf((uint8_t*)Data_rx);



//////		if(Data_rx[5]==0)
//////		{
//////			TIM_SetCompare2(TIM3,100);			
//////		}
//////		else if(Data_rx[5]==1)
//////		{
//////			TIM_SetCompare2(TIM3,0);			
//////		}
////		
////		

////        Recieve_And_Send();
//        send_tx();

//        NRF24L01_SendBuf((uint8_t*)Data_tx);



////		OLED_ShowSignedNum(0, 40, Data_rx[2] , 2, OLED_6X8);
////		OLED_ShowSignedNum(0, 50, Data_rx[3] , 2, OLED_6X8);
////		OLED_ShowSignedNum(0, 60, Data_rx[4] , 2, OLED_6X8);


//     	OLED_Update();		
		
	}

	
	
}









