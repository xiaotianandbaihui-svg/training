#include <stm32f4xx.h>
#include "pid.h"
#include "Encoder.h"
#include "pwm.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lcd.h"
#include <string.h>
struct STATE_PACK_1
{
	uint8_t GO_Straight;/*走直线标志*/
	uint8_t Turn_90;/*转弯90度*/
	uint8_t Turn_180;/*原地掉头*/
	uint8_t Turn_Success;/*转弯或者掉头成功*/
	uint8_t Arrive_Target;/*到达目标位置*/
	uint8_t Spin_Success;/*掉头成功*/
};



/*元操作标志位包*/
struct STATE_PACK_1	state_pack=
{
	 .GO_Straight=0,/*走直线标志*/
	 .Turn_90=0,/*转弯90度*/
	 .Turn_180=0,/*原地掉头*/
	 .Turn_Success=0,/*转弯或者掉头成功*/
	 .Arrive_Target=0,/*到达目标位置*/
	 .Spin_Success=0
};


/*----------------------------------------------------------------------------*/
/*40ms进一次中断*/
/*这边默认是不进行PID的*/
void TIM_PID_Others_Init()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_InternalClockConfig(TIM2);
	
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period=40000-1;
	TIM_TimeBaseInitStructure.TIM_Prescaler=84-1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
	
	
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x02;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM2,DISABLE);
}

void Shut_Down_PID()
{
	TIM_Cmd(TIM2,DISABLE);//关闭定时器，相当于关闭PID
}

void Restart_PID()
{
	TIM_Cmd(TIM2,ENABLE);
}



void LocationRing_PID(PID_InitTypeDef*My_PID);
void VelocityRing_PID(PID_InitTypeDef*My_PID);
void Xunji_Mini_PID(PID_Mini*My_PID);
PID_InitTypeDef My_PID_1;
PID_InitTypeDef My_PID_2;
PID_Mini XunjiPID;
double wheel_have_role[2]={0};//轮子已经滚过的距离
int16_t temp5=0;
extern u8 Res;

typedef struct 
{
	 uint8_t start;/*开始启动*/
	uint8_t processing;/*小车在路上*/
	uint8_t arrive_target;/*小车到达药房*/
	uint8_t re_start;/*小车返回开始*/
	uint8_t arrive_start;/*小车到达起点*/
}state_pack_start_end;

extern state_pack_start_end state_pack_2;


void TIM2_IRQHandler()
{
	/*PID的目标值由外部设置------My_PID.Location_Target_Val------来实现*/
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET)
	{
//		int16_t temp[4]={300,300,300,300};
		
		
		
		int16_t temp[4] = {0,0,0,0};/*依据输出，设置电机*/
		if (state_pack.GO_Straight == 1)
		{
			Get_Encoder_Straight(TIM3,&wheel_have_role[0],&My_PID_1);/*获取当前已经走过的直线距离，并且获取当前实际速度*/
			LocationRing_PID(&My_PID_1);/*进行位置环PID，得到速度环输入*/

			/*调试用*/
			/*测试最大速度为50cm/s*/
		

			VelocityRing_PID(&My_PID_1);/*进行速度环PID*/
			temp[0] = My_PID_1.Velocity_Out;
			temp[2] = My_PID_1.Velocity_Out;
			
			/*左半边的轮子的PID*/
			
			Get_Encoder_Straight(TIM4,&wheel_have_role[1],&My_PID_2);/*获取当前已经走过的直线距离，并且获取当前实际速度*/
			LocationRing_PID(&My_PID_2);/*进行位置环PID，得到速度环输入*/

			/*调试用*/
			/*测试最大速度为50cm/s*/
	
		
			VelocityRing_PID(&My_PID_2);/*进行速度环PID*/
			temp[1] = My_PID_2.Velocity_Out;
			temp[3] = My_PID_2.Velocity_Out;
			/*右半边的轮子的PID*/
			
			
				
			/*循迹PID,原始版本*/
			if(My_PID_1.Location_Error>=15&&My_PID_2.Location_Error>=15)
		//if(1)
			{
				XunjiPID.Target_Val=54;
				XunjiPID.Actual_Val=Res;
				Xunji_Mini_PID(&XunjiPID);
				temp[1]-=XunjiPID.Out;
				temp[0]+=XunjiPID.Out;
			}
			
		}
		else if (state_pack.Turn_90 == 1 || state_pack.Turn_180 == 1)
		{			
			
			Get_Encoder_Turn(TIM3,&wheel_have_role[0],&My_PID_1);
			LocationRing_PID(&My_PID_1);/*进行位置环PID，得到速度环输入*/
			

			VelocityRing_PID(&My_PID_1);/*进行速度环PID*/
			temp[0] = My_PID_1.Velocity_Out;
			temp[2] = My_PID_1.Velocity_Out;

						
			Get_Encoder_Turn(TIM4,&wheel_have_role[1],&My_PID_2);
			
			
			LocationRing_PID(&My_PID_2);/*进行位置环PID，得到速度环输入*/
			

			VelocityRing_PID(&My_PID_2);/*进行速度环PID*/
			temp[1] = My_PID_2.Velocity_Out;
			temp[3] = My_PID_2.Velocity_Out;

			
		}

	//	printf("%f,%f,%f,%f,%f,%f\n", My_PID_1.Location_Target_Val, My_PID_1.Location_Actual_Val,My_PID_1.Velocity_Out,My_PID_2.Location_Target_Val, My_PID_2.Location_Actual_Val,My_PID_2.Velocity_Out);
	//	printf("%f,%f,%f,%f,%f,%f\n", My_PID_1.Velocity_Target_Val, My_PID_1.Velocity_Actual_Val,My_PID_1.Velocity_Out,My_PID_2.Velocity_Target_Val, My_PID_2.Velocity_Actual_Val,My_PID_2.Velocity_Out);

		/*接下来，就要判断是否到位，理论上是一个阈值，来解除标志位，这个函数的实现等等再说*/
		/*--------------------------------------------------------------------------------*/
		



		/*与目标距离差0.1厘米以内就可以认为成功到达了,可以看到我这里只是以一个轮子作为标定。因为我认为只要更跟随的好，车要么是直行的，要么是圆圈的*/
		if (fabs(My_PID_1.Location_Error) < 0.5 && fabs(My_PID_2.Location_Error) < 0.5)
		//if (0)		
		{
			if (state_pack.GO_Straight == 1)
			{
				state_pack.GO_Straight = 0;
				state_pack.Arrive_Target = 1;
				for (int i = 0; i < 4; i++)
					temp[i] = 0;
				memset(wheel_have_role, 0, sizeof(double) * 2);/*成功之后，就要把轮子走的距离这些的全部清零*/
				My_PID_1.Location_Integral = 0;//隔离积分项影响
					My_PID_2.Location_Integral=0;
					My_PID_1.Velocity_Integral=0;
					My_PID_2.Velocity_Integral=0;
					XunjiPID.Integral=0;/*直线循迹的积分项，也必须消除*/
			}
			else if (state_pack.Turn_90 == 1)
			{
				state_pack.Turn_90 = 0;
				state_pack.Turn_Success = 1;
				for (int i = 0; i < 4; i++)
					temp[i] = 0;
				memset(wheel_have_role, 0, sizeof(double) * 2);/*成功之后，就要把轮子走的距离这些的全部清零*/
				My_PID_1.Location_Integral = 0;//隔离积分项影响
					My_PID_2.Location_Integral=0;
					My_PID_1.Velocity_Integral=0;
					My_PID_2.Velocity_Integral=0;
			}
			else if (state_pack.Turn_180 == 1)
			{
				state_pack.Turn_180 = 0;
				state_pack.Spin_Success = 1;
				for (int i = 0; i < 4; i++)
					temp[i] = 0;
				memset(wheel_have_role, 0, sizeof(double) * 2);/*成功之后，就要把轮子走的距离这些的全部清零*/
				My_PID_1.Location_Integral = 0;//隔离积分项影响
					My_PID_2.Location_Integral=0;
					My_PID_1.Velocity_Integral=0;
					My_PID_2.Velocity_Integral=0;
			}
		}
/*--------------------------------------------------------------------------------*/

		Foward_Back(temp);


			//if(My_PID_1.Location_Error>=15&&My_PID_2.Location_Error>=15)
//			if(1)
//			{
//				XunjiPID.Target_Val=54;
//				XunjiPID.Actual_Val=Res;
//				Xunji_Mini_PID(&XunjiPID);
//				temp[1]-=XunjiPID.Out;
//				temp[0]+=XunjiPID.Out;
//				LCD_ShowNum(1,1,abs(temp[0]),4,16);
//				LCD_ShowNum(1,20,abs(temp[1]),4,16);	
//				LCD_ShowNum(1,40,Res,4,16);			
//			}
//			Foward_Back(temp);



		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	}
}
/*-----------------------------------------------------------------------------*/

	



/*PID部分*/
/*-----------------------------------------------------------------------------*/
#define Location_Outt_Limit 45.0 /*位置输出限幅*/
#define Velocity_Outt_Limit 1000.0	/*速度输出限幅*/
#define Location_Intt_Limit 45.0	/*位置积分限幅*/
#define Velocity_Intt_Limit 1000.0	/*速度积分限幅*/
/*这个由oc*/
/*左边轮子的*/
PID_InitTypeDef My_PID_1=
{
	. Location_Kp=6,
	. Location_Ki=0.02,
	. Location_Kd=0.8,

	. Location_Error=0.0,
	. Location_Last_Error=0.0,
	. Location_Integral=0.0,
	. Location_Target_Val=0.0,
	. Location_Actual_Val=0.0,
	. Location_Out=0.0,
	
	
	.Location_Out_Limit=Location_Outt_Limit,
	.Location_Int_Limit=Location_Intt_Limit,
	/*这里是我们的位置环*/

	. Velocity_Kp=27.5,
	. Velocity_Ki=5,
	. Velocity_Kd=0.8,

	. Velocity_Error=0.0,
	. Velocity_Last_Error=0.0,
	. Velocity_Integral=0.0,
	. Velocity_Target_Val=0.0,
	. Velocity_Actual_Val=0.0,
	. Velocity_Out=0.0,
	
	.Velocity_Out_Limit=Velocity_Outt_Limit,
	.Velocity_Int_Limit=Velocity_Intt_Limit
	/*这里是我们的速度环*/
};

/*右边轮子的*/
PID_InitTypeDef My_PID_2=
{
	. Location_Kp=6,
	. Location_Ki=0.02,
	. Location_Kd=0.8,

	. Location_Error=0.0,
	. Location_Last_Error=0.0,
	. Location_Integral=0.0,
	. Location_Target_Val=0.0,
	. Location_Actual_Val=0.0,
	. Location_Out=0.0,
	

	.Location_Out_Limit=Location_Outt_Limit,
	.Location_Int_Limit=Location_Intt_Limit,
	/*这里是我们的位置环*/

	. Velocity_Kp=27.5,
	. Velocity_Ki=5,
	. Velocity_Kd=0.8,

	. Velocity_Error=0.0,
	. Velocity_Last_Error=0.0,
	. Velocity_Integral=0.0,
	. Velocity_Target_Val=0.0,
	. Velocity_Actual_Val=0.0,
	. Velocity_Out=0.0,
	
	.Velocity_Out_Limit=Velocity_Outt_Limit,
	.Velocity_Int_Limit=Velocity_Intt_Limit
	/*这里是我们的速度环*/
};


PID_Mini XunjiPID={

	. Kp=37.5,
	. Ki=0.01,
	. Kd=0,
	. Error=0,
	. Last_Error=0,
	. Integral=0,
	. Target_Val=0,
	. Actual_Val=0,
	. Out=0,
	

	. Out_Limit=500,
	. Int_Limit=500

};
/*定义PID所用的结构体(包含各个参数)*/



/*位置环的实现*/
void LocationRing_PID(PID_InitTypeDef*My_PID)
{
	/*获取误差*/
	My_PID->Location_Error=My_PID->Location_Target_Val-My_PID->Location_Actual_Val;
	/*累计误差*/
	My_PID->Location_Integral+=My_PID->Location_Error;
	
	/*积分限幅*/
	if(My_PID->Location_Integral>My_PID->Location_Int_Limit)
		My_PID->Location_Integral=My_PID->Location_Int_Limit;
	else if(My_PID->Location_Integral<-My_PID->Location_Int_Limit)
			My_PID->Location_Integral=-My_PID->Location_Int_Limit;	
	
	My_PID->Location_Out=My_PID->Location_Kp*My_PID->Location_Error+My_PID->Location_Ki*My_PID->Location_Integral+My_PID->Location_Kd*(My_PID->Location_Error-My_PID->Location_Last_Error);
	
	/*输出限幅*/
	if(My_PID->Location_Out>My_PID->Location_Out_Limit)
			My_PID->Location_Out=My_PID->Location_Out_Limit;
	else if(My_PID->Location_Out<-My_PID->Location_Out_Limit)
			My_PID->Location_Out=-My_PID->Location_Out_Limit;		
	
		/*记录误差*/
	My_PID->Location_Last_Error=My_PID->Location_Error;
	
}
/*速度环的实现*/
void VelocityRing_PID(PID_InitTypeDef*My_PID)
{
	My_PID->Velocity_Target_Val=My_PID->Location_Out;//串级PID的，那么速度环的输入就是位置环的输出
	
	/*获取误差*/
	My_PID->Velocity_Error=My_PID->Velocity_Target_Val-My_PID->Velocity_Actual_Val;
	/*累计误差*/
	My_PID->Velocity_Integral+=My_PID->Velocity_Error;
	
	/*积分限幅*/
	if(My_PID->Velocity_Integral>My_PID->Velocity_Int_Limit)
		My_PID->Velocity_Integral=My_PID->Velocity_Int_Limit;
	else if(My_PID->Velocity_Integral<-My_PID->Velocity_Int_Limit)
			My_PID->Velocity_Integral=-My_PID->Velocity_Int_Limit;	
	
	
	My_PID->Velocity_Out=My_PID->Velocity_Kp*My_PID->Velocity_Error+My_PID->Velocity_Ki*My_PID->Velocity_Integral+My_PID->Velocity_Kd*(My_PID->Velocity_Error-My_PID->Velocity_Last_Error);
	
	/*输出限幅*/
	if(My_PID->Velocity_Out>My_PID->Velocity_Out_Limit)
			My_PID->Velocity_Out=My_PID->Velocity_Out_Limit;
	else if(My_PID->Velocity_Out<-My_PID->Velocity_Out_Limit)
			My_PID->Velocity_Out=-My_PID->Velocity_Out_Limit;		
	
		/*记录误差*/
	My_PID->Velocity_Last_Error=My_PID->Velocity_Error;
}


void Xunji_Mini_PID(PID_Mini*My_PID)
{
	/*获取误差*/
	My_PID->Error=My_PID->Target_Val-My_PID->Actual_Val;
	/*累计误差*/
	My_PID->Integral+=My_PID->Error;
	
	/*积分限幅*/
	if(My_PID->Integral>My_PID->Int_Limit)
		My_PID->Integral=My_PID->Int_Limit;
	else if(My_PID->Integral<-My_PID->Int_Limit)
			My_PID->Integral=-My_PID->Int_Limit;	
	
	My_PID->Out=My_PID->Kp*My_PID->Error+My_PID->Ki*My_PID->Integral+My_PID->Kd*(My_PID->Error-My_PID->Last_Error);
	
	/*输出限幅*/
	if(My_PID->Out>My_PID->Out_Limit)
			My_PID->Out=My_PID->Out_Limit;
	else if(My_PID->Out<-My_PID->Out_Limit)
			My_PID->Out=-My_PID->Out_Limit;		
	
		/*记录误差*/
	My_PID->Last_Error=My_PID->Error;
}



