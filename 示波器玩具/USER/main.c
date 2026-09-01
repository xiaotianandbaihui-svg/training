#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"
#include "lcd.h"
#include "key.h"  
#include "touch.h" 
#include "stm32f4xx_conf.h"
#include "ADC_TEST.h"
int main(void)
{ 

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200
	
//	LED_Init();					//初始化LED 
 	LCD_Init();					//LCD初始化 
//	KEY_Init(); 				//按键初始化  
//	tp_dev.init();				//触摸屏初始化
 	POINT_COLOR=BLACK;//设置字体为黑色 
//	MyDMA_Init();
//	MyADC_Init();
//	ADC_Config();
	ADC_Config_For_Multi_Channel();
	My_TIM_Init();
	LCD_ShowString(0,0,264,12,12,"Current Fre:       Hz");
	LCD_ShowString(0,12,264,12,12,"Current Vpx:       mV");
	LCD_ShowString(0,24,264,12,12,"Current Vpn:       mV");
//	int i=0;
	while(1)
	{
//		printf("%d,%d\n",data2[i*2],data2[i*2+1]);
//		i++;
//		if(i==BUFFER*2/2)
//		{
//			i=0;
//		}
//		Data_Process_Two();
//	Data_Process();//第一套测频方法	
		Data_Process_Two_For_Multi_Channel();
//		if(flag_dma_it_tc==1)
//		{
//			flag_dma_it_tc=0;
//		}
//						LCD_ShowNum(0,0,DMA2_Stream0->NDTR,5,12);

//		if(DMA_GetCmdStatus(DMA2_Stream0)!=DISABLE)
//	ADC_Config();

	}
}
