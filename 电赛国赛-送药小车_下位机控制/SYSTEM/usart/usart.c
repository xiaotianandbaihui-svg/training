#include "sys.h"
#include "usart.h"	
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F4探索者开发板
//串口1初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/6/10
//版本：V1.5
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持
////////////////////////////////////////////////////////////////////////////////// 	  
 
struct STATE_PACK_1
{
	uint8_t GO_Straight;/*走直线标志*/
	uint8_t Turn_90;/*转弯90度*/
	uint8_t Turn_180;/*原地掉头*/
	uint8_t Turn_Success;/*转弯或者掉头成功*/
	uint8_t Arrive_Target;/*到达目标位置*/
	uint8_t Spin_Success;/*掉头成功*/
};
extern struct STATE_PACK_1	state_pack;

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif
 
#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	

//初始化IO 串口1 
//bound:波特率
void uart_init(u32 bound){
   //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//使能USART1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10复用为USART1
	
	//USART1端口配置
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA9，PA10

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  USART_Init(USART1, &USART_InitStructure); //初始化串口1
	
  USART_Cmd(USART1, ENABLE);  //使能串口1 
	
	//USART_ClearFlag(USART1, USART_FLAG_TC);
	
#if EN_USART1_RX	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启相关中断

	//Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
#endif
	
}
/*
*Res取值
*0：空操作
*2：左转
*3：直行
*4：右转
*/
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

extern PID_InitTypeDef My_PID_1;
extern PID_InitTypeDef My_PID_2;

u8 Res;//记录摄像头传回的方向或者偏离强度
u8 counter=0;






extern uint8_t will_get_num;
extern char recieve_buffer[20];
uint8_t once=0;
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
		
		if(will_get_num==1)
		{
			if(once==0)
			{
				once=1;
				counter=0;
			}
			recieve_buffer[counter]=Res;
		}
		
		counter++;
		if(counter>=20)
		{
			if(will_get_num==1)
			{
				once=0;
				will_get_num=0;
			}
		}
		LCD_ShowNum(60,1,Res,4,16);
		LCD_ShowNum(1,1,counter,4,16);		
//		if(state_pack.GO_Straight==1/*&&fabs(My_PID_1.Location_Error)>=15&&fabs(My_PID_2.Location_Error)>=15*/)/*只有在直行的时候才会直行这个操作*/
//		{
//			XunjiPID.Target_Val=56;
//			XunjiPID.Actual_Val=Res;
//			Xunji_Mini_PID(&XunjiPID);
//			
//			if((double)TIM5->CCR3+XunjiPID.Out>=1000)
//				TIM5->CCR3=1000;
//			else if((double)TIM5->CCR3+XunjiPID.Out<=0)
//				TIM5->CCR3=0;
//			else
//				TIM5->CCR3=(uint16_t)((double)TIM5->CCR3+XunjiPID.Out);
//			
//			if((double)TIM5->CCR1-XunjiPID.Out>=1000)
//				TIM5->CCR1=1000;
//			else if((double)TIM5->CCR1-XunjiPID.Out<=0)
//				TIM5->CCR1=0;
//			else
//				TIM5->CCR1=(uint16_t)((double)TIM5->CCR1-XunjiPID.Out);
//		

			
//			int16_t temp=0;
//			int16_t temp_2=0;
//			if(Res<51)
//			{
//				if(/*TIM5->CCR3!=0*/1)
//				{
//					temp=TIM5->CCR3+20*(56-Res);
//					temp_2=TIM5->CCR1+20*(Res-56);
//					if(temp>=1000)
//						temp=1000;
//					if(temp_2<=0)
//						temp_2=0;
//					TIM5->CCR3=temp;
//					TIM5->CCR1=temp_2;
//				}
//			}
//			else if(Res>51)
//			{
//					if(/*TIM5->CCR1!=0*/1)
//					{
//						temp=TIM5->CCR1+20*(Res-56);
//						temp_2=TIM5->CCR3+20*(56-Res);
//						if(temp>=1000)
//							temp=1000;
//						if(temp_2<=0)
//							temp_2=0;
//							TIM5->CCR1=temp;
//						TIM5->CCR3=temp_2;
//					}
//			}
//			LCD_ShowNum(20,20,temp,4,16);




				


		}
			
		/*if(state_pack.GO_Straight==1)
		{
			printf("%d",2);//表示寻迹模式
		}
		else if(state_pack.Turn_180==1||state_pack.Turn_90==1)
		{
			printf("%d",4);//除了走直线和识别数字摄像头参与，其他都不参与
		}*/
//  } 
} 


void uart_init_2(u32 bound){
   //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //GPIOA10复用为USART1
	
	//USART2端口配置
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA2，PA3

   //USART2 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  	USART_Init(USART2, &USART_InitStructure); //初始化串口2
	
  	USART_Cmd(USART2, ENABLE);  //使能串口1 
	
	//USART_ClearFlag(USART2, USART_FLAG_TC);
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启相关中断

	//Usart2 NVIC 配置
  	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
	
}

void USART2_IRQHandler(void)                	//串口1中断服务程序
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART2);//(USART1->DR);	//读取接收到的数据
	//printf("2");
				counter++;


	//		if(Res<50)
	//		{
	//			/*前轮向右修正*/
	//			uint16_t temp=TIM3->CCR1;
	//			TIM3->CCR1+=200*(50-Res);
	//		}
	//		else if(Res>50)
	//		{
	//			/*前路向左修正*/
	//		}
	//		else 
	//		{
	//			/*空操作，轮子不修正*/
	//		}
			
	
  } 
} 



#endif	

 



