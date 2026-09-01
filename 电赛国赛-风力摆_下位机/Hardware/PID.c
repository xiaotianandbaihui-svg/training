#include "stm32f10x.h"                  // Device header
#include "PID.h"
#include "math.h"
#include "inv_mpu.h"
#include "Delay.h"
#include "stdio.h"
#include "Serial.h"
#include "OLED.h"
#include "nrf24l01.h"
#include "Function.h"



//俯仰角的角度和角速度环
PID_t Pitch_angle;         //角度    
PID_t Pitch_angularSpeed;  //角速度

//翻滚角的角度和角速度环
PID_t Roll_angle;
PID_t Roll_angularSpeed;

//配置定时器 使得每过1ms进入一次中断 读取当前的角度 产生一个PWM输出给到电机
//可以有每过40ms进行一次调整
int count,time;
float Pitch,Roll,Yaw;
float gyro_pitch,gyro_roll;

short gyro_raw[3]={0}; //角速度的原始值
float gyro_real[3]={0};//角速度的真实值
float gyro_scale=16.4f;//角速度计算因子

float ave_filter[5][3]={0};   //滑动滤波窗口
float sum_angularSpeed[3]={0};//零漂抑制(角速度)
float sum_angle[3]={0};       //零漂抑制(角度)

extern uint8_t Data_tx[32];
extern uint8_t Data_rx[32];

extern float gyro_last;
extern float gyro_current;
extern float gyro_error;



//下位机主要负责接收数据
//下位机也要发送角度 角加速度给上位机 让上位机调参




void PID_TIM_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			//开启TIM2的时钟
	
	/*配置时钟源*/
	TIM_InternalClockConfig(TIM2);		              //选择TIM2为内部时钟，若不调用此函数，TIM默认也为内部时钟
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	//计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;				    //计数周期，即ARR的值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;				//预分频器，即PSC的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;			//重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);				//将结构体变量交给TIM_TimeBaseInit，配置TIM2的时基单元	
	
	/*中断输出配置*/
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);						//清除定时器更新标志位
																//TIM_TimeBaseInit函数末尾，手动产生了更新事件
																//若不清除此标志位，则开启中断后，会立刻进入一次中断															//如果不介意此问题，则不清除此标志位也可
	
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);					//开启TIM2的更新中断
	
	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);				//配置NVIC为分组2
																//即抢占优先级范围：0~3，响应优先级范围：0~3
																//此分组配置在整个工程中仅需调用一次
																//若有多个中断，可以把此代码放在main函数内，while循环															//若调用多次配置分组的代码，则后执行的配置会覆盖先执行的配置
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;						//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;				//选择配置NVIC的TIM2线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//指定NVIC线路的抢占优先级为2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);								//将结构体变量交给NVIC_Init，配置NVIC外设
	
	/*TIM使能*/
	TIM_Cmd(TIM2, ENABLE);			                            //使能TIM2，定时器开始运行
	

}

uint8_t flag=0;
uint8_t flagx=0;
uint8_t state=0;
uint8_t flag_swing;
extern int flag_start;
int start_time=0;
int deal_time;
extern int flag_count;
extern float Pitch_max;
extern float Pitch_max_temp;
int deal_flag;



//void TIM2_IRQHandler(void)
//{
//	
//	
//	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
//	{
//		
//		
//		if(flag_start==1)
//		{
//			start_time++;
//			if(start_time>=500)
//			{
//				flag_start=0;
//			    start_time=0;
//				TIM_SetCompare3(TIM3,0);			
//			}
//			
//		}
////		if(flagx==0)
////		{
////			flagx=1;			
////			state=NRF24L01_GetRxBuf(Data_rx);
////		}
////		else if(flagx==1&&state==0)
////		{
////			flagx=0;
////			NRF24L01_SendBuf(Data_tx);
//////			if(state==0)
//////			{
//////				/*收到数据了进行的操作*/
//////			}
////		}
////				
////		
////		time++;
////		if(time>=5)
////		{
////			time=0;
////			if(flag==0)
////			{
////				
////				NRF24L01_SendBuf((uint8_t*)Data_tx);
////				
////				
////			}
////			
////		}	
//		
//	    count++;                                  //5ms获取一次数据
//		if(count>=10)
//	{
//		
//		flag_swing=1;
//		count=0;
//			
//		gyro_last=gyro_real[0];                   //现在的俯仰角速度
//		
//		MPU6050_DMP_Get_Data(&Pitch,&Roll,&Yaw);  //获取当前的角度值	
//		
//		Get_Gyro();
//			
//		gyro_current=gyro_real[0];
//		
//		gyro_error=fabs(gyro_current-gyro_last);
//		
//		
//		Pitch=Pitch-sum_angle[0];
//		Roll=Roll-sum_angle[1];
//		
//		send_tx();
//		
//		draw_line(30,0);
//		
//		
//	    NRF24L01_SendBuf((uint8_t*)Data_tx);
//			
//	 }
//	
//	
//		  flag_count++;                              //更新最大值的地方
//		  if(fabs(Pitch)>Pitch_max_temp)
//	      {			  
//			  Pitch_max_temp=fabs(Pitch);			  
//	      }
//		  if(flag_count>=500)
//		  {			  
//			  flag_count=0;
//			  Pitch_max=Pitch_max_temp;
//			  Pitch_max_temp=0;			  
//		  }
//		  
////	 deal_time++;//每20ms调控一次
////	 if(deal_time>=20)
////	 {		 
////		 deal_time=0;
////		 deal_flag=1;	 
////		 
////	 }
//	 		
//	 
//	 
//	 
//			
////			//进行PID的调整
////			//目标值在外面设定 在主循环里通过无线发送模块发送过来的模式实时更新目标值
////			//读取实际值
//////			MPU6050_DMP_Get_Data(&Pitch,&Roll,&Yaw);//获取当前的角度值
////			Get_Gyro();
//////			
//////			//每进一次中断 读取一次实际值 每一个值都对应一个PID环路的实际值 我们要确定的目标值只有两个角度，通过主循环来确定
////			Pitch=Pitch-sum_angle[0];
////			Roll=Roll-sum_angle[1];
//////			
////			gyro_pitch=gyro_real[1];
////		    gyro_roll=gyro_real[0];
//////			//得到实际值和目标值之后，再调用PID的代码  进行环路控制
//////			Pitch_angle.Target=由外面确定
//////			//Roll_angle.Target
////			Pitch_angle.Actual=Pitch;			
////			Roll_angle.Actual=Roll;
//////			
//////			Pitch_angularSpeed.Actual=gyro_pitch;
//////			Roll_angularSpeed.Actual=gyro_roll;
//////			
//////			//先只写俯仰角			
////			PID_Update(&Pitch_angle);
////			
////			PWM=Pitch_angle.Out;
//			
//			
////			Pitch_angularSpeed.Target=Pitch_angle.Out;//out输出值决定角加速度的目标值 调参确定角加速度的大小
////			//以及角加速度大概在哪个范围 让调参有一个大致的方向
////			PID_Update(&Pitch_angularSpeed);//此时的out值直接决定PWM
////			//由于out值有正有负  需要确定是哪个电机在工作
////			//比如我现在一二PWM给俯仰角
////            if(Pitch_angularSpeed.Out>0)
////			{
////			   TIM_SetCompare1(TIM3,Pitch_angle.Out);//注意现在PWM的范围在0-100之间
////			   TIM_SetCompare2(TIM3,0);		
////			}
////			else if(Pitch_angularSpeed.Out<0)
////			{
////			   TIM_SetCompare2(TIM3,-Pitch_angle.Out);//注意现在PWM的范围在0-100之间
////			   TIM_SetCompare1(TIM3,0);		
////			}
////			
//			//接下来写翻滚角
//			
//			
//			
//			//Serial_Printf("\r\n Pitch=%f Roll=%f Yaw=%f",Pitch,Roll,Yaw);
//			
//		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//	}
//}


void PID_Update(PID_t *p)
{
	p->Err1 = p->Err0;
	p->Err0 = p->Target - p->Actual;
	
	if (p->Ki != 0)
	{
		if (p->ErrIntThreshold == 0)
		{
			p->ErrInt += p->Err0;
		}
		else if (fabs(p->Err0) < p->ErrIntThreshold)
		{
			p->ErrInt += p->Err0;
		}
		else
		{
			p->ErrInt = 0;
		}
	}
	else
	{
		p->ErrInt = 0;
	}
	
	if(p->ErrInt>p->ErrIntMax)
	{
		p->ErrInt=p->ErrIntMax;	//积分限幅
	}
	
	
	
	p->Out = p->Kp * p->Err0
		   + p->Ki * p->ErrInt
		   + p->Kd * (p->Err0 - p->Err1);
	
	//这里的输出直接作用于pwm的ccr 所以限幅100
	
	
	
	
	if (p->Out > p->OutMax) {p->Out = p->OutMax;}
	if (p->Out < p->OutMin) {p->Out = p->OutMin;}
}


void PID_Clear(PID_t *p)
{
	p->Err0 = 0;
	p->Err1 = 0;
	p->ErrInt = 0;
}

//先只写好俯仰的就可以




//void test_self_angularSpeed(void)//角速度，零偏校准
//{
//	
//	
//	for(int i=0;i<100;i++)
//	{
//		if(mpu_get_gyro_reg(gyro_raw,NULL)==0)
//		{
//			sum_angularSpeed[0]+=gyro_raw[0];
//			sum_angularSpeed[1]+=gyro_raw[1];
//			sum_angularSpeed[2]+=gyro_raw[2];			
//		}
//	}
//	sum_angularSpeed[0]/=100.0;
//	sum_angularSpeed[1]/=100.0;
//	sum_angularSpeed[2]/=100.0;
//}

//void test_self_angle(void)        //角度零漂校准
//{
//		 
//	for(int i=0;i<10;i++)
//	{
//		Delay_ms(2000);
//	}
//	

//	for(int i=0;i<500;i++)
//	{	
//		MPU6050_DMP_Get_Data(&Pitch,&Roll,&Yaw);
//		Delay_ms(10);
//		sum_angle[0]+=Pitch;
//		sum_angle[1]+=Roll;
//		sum_angle[2]+=Yaw;
//	}
//	sum_angle[0]/=500.0;
//	sum_angle[1]/=500.0;
//	sum_angle[2]/=500.0;

//}

void Get_Gyro(void)        //获取角速度(这个函数里使用了5点窗滑动均值滤波)
{
	
		if(mpu_get_gyro_reg(gyro_raw,NULL)==0)
		{
			for(int j=0;j<4;j++)
			{
				for(int i=0;i<3;i++)
				{
					ave_filter[j][i]=ave_filter[j+1][i];
				}
			}
			
			for(int i=0;i<3;i++)
			{
				ave_filter[4][i]=((gyro_raw[i]-sum_angularSpeed[i])/gyro_scale);
			}
			gyro_real[1]=((ave_filter[0][0]+ave_filter[1][0]+ave_filter[2][0]+ave_filter[3][0]+ave_filter[4][0]))/5.0;
			gyro_real[0]=((ave_filter[0][1]+ave_filter[1][1]+ave_filter[2][1]+ave_filter[3][1]+ave_filter[4][1]))/5.0;
			gyro_real[2]=((ave_filter[0][2]+ave_filter[1][2]+ave_filter[2][2]+ave_filter[3][2]+ave_filter[4][2]))/5.0;
			
		}
		
}
		

//void Recieve_And_Send_IT(void)
//{
//		  
//	      if(flag==0)
//		{
////			OLED_ShowFloatNum(0,16,DataToReceive[0],2,2,OLED_6X8);
////			OLED_ShowFloatNum(0,24,DataToReceive[1],2,2,OLED_6X8);
//			
////			Data_tx[0]=Pitch;
////			Data_tx[1]=Roll;
////			Data_tx[2]=gyro_real[0];
////			Data_tx[3]=gyro_real[1];			
//			NRF24L01_SendBuf((uint8_t*)Data_tx);
//			
//			
//		}
//		flag=NRF24L01_GetRxBuf((uint8_t*) Data_rx);
//	
//	
//}











