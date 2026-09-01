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


/*我第一遍文件系统移植失败，大概率是因为底层驱动没搞好*/



//#include "arm_math.h"
//#include "math.h"
#include "ff.h"
#include "diskio.h"
FIL fp;
FATFS fs;//文件系统
MKFS_PARM format_opt = {
    .fmt = FM_FAT32,     // 强制格式化为FAT32,
    .n_fat = 1,          // 单FAT表
    .align = 0,          // 自动对齐
    .n_root = 0,         // FAT32忽略此参数
    .au_size = 0         // 自动选择簇大小
};

__attribute__ ((aligned(8))) uint16_t Memory[DATA_BLOCK]={0};
__attribute__ ((aligned(8))) uint16_t Memory2[DATA_BLOCK]={0};
uint8_t work_buffer[4096];
int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);  
	uart_init(115200);		
	
	LED_Init();					
	KEY_Init(); 				  
	tp_dev.init();	
  LCD_Init();	
   /* SD卡物理层初始化 */
//    while(SD_Init()) { 
//        LCD_ShowString(30,150,200,16,16,"SD Card Error!");
//        delay_ms(500);					
//        LCD_ShowString(30,150,200,16,16,"Please Check! ");
//        delay_ms(500);
//    }
	
	/*系统初始化及挂载*/
	if (disk_initialize(0) != RES_OK) {
			printf("初始化SD卡失败，手动的");
	}
	
	FRESULT flag1=f_mount(&fs,"0:",1);
	if(flag1!=FR_OK)
	{
		printf("文件系统挂载错误，接下来进行格式化及重挂载操作，标志flag1,%d\n",flag1);
		
		FRESULT flag2=f_mkfs("0:",&format_opt,work_buffer,sizeof(work_buffer));
		if(flag2!=FR_OK)
		{
			printf("SD卡格式化失败");
		}
		else
		{
			printf("SD卡格式化成功，重挂载启动");
			f_mount(NULL,"0:",1);//立刻去挂载
			f_mount(&fs,"0:",1);//立即挂载
		}
		
	}
	else
	{
		printf("文件系统挂载成功，标志flag1,%d\n",flag1);
		
	}
//		/*文件试创建*/
//				FRESULT flag2=f_open(&fp,"0:huihui.txt",FA_CREATE_ALWAYS|FA_WRITE|FA_READ);
//				if(flag2!=FR_OK)
//				{
//					printf("文件打开失败\n");
//				}
//				else
//				{
//						printf("文件打开成功%d\n",flag2);	
//				}


//	
//	/*文件试写入*/
//	UINT count_write=0;
//	FRESULT flag3=f_write(&fp,Memory,4096*2,&count_write);
//	if(flag3!=FR_OK)
//	{
//				printf("数据写入失败，错误码%d\n",flag3);
//	}
//	else
//	{
//				printf("数据写入成功,%d\n",count_write);		
//	}
//	
//	/*文件试读出*/
//	UINT count_read=0;
//	f_lseek(&fp,0);
//	FRESULT flag4=f_read(&fp,Memory2,4096*2,&count_read);
//	if(flag4!=FR_OK)
//	{
//				printf("数据读出失败，错误码%d\n",flag4);
//	}
//	else
//	{
//				printf("数据读出成功,%d\n",count_read);		
//	}
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



