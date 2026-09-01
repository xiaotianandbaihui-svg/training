#include <stm32f4xx.h>
#include "stm32f4xx_adc.h"
#include "stm32f4xx_rcc.h"
#include "lcd.h"
#include "My_Flash.h"
#include "key.h"
#include "My_ADC_DMA.h"
#include "sdio_sdcard.h"  
#include "ff.h"
#include "usart.h"

//基地址

//__IO uint16_t* Sub_Base=(__IO uint16_t*)0x08010000;
//__IO uint16_t *Base =(__IO uint16_t*)0x08010000;
extern FIL fp;
__attribute__ ((aligned(8))) uint8_t a[2*DATA_BLOCK]={0};
__attribute__ ((aligned(8))) uint8_t a2[2*DATA_BLOCK]={0};



uint32_t timer=0;//系统时间
uint32_t time_for_record=0;//记录录音录了多久
uint8_t flag=0;	//判断一秒结束，转存数据
uint8_t state=0;//判断是录音还是播放状态还是空闲状态。
	uint8_t key=0;
uint8_t flag_suspend_key=0;//按钮点击状态
uint8_t is_have=0;//判断是否存在一段录音了
extern uint16_t Memory[DATA_BLOCK];
extern uint16_t Memory2[DATA_BLOCK];
uint8_t swap=0;

extern uint32_t error_value;
uint32_t times=0;//更改次数
uint8_t counter=0;

void Time_Init()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);
}

void My_GPIO_ADC_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AN;
//	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	
	GPIO_Init(GPIOC,&GPIO_InitStructure);
}

void ADC1_Init()
{
	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_DeInit();
	ADC_CommonInitStructure.ADC_DMAAccessMode=ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_Mode=ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler=ADC_Prescaler_Div4;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay=ADC_TwoSamplingDelay_10Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);
	
	ADC_InitStructure.ADC_ContinuousConvMode=DISABLE;
	ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_T8_TRGO;
	ADC_InitStructure.ADC_NbrOfConversion=1;
	ADC_InitStructure.ADC_Resolution=ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode=DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge=ADC_ExternalTrigConvEdge_Rising;
	ADC_Init(ADC1,&ADC_InitStructure);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_12,1,ADC_SampleTime_480Cycles);//菜单1

	ADC_DMARequestAfterLastTransferCmd(ADC1,ENABLE);
	ADC_DMACmd(ADC1,ENABLE);
	ADC_Cmd(ADC1,ENABLE);
}


void DMA_ADC1_Init(uint32_t Memory)
{
	DMA_InitTypeDef DMA_InitSturcture;
	DMA_InitSturcture.DMA_BufferSize=DATA_BLOCK;
	DMA_InitSturcture.DMA_Channel=DMA_Channel_0;
	DMA_InitSturcture.DMA_DIR=DMA_DIR_PeripheralToMemory;
	DMA_InitSturcture.DMA_FIFOMode=DMA_FIFOMode_Enable;
	DMA_InitSturcture.DMA_FIFOThreshold=DMA_FIFOThreshold_HalfFull;
	DMA_InitSturcture.DMA_Memory0BaseAddr=(uint32_t) Memory;
	DMA_InitSturcture.DMA_MemoryBurst=DMA_MemoryBurst_INC4;
	DMA_InitSturcture.DMA_MemoryDataSize=DMA_MemoryDataSize_HalfWord;
	DMA_InitSturcture.DMA_MemoryInc=DMA_MemoryInc_Enable;
	DMA_InitSturcture.DMA_Mode=DMA_Mode_Circular;
	DMA_InitSturcture.DMA_PeripheralBaseAddr=(uint32_t) (&ADC1->DR);
	DMA_InitSturcture.DMA_PeripheralBurst=DMA_PeripheralBurst_Single;
	DMA_InitSturcture.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord;
	DMA_InitSturcture.DMA_PeripheralInc=DMA_PeripheralInc_Disable;
	DMA_InitSturcture.DMA_Priority=DMA_Priority_VeryHigh;
	DMA_Init(DMA2_Stream0,&DMA_InitSturcture);
	DMA_ITConfig(DMA2_Stream0,DMA_IT_TC,ENABLE);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=DMA2_Stream0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=2;
	NVIC_Init(&NVIC_InitStructure);
	
	
	DMA_DoubleBufferModeConfig(DMA2_Stream0,(uint32_t)Memory2,DMA_Memory_0);
	DMA_DoubleBufferModeCmd(DMA2_Stream0,ENABLE);
	
	DMA_Cmd(DMA2_Stream0,ENABLE);
}

void DMA2_Stream0_IRQHandler()
{
	if(DMA_GetITStatus(DMA2_Stream0,DMA_IT_TCIF0)==SET)
	{
		if(key==0)
		{
			timer++;
			flag=1;
		}			
		if(timer==Record_Time)
		{	

		time_for_record=Record_Time;
			TIM_Cmd(TIM8,DISABLE);
			state=0;	

		}			
			if(swap==0)
				swap=1;
			else
				swap=0;

		DMA_ClearITPendingBit(DMA2_Stream0,DMA_IT_TCIF0);
	}
}
uint32_t show(uint8_t i)
{
			uint32_t temp=1;
	for(int j=0;j<i;j++)
	{
		temp*=10;
	}
	return temp;
}
void Re_Init()
{
	if(flag==1)
	{
			
		LCD_ShowNum(0,0,timer*100*DATA_BLOCK/1000000,5,16);
//		LCD_ShowString(3*8,0,8*6,16,16,".");
		LCD_ShowChar(24+24,0,'.',16,0);
		for(int i=0;i<6;i++)
		{
					LCD_ShowNum((4+3+i)*(8),0,((timer*100*DATA_BLOCK)%show(6-i))/show(6-i-1),1,16);
		}

		flag=0;
		if(swap==1)
		{
			DWORD file_size= f_size(&fp);
			f_lseek(&fp,file_size);
				UINT count_write=0;
				FRESULT flag3=f_write(&fp,Memory,DATA_BLOCK*2,&count_write);
		}
		if(swap==0)
		{
			DWORD file_size2= f_size(&fp);
			f_lseek(&fp,file_size2);
			UINT count_write2=0;
			FRESULT flag4=f_write(&fp,Memory2,DATA_BLOCK*2,&count_write2);
		}
		if(timer==0)
		{
			TIM_Cmd(TIM8,DISABLE);
//			Base=Base-(DATA_BLOCK*Record_Time);
		}
		else
		{			
		}		
		if(timer==Record_Time)
		{
			timer=0;
			f_close(&fp);
//			fp=NULL;
		}

	}
}
void TIM8_Init_For_ADC()
{
	TIM_InternalClockConfig(TIM8);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV4;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period=100-1;
	TIM_TimeBaseInitStructure.TIM_Prescaler=168-1;//20Khz
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM8,&TIM_TimeBaseInitStructure);
	TIM_SelectOutputTrigger(TIM8,TIM_TRGOSource_Update);
	TIM_Cmd(TIM8,DISABLE);
}

void Init_ADC_DMA_Part(uint32_t m)
{
	Time_Init();
	My_GPIO_ADC_Init();
	DMA_ADC1_Init(m);
	ADC1_Init();
	TIM8_Init_For_ADC();
}

void Record_Suspend_Start()
{
	key=KEY_Scan(0);
	if(key==1)
	{
		if(state==1)//在录音状态点一下暂停
		{
			if(flag_suspend_key==0)
			{
				TIM_Cmd(TIM8,DISABLE);
				flag_suspend_key=1;
			}
			else
			{
				TIM_Cmd(TIM8,ENABLE);
				flag_suspend_key=0;
			}
				
		}
	}
	else if(key==2)
	{
		if(state==1)//在录音状态下按下key1，直接结束录音 
		{
			TIM_Cmd(TIM8,DISABLE);
			TIM8->CNT=0;
			DMA_Cmd(DMA2_Stream0,DISABLE);
			DMA2_Stream0->NDTR=DATA_BLOCK;
			DMA_Cmd(DMA2_Stream0,ENABLE);
			state=0;//状态变为空闲状态
			time_for_record=timer;//记录录音时间。
			timer=0;
//			Base=Base-DATA_BLOCK*time_for_record;
			f_close(&fp);
//			fp=NULL;
									LCD_ShowNum(0,148,time_for_record,5,16);
		}
	}
	else if(key==3)
	{
		if(state==0)
		{								
	/*文件试创建*/
			FRESULT flag2=f_open(&fp,"0:voice5.txt",FA_OPEN_ALWAYS|FA_WRITE|FA_READ);
//				if(flag2!=FR_OK)
//				{
//					printf("文件打开失败\n");
//				}
//				else
//				{
//						printf("文件打开成功%d\n",flag2);	
//				}

			state=1;
			if(is_have==1)
			{			

				LCD_ShowString(0,30,8*3,16,16,"PRE");
	

				f_lseek(&fp,0);			

				LCD_ShowString(0,30,8*3,16,16,"OKL");
			}
			is_have=1;		
			TIM_Cmd(TIM8,ENABLE);

		}
	}
}












void DAC_Init_()
 {
	 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC,ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);

	 DAC_InitTypeDef DAC_InitStructure;
	 DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude=DAC_LFSRUnmask_Bit0;
	 DAC_InitStructure.DAC_OutputBuffer=DAC_OutputBuffer_Disable;
	 DAC_InitStructure.DAC_Trigger=DAC_Trigger_T4_TRGO;
	 DAC_InitStructure.DAC_WaveGeneration=DAC_WaveGeneration_None;
	 
	 DAC_Init(DAC_Channel_1,&DAC_InitStructure);
	 
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AN;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
		
		GPIO_Init(GPIOA,&GPIO_InitStructure);
	 
	 DAC_Cmd(DAC_Channel_1,ENABLE);
	 DAC_DMACmd(DAC_Channel_1,ENABLE);
	 DAC_SetChannel1Data(DAC_Align_12b_R,0);
 }
void DAC_DAM_Init()
{
	    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

	DMA_Cmd(DMA1_Stream5,DISABLE);
		  DMA_InitTypeDef            DMA_InitStructure;
     
    DMA_StructInit( &DMA_InitStructure);
    DMA_InitStructure.DMA_Channel = DMA_Channel_7;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&DAC->DHR12R1; 
		DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)a;	
		DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;		
		DMA_InitStructure.DMA_BufferSize = DATA_BLOCK;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
DMA_InitStructure.DMA_MemoryBurst=DMA_MemoryBurst_INC4;
		DMA_Init(DMA1_Stream5, &DMA_InitStructure);
		
		NVIC_InitTypeDef NVIC_InitStructure;
		NVIC_InitStructure.NVIC_IRQChannel=DMA1_Stream5_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority=2;
		NVIC_Init(&NVIC_InitStructure);

		DMA_ClearFlag(DMA1_Stream5,DMA_FLAG_TCIF5);
		DMA_ITConfig(DMA1_Stream5,DMA_IT_TC,ENABLE);
	DMA_DoubleBufferModeConfig(DMA1_Stream5,(uint32_t)a2,DMA_Memory_0);
	DMA_DoubleBufferModeCmd(DMA1_Stream5,ENABLE);
		DMA_Cmd(DMA1_Stream5, ENABLE);
}
uint8_t swap_dac=0;
uint32_t byte=0;
void DMA1_Stream5_IRQHandler()
{
	if(DMA_GetITStatus(DMA1_Stream5,DMA_IT_TCIF5)==SET)
	{
		times++;		
		if(times<=(time_for_record-1))
		{
				if(swap_dac==0)
				{
					swap_dac=1;
					f_lseek(&fp,times*DATA_BLOCK*2);
					UINT count_read2=0;
					FRESULT flag5=f_read(&fp,a2,DATA_BLOCK*2,&count_read2);
				}
				else
				{
					swap_dac=0;
					f_lseek(&fp,times*DATA_BLOCK*2);
					UINT count_read3=0;
					FRESULT flag5=f_read(&fp,a,DATA_BLOCK*2,&count_read3);
				
				}
		}
		LCD_ShowNum(0,300,times,2,16);
		if(times>=time_for_record)
		{
			times=0;			
			LCD_ShowNum(0,100,time_for_record,5,16);
			swap_dac=0;
			DAC_DAM_Init();
			state=0;
			TIM4->CNT=0;
			TIM_Cmd(TIM4,DISABLE);
			f_close(&fp);
		}
		DMA_ClearITPendingBit(DMA1_Stream5,DMA_IT_TCIF5);
	}
}

void TIM_Init_For_DAC()
{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);

	TIM_InternalClockConfig(TIM4);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period=100-1;
	TIM_TimeBaseInitStructure.TIM_Prescaler=84-1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);
	
	TIM_SelectOutputTrigger(TIM4,TIM_TRGOSource_Update);
	
	TIM_Cmd(TIM4,DISABLE);
}

void DAC_Part_Init()
{
	DAC_DAM_Init();
	DAC_Init_();
	TIM_Init_For_DAC();
}


void Key_For_DAC()
{
	if(key==1)
	{
		if(state==0)
		{
			if(is_have==1)
			{
//			time_for_record=600;
				FRESULT flag2=f_open(&fp,"0:voice5.txt",FA_READ);
			printf("%d\n",flag2);
				UINT count_read=0;
				f_lseek(&fp,0);
				FRESULT flag4=f_read(&fp,a,DATA_BLOCK*2,&count_read);
				state=2;
				TIM_Cmd(TIM4,ENABLE);
			}
		}
	}
}
