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
#include "math.h"
#include "Function.h"


//Pitch  俯仰角
//Roll   翻滚角
//Yaw    偏航角




extern float Pitch,Roll,Yaw;
extern float sum_angle[3]; 
extern float gyro_real[3];



extern float Pitch_temp,Roll_temp;
extern float gyro_real_temp[3];

extern uint8_t Data_tx[32];
extern uint8_t Data_rx[32];




//完成的几种模式
//画直线
//指定角度和长度画直线
//画圆
//手柄控制




//起摆程序
//角度
//俯仰角 顺时针为正  
//翻滚角 逆时针为正

//角速度
//俯仰角 顺时针为正 
//翻滚角 逆时针为正


//大概范围
//角度范围  (-50，50)
//角速度    (-40,40)
//写起摆程序

//摆到一定的角度就停止


//起摆到一定程度后退出起摆程序

//俯仰角的起摆
//起摆时一个电机开始转动
//
//
//

//		TIM_SetCompare1(TIM3,5);//右边
//		TIM_SetCompare2(TIM3,100);//前面
//		TIM_SetCompare3(TIM3,5);//后面
//		TIM_SetCompare4(TIM3,5);//左边


//用角加速度可以吗 两个角速度的差如果小于5 则停风扇的转动 再判断角速度方向 来确定另一个电机的运动
//

float gyro_last;
float gyro_current;
float gyro_error;
float Pitch_max;
float Pitch_max_temp;
int   flag1,flag2;
extern uint8_t flag_swing;
int flag_count;
int flag_start;

extern PID_t Pitch_angle;
extern PID_t Roll_angle;






//起振程序
//参数 起振的角度 
//达到角度后返回0
uint8_t start_swing(float Pitch_target)
{
	 
	while(1)
	{
				 
		 if(flag_swing==1)
	    {
			
		  flag_swing=0; 
			
		  
		  if(Pitch_max<3)
		  {
			  flag_start=1;
			  TIM_SetCompare3(TIM3,100);			  
		  }
		  
		if(flag_start==0)
		{			
			
		  
		 if(Pitch_max<Pitch_target)
		 {	 
			 
			 
         if(gyro_real[0]>0)
         {
			 flag1=1;
			 TIM_SetCompare3(TIM3,30);		 
		 }
		 
		 
         if(flag1==1&&Pitch>0&&gyro_real[0]>0)
		 {
			 flag1=0;
		     TIM_SetCompare3(TIM3,0);			 
		 }
		  
	     }
		 
		 else 
		 {			 
			  TIM_SetCompare3(TIM3,0);			 
			  return 0;			 
		 }
		 
		 
	    }
		 
	    }
		 
		
	}

}

void send_tx(void)
{
		if(Pitch>=0)
		{
		   Data_tx[0]=0;					
		   Data_tx[1]=Pitch;
		   Data_tx[2]=Pitch*100-Data_tx[1]*100;
			
		}
		else if(Pitch<0)
		{
		   Data_tx[0]=1;
		   Pitch_temp=-Pitch;
		   Data_tx[1]=Pitch_temp;
		   Data_tx[2]=Pitch_temp*100-Data_tx[1]*100;
		   
		}

		
		if(Roll>=0)
		{
		  Data_tx[3]=0;
		  Data_tx[4]=Roll;
		  Data_tx[5]=Roll*100-Data_tx[4]*100;
		}
		else if(Roll<0)
		{
		  Data_tx[3]=1;	
		  Roll_temp=-Roll;
          Data_tx[4]=Roll_temp;
		  Data_tx[5]=Roll_temp*100-Data_tx[4]*100;		
		}	
	
		if(gyro_real[0]>=0)
		{
		  Data_tx[6]=0;
		  Data_tx[7]=gyro_real[0];
		  Data_tx[8]=gyro_real[0]*100-Data_tx[7]*100;
			
		}
		else if(gyro_real[0]<0)
		{
		  Data_tx[6]=1;
		  gyro_real_temp[0]=-gyro_real[0];
		  Data_tx[7]=gyro_real_temp[0];
		  Data_tx[8]=gyro_real_temp[0]*100-Data_tx[7]*100;	
		}
		 
		
		if(gyro_real[1]>=0)
		{
		  Data_tx[9]=0;
		  Data_tx[10]=gyro_real[1];
			
		  Data_tx[11]=gyro_real[1]*100-Data_tx[10]*100;
			
		}
		else if(gyro_real[1]<0)
		{
			
		  Data_tx[9]=1;
		  gyro_real_temp[1]=-gyro_real[1];
          Data_tx[10]=gyro_real_temp[1];
		  Data_tx[11]=gyro_real_temp[1]*100-Data_tx[10]*100;
			
		}
		
		Data_tx[12]=gyro_error;
		Data_tx[13]=gyro_error*100-Data_tx[12]*100;	
	
}

//画直线 画指定长度和角度的直线
//先只画一个俯仰角 特定的角度 


#define H       90          //离地高度
#define pole_L  54          //杆长
#define g       980         //重力加速度
#define PI      3.14159

int Pitch_PWM;
int Roll_PWM;

//对角速度进行PID 进行角速度跟随

extern PID_t Pitch_angularSpeed;
extern PID_t Roll_angularSpeed;
void draw_line(float circle_L,float circle_angle)//角度在0-180范围内
{
	//知道了长度和角度之后 算出起摆的总能量
	
	int state;//摆的时候两个方向的速度需要对应
	if(cos(circle_angle*PI/180)>0)
	{
		state=-1;  //表示角速度方向相反
	}
	else if(cos(circle_angle*PI/180)<0)
	{
		state=1;   //表示角速度方向一致
	}
	
	float pole_angle;
	float pole_angle_current;
	float x,y,r;
	float V_target;
	float V_current;
	float V_x_target;
	float V_y_target;
	
	float V_x_current;
	float V_y_current;	
	
	pole_angle=atan(circle_L/H);
	x=H*tan(fabs(Pitch*PI/180));
	y=H*tan(fabs(Roll*PI/180));
	r=sqrt(x*x+y*y);
	
    pole_angle_current=atan(r/H);
		
	if(pole_angle_current>0)
	{
		Data_tx[18]=0;	
		Data_tx[19]=pole_angle_current*100;
	}	
	else if(pole_angle_current<0)
	{
		Data_tx[18]=1;
		Data_tx[19]=-pole_angle_current*100;
	}

	
	if((pole_angle_current-pole_angle)<0)
	{			   
	   V_target=sqrt(2*g*pole_L*(cos(pole_angle_current)-cos(pole_angle)));
	}
	else
    {		
		V_target=0;
	}	
	
	   V_x_current=gyro_real[0]*pole_L*PI/180;
	   V_y_current=gyro_real[1]*pole_L*PI/180;         //即厘米每秒的速度
		
	   if(V_x_current<0)
	   {
          V_x_target=-V_target*fabs(cos(circle_angle));
		  V_y_target=-V_target*sin(circle_angle)*state;
	   }
	   
	   if(V_x_current>0)
	   {
		  V_x_target=V_target*fabs(cos(circle_angle));
		  V_y_target=V_target*sin(circle_angle)*state;	   
	   }
	   
	   //把两个参数都传过去
	   if(V_x_target>0)
	   {
		   Data_tx[14]=0;
		   Data_tx[15]=V_x_target;
	   }
	   else if(V_x_target<0)
	   {
		   Data_tx[14]=1;
		   Data_tx[15]=-V_x_target;  
	   }
	   
	   if(V_x_current>0)
	   {
		   Data_tx[16]=0;
		   Data_tx[17]=V_x_current;	   
	   }
	   else if(V_x_current<0)
	   {
		    Data_tx[16]=1;
		    Data_tx[17]=-V_x_current;	   
	   }
	   

	   
	   
	   
	   
	   
	   //接下来进行PID的调控
	   //先对俯仰角进行PID
	   
//	   Pitch_angularSpeed.Actual=V_x_current;
//	   Pitch_angularSpeed.Target=V_x_target;
//	   
//	   PID_Update(&Pitch_angularSpeed);
//	   
//	   Pitch_PWM=Pitch_angularSpeed.Out;
//	   
//	   if(Pitch_PWM>0)
//	   {
//		   	TIM_SetCompare3(TIM3,Pitch_PWM);
//		    TIM_SetCompare2(TIM3,0);   
//	   }
//	   else if(Pitch_PWM<0)
//	   {
//		   TIM_SetCompare3(TIM3,0);
//		   TIM_SetCompare2(TIM3,-Pitch_PWM);   
//	   }


//	   Roll_angularSpeed.Actual=V_y_current;
//	   Roll_angularSpeed.Target=V_y_target;
//	   
//	   PID_Update(&Roll_angularSpeed);
//	   
//	   Roll_PWM=Roll_angularSpeed.Out;
//	   
//	   if(Roll_PWM>0)
//	   {
//		   TIM_SetCompare4(TIM3,Roll_PWM);
//		   TIM_SetCompare1(TIM3,0);
//	   }
//	   else if(Roll_PWM<0)
//	   {
//		   TIM_SetCompare4(TIM3,0);
//		   TIM_SetCompare1(TIM3,-Roll_PWM);
//	   }
	
}


//画指定半径的圆
//给出半径r，要写出需要的切向速度


//地板上的数值除以20得到真实距离，距离单位为厘米
PID_t circle_Pitch_angularSpeed;
PID_t circle_Roll_angularSpeed;

void draw_circle(float circle_r)
{
	
	float pole_angle;
	float V;
	float target_r;
	float circle_angle;
	float V_x_target;
	float V_y_target;
	float V_x_current;
	float V_y_current;
	
	int circle_pitch_pwm;
	int circle_roll_pwm;
	
	
	pole_angle=atan(circle_r/H);
	target_r=pole_L*sin(pole_angle);
	
	V=sqrt(g*tan(pole_angle)*target_r);//切向加速度
	
	//然后确定旋转方向，进行速度分解
	
	circle_angle=fabs(atan(tan(Roll*PI/180)/tan(Pitch*PI/180)));
	
	//接下来进行速度分解
	if(Pitch>0&&Roll<0)
	{
		V_x_target=V*sin(circle_angle);
		V_y_target=V*cos(circle_angle);
	}
	if(Pitch<0&&Roll<0)
	{
		V_x_target=V*sin(circle_angle);
		V_y_target=-V*cos(circle_angle);	
	}
	if(Pitch<0&&Roll>0)
	{
		V_x_target=-V*sin(circle_angle);
		V_y_target=-V*cos(circle_angle);	
	}
	if(Pitch>0&&Roll>0)
	{
		V_x_target=-V*sin(circle_angle);
		V_y_target=V*cos(circle_angle);
	}
	
	
	 V_x_current=gyro_real[0]*pole_L*PI/180;
	 V_y_current=gyro_real[1]*pole_L*PI/180;
	
	 //接下来进行两个PID的调控
	
	 circle_Pitch_angularSpeed.Actual=V_x_current;
	 circle_Pitch_angularSpeed.Target=V_x_target;
	
	 PID_Update(&circle_Pitch_angularSpeed);
	 
	 circle_pitch_pwm=circle_Pitch_angularSpeed.Out;
	
	  if(circle_pitch_pwm>0)
	  {
	     TIM_SetCompare3(TIM3,circle_pitch_pwm);
	     TIM_SetCompare2(TIM3,0);   
	  }
	  else if(circle_pitch_pwm<0)
	  {
	    TIM_SetCompare3(TIM3,0);
	    TIM_SetCompare2(TIM3,-circle_pitch_pwm);   
	  }
	  
	  circle_Roll_angularSpeed.Actual=V_y_current;
	  circle_Roll_angularSpeed.Target=V_y_target;
	  
	  PID_Update(&circle_Roll_angularSpeed);
	  
	  circle_roll_pwm=circle_Roll_angularSpeed.Out;
	  
	  if(circle_roll_pwm>0)
	  {
		  TIM_SetCompare4(TIM3,circle_roll_pwm);
		  TIM_SetCompare1(TIM3,0);	  
	  }
	  else if(circle_roll_pwm<0)
	  {
		  TIM_SetCompare4(TIM3,0);
		  TIM_SetCompare1(TIM3,-circle_roll_pwm);	  
	  }
}


//根据最高点的角度与目标角度的区别，再给出哪个电机转动 并且转动的时间
//void draw_line(float Pitch_target,float Roll_target)
//{

//	float Pitch_error;
//	float Roll_error;
//	
//	Pitch_error=Pitch_target-Pitch_max;
//	
//	Pitch_time=10*fabs(Pitch_error);
//	
//	if(Pitch_error>0)
//	{
//		//加速  就用一个电机		
//		if(gyro_real[0]>0)
//		{			
//		   TIM_SetCompare3(TIM3,30);				
//		}	
//	}
//	else if(Pitch_error<0)
//	{
//		
//		
//		
//		
//		
//	
//	}
//	
//	
//	
//	
////	if(Pitch_max<Pitch_target-3)
////	{
////	  start_swing(Pitch_target+10);
////	}
//	
////	Pitch_angle.Actual=Pitch;
////	Roll_angle.Actual=Roll;
////	//画线时这两个要实时变化
////	
////	if(Pitch>10&&fabs(Pitch-Pitch_target)<2)
////	{	
////	   Pitch_angle.Target= -Pitch_target;
////	}
////	if(Pitch<-10&&fabs(Pitch_target+Pitch)<2)
////	{
////	   Pitch_angle.Target=Pitch_target;
////	}
////	
////	Roll_angle.Target= Roll_target;
////	
////	PID_Update(&Pitch_angle);
////	
////	PWM=Pitch_angle.Out;
////	
////	if(PWM>0)
////	{
////		TIM_SetCompare3(TIM3,PWM);
////      TIM_SetCompare2(TIM3,0);
////	}
////	else if(PWM<0)
////	{
////		TIM_SetCompare2(TIM3,-PWM);	
////		TIM_SetCompare3(TIM3,0);
////	}

//}


//volatile float length=0,sway_angle=0,current_sway_angle=0,standard_speed=0,pid_speed_y=0;
//	static PID_LocTypeDef PID_straight;
//			PID_straight.kp=482;	
//			PID_straight.ki=18.7;
//			PID_straight.kd=2;
//	set_length=set_length/2;//设置直线长度的一半
//	sway_angle=atan(set_length/POLE_LENGTH);//设定的杆的摆角，该角度为弧度制
// 	while(1)
//	{
//		if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0)//读取欧拉角的值
//		{ 
//			length=sqrt((POLE_LENGTH*tan(pitch*PI/180))*(POLE_LENGTH*tan(pitch*PI/180))
//							+(POLE_LENGTH*tan(roll*PI/180))*(POLE_LENGTH*tan(roll*PI/180)));//计算激光点距圆心距离
//			current_sway_angle=atan(length/POLE_LENGTH);//计算当前摆角,该角度为弧度制
//			if(roll<0) length=-length;
//			if(fabs(length)<set_length)
//			standard_speed=(sqrt(2*980*SWAY_LENGTH*(cos(current_sway_angle)-cos(atan(set_length/POLE_LENGTH))))
//			/SWAY_LENGTH)*180/PI;//计算角速度，单位为度/s
//			else standard_speed=0;
//			if(current_angle_speed_y<0) standard_speed=-standard_speed;
//			if(pid_flag==0)
//			{
//				pid_flag=1;
//				pid_speed_y=PID_location(standard_speed,current_angle_speed_y,&PID_straight);
//				motor_y(pid_speed_y);
//			}//20ms计算一次PID并执行
//			send_data[0]=pitch*10;
//			send_data[1]=roll*10;
//			send_data[2]=length;
//			send_data[3]=angle;
//			send_data[4]=current_angle_speed_x;
//			send_data[5]=current_angle_speed_y;
//			send_data[6]=standard_speed_x;
//			send_data[7]=standard_speed_y;
//			vcan_sendware((uint8_t *)send_data,sizeof(send_data));//向上位机发送数据并显示波形
//		}
//	}


























