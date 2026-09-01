#include <stm32f4xx.h>
#include "My_ADC_DMA.h"
#include "lcd.h"
#include "sys.h"
#include "delay.h"  
#include "usart.h"    
#include "led.h"
#include "lcd.h"
#include "key.h"  
#include "touch.h" 
#include "My_Flash.h"
#include "sdio_sdcard.h"  

#include "arm_math.h"
#include "math.h"
uint16_t Memory[DATA_BLOCK]={0};
uint16_t Memory2[DATA_BLOCK]={0};
int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);  
	uart_init(115200);		
	
	LED_Init();					
	KEY_Init(); 				  
	tp_dev.init();	
  LCD_Init();	
 	while(SD_Init())//ºÏ≤‚≤ªµΩSDø®
	{
		LCD_ShowString(30,150,200,16,16,"SD Card Error!");
		delay_ms(500);					
		LCD_ShowString(30,150,200,16,16,"Please Check! ");
		delay_ms(500);
		LED0=!LED0;//DS0…¡À∏
	}

	LCD_ShowString(120-12*2,160,12*4,24,24,"Init");
	LCD_ShowString(120-8*5,160+24+16,8*11,16,16,"Please Wait");

	
//	My_Flash_Era_Unlock();
	LCD_Clear(WHITE);
	Init_ADC_DMA_Part((uint32_t)Memory);
	
	DAC_Part_Init();
	while(1)
	{
//		LCD_ShowNum(0,116,Memory[100],4,16);
		Re_Init();
		Record_Suspend_Start();
		Key_For_DAC();
	}
}
