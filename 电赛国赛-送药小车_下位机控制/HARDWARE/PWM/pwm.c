#include "pwm.h"
#include "led.h"
#include "usart.h"
//TIM14 PWM部分初始化 
//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数
void TIM8_PWM_Init_(u16 arr,u16 psc,u16* oc_count)
{		 					 
	//此部分需手动修改IO口设置
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);  	//TIM8时钟使能    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); 	//使能PORTF时钟	

	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_TIM8); 
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_TIM8); 
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource8,GPIO_AF_TIM8); 
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource9,GPIO_AF_TIM8); 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;           
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       
	GPIO_Init(GPIOC,&GPIO_InitStructure);              
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;           
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;      
	GPIO_Init(GPIOC,&GPIO_InitStructure);              

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;          
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;       
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       
	GPIO_Init(GPIOC,&GPIO_InitStructure);             

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;          
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       
	GPIO_Init(GPIOC,&GPIO_InitStructure);             


	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseStructure.TIM_Period=arr;   //自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM8,&TIM_TimeBaseStructure);//初始化定时器8
	
	//初始化TIM8 Channel1 PWM模式 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
	
	TIM_OCInitStructure.TIM_Pulse=oc_count[0];
	TIM_OC1Init(TIM8, &TIM_OCInitStructure);  
	TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
	
	TIM_OCInitStructure.TIM_Pulse=oc_count[1];
	TIM_OC2Init(TIM8, &TIM_OCInitStructure);  
	TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Enable);

	TIM_OCInitStructure.TIM_Pulse=oc_count[2];
	TIM_OC3Init(TIM8, &TIM_OCInitStructure);  
	TIM_OC3PreloadConfig(TIM8, TIM_OCPreload_Enable);

	TIM_OCInitStructure.TIM_Pulse=oc_count[3];
	TIM_OC4Init(TIM8, &TIM_OCInitStructure); 
	TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);
	
	TIM_CtrlPWMOutputs(TIM8, ENABLE);

  TIM_ARRPreloadConfig(TIM8,ENABLE);//ARPE使能 eeeerrr
 
	TIM_Cmd(TIM8, ENABLE);  
}  


void TIM5_PWM_Init_(u16 arr,u16 psc,u16* oc_count)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);  	//TIM5时钟使能    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 	//使能PORTF时钟	

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM5); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_TIM5); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_TIM5); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_TIM5); 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;           
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       
	GPIO_Init(GPIOA,&GPIO_InitStructure);              
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;           
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;      
	GPIO_Init(GPIOA,&GPIO_InitStructure);              

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;          
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;       
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       
	GPIO_Init(GPIOA,&GPIO_InitStructure);             

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;          
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       
	GPIO_Init(GPIOA,&GPIO_InitStructure);             

	
	TIM_TimeBaseStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseStructure.TIM_Period=arr;   //自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseStructure);//初始化定时器14
	
	//初始化TIM5 Channel1 PWM模式 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
	TIM_OCInitStructure.TIM_Pulse=oc_count[0];
	TIM_OC1Init(TIM5, &TIM_OCInitStructure);  


	
	TIM_OCInitStructure.TIM_Pulse=oc_count[1];
	TIM_OC2Init(TIM5, &TIM_OCInitStructure);  


	TIM_OCInitStructure.TIM_Pulse=oc_count[2];
	TIM_OC3Init(TIM5, &TIM_OCInitStructure);  


	TIM_OCInitStructure.TIM_Pulse=oc_count[3];
	TIM_OC4Init(TIM5, &TIM_OCInitStructure);  


 
  TIM_ARRPreloadConfig(TIM5,ENABLE);//ARPE使能 eeeerrr
	TIM_Cmd(TIM5, ENABLE);  	
}

/*正反转*/
void Foward_Back_Control_GPIO_Init()
{
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
//	GPIO_InitTypeDef GPIO_InitStructure;
//	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
//	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
//	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
//	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
//	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
//	GPIO_Init(GPIOC,&GPIO_InitStructure);
}


void Foward_Back(int16_t* Speed)
{
	
	if(Speed[0]>0)
	{
		TIM_SetCompare1(TIM5,Speed[0]);
		TIM_SetCompare2(TIM5,0);		
	}
	else  if(Speed[0]<0)
	{
		TIM_SetCompare1(TIM5,0);
		TIM_SetCompare2(TIM5,-Speed[0]);		
	}
	

	if(Speed[1]>0)
	{
		TIM_SetCompare3(TIM5,Speed[1]);
		TIM_SetCompare4(TIM5,0);		
	}
	else  if(Speed[1]<0)
	{
		TIM_SetCompare3(TIM5,0);
		TIM_SetCompare4(TIM5,-Speed[1]);		
	}

	if(Speed[2]>0)
	{
		TIM_SetCompare1(TIM8,Speed[2]);
		TIM_SetCompare2(TIM8,0);		
	}
	else  if(Speed[2]<0)	
	{
		TIM_SetCompare1(TIM8,0);
		TIM_SetCompare2(TIM8,-Speed[2]);		
	}
	
	if(Speed[3]>0)
	{
		TIM_SetCompare3(TIM8,Speed[3]);
		TIM_SetCompare4(TIM8,0);		
	}
	else if(Speed[3]<0)
	{
		TIM_SetCompare3(TIM8,0);
		TIM_SetCompare4(TIM8,-Speed[3]);		
	}	
	
	if(Speed[0]==0&&Speed[1]==0&&Speed[2]==0&&Speed[3]==0)
	{
		TIM_SetCompare1(TIM5,1000);
		TIM_SetCompare2(TIM5,1000);		
		TIM_SetCompare3(TIM5,1000);
		TIM_SetCompare4(TIM5,1000);		

		TIM_SetCompare1(TIM8,1000);
		TIM_SetCompare2(TIM8,1000);		
		TIM_SetCompare3(TIM8,1000);
		TIM_SetCompare4(TIM8,1000);		

	}
}
