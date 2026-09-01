#include "exti.h"
#include "delay.h" 
#include "led.h" 
#include "key.h"
#include "Encoder.h"
#include "Op_CORE.h"
typedef struct 
{
	 uint8_t start;/*开始启动*/
	uint8_t processing;/*小车在路上*/
	uint8_t arrive_target;/*小车到达药房*/
	uint8_t re_start;/*小车返回开始*/
	uint8_t arrive_start;/*小车到达起点*/
}state_pack_start_end;

extern state_pack_start_end state_pack_2;

extern uint8_t mode;
//外部中断3服务程序
void EXTI3_IRQHandler(void)
{
	delay_ms(20);	//消抖
	if(KEY1==0&&GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==1)	 
	{				 
		GPIO_ResetBits(GPIOF,GPIO_Pin_0);//绿灯熄灭
		GPIO_SetBits(GPIOC,GPIO_Pin_1);/*红灯亮*/		
		/*点灯*/		
		state_pack_2.arrive_start=0;
		mode=1;
		state_pack_2.start=1;//表示在模式2开始
	}		 
	 EXTI_ClearITPendingBit(EXTI_Line3);  //清除LINE3上的中断标志位  
}
//外部中断4服务程序
void EXTI4_IRQHandler(void)
{
	delay_ms(20);	//消抖
	if(KEY0==0&&GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==1)	 
	{				 
		GPIO_ResetBits(GPIOF,GPIO_Pin_0);//绿灯熄灭
		GPIO_SetBits(GPIOC,GPIO_Pin_1);/*红灯亮*/		
		/*点灯*/		
		state_pack_2.arrive_start=0;
		mode=0;//设置为为模式1，也就是说我们现在要开始去近端药房
		state_pack_2.start=1;//表示开始了
	}		 
	 EXTI_ClearITPendingBit(EXTI_Line4);//清除LINE4上的中断标志位  
}
	   
//外部中断初始化程序
//初始化PE2~4,PA0为中断输入.
void EXTIX_Init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//使能SYSCFG时钟
	
 
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource3);//PE3 连接到中断线3
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource4);//PE4 连接到中断线4
	
	
	/* 配置EXTI_Line3,4 */
	EXTI_InitStructure.EXTI_Line =  EXTI_Line3 | EXTI_Line4;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿触发
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//中断线使能
  EXTI_Init(&EXTI_InitStructure);//配置
 	
	
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;//外部中断3
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置
	
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;//外部中断4
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级1
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置   
}












