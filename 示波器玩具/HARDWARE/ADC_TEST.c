#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_adc.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_dma.h"
#include "ADC_TEST.h"
#include <stdlib.h>
#include <stdio.h>
#include "lcd.h"

uint16_t data1[BUFFER]={0};
uint16_t data2[BUFFER*2]={0};
uint8_t flag_dma_it_tc =0;
uint16_t* Data=data1;
/*数据处理使用的*/
uint16_t max=0;
uint16_t min=4095;
uint16_t last_max=0;
uint16_t last_min=4095;
uint16_t sub_max=0;
uint16_t sub_min=4095;
uint32_t middle=0;
uint16_t flag_enter_max=0;
uint16_t flag_enter_min=0;
uint16_t flag_exit_max=0;
uint16_t flag_exit_min=0;
uint16_t flag_enter_middle=0;
uint16_t flag_first_max_p=0;
uint16_t flag_first_min_p=0;
uint16_t flag_first_middle_p=0;
uint32_t half_T=0;
uint32_t T=0;
uint32_t f=0;
uint8_t Wave_Shape=0;
uint16_t Wave_Shape_Triangle_P=0;
uint16_t Wave_Shape_Rectangle_P=0;
uint16_t Wave_Shape_Sin_P=0;
uint64_t sum=0;


uint16_t time_tim1=0;


uint16_t flag_first_time=0;
uint16_t flag_first_i=0;
uint16_t flag_start_pos_i=0;
uint16_t flag_stop_pos_i=0;
/*先做个单通道的*/
//void MyADC_Init()
//{
//	ADC_CommonInitTypeDef ADC_CommonInitStruct;
//	GPIO_InitTypeDef GPIO_InitStruct;
//	ADC_InitTypeDef ADC_InitStruct;
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);//打开ADC1
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);
//	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AN;
//	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
//	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_2;
//	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
//	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;
//	GPIO_Init(GPIOC,&GPIO_InitStruct);
//	
//	ADC_DeInit();
//	ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;
//  ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div4;
//  ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;//多ADC才
//  ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//多ADC才用得到
//  ADC_CommonInit(&ADC_CommonInitStruct);

//  ADC_StructInit(&ADC_InitStruct);
//  ADC_InitStruct.ADC_ScanConvMode = DISABLE;
//  ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
//  ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_RisingFalling;
//	/*TRGO上升沿触发一次转化，触发转化后可认为立即输出一个数值
//	*但是根据实验，或许要改为双边沿触发，到时候试试看吧
//  */
//	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
//  ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
//  ADC_InitStruct.ADC_NbrOfConversion = 1;
//	
//  ADC_Init(ADC1,&ADC_InitStruct);
//  
//	
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_144Cycles);

//	//这里顺序还是蛮重要的
//  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
//  ADC_DMACmd(ADC1, ENABLE);
//  ADC_Cmd(ADC1, ENABLE);
//}
///*初始化先调用这个函数*/
//void MyDMA_Init(void)
//{
//	DMA_InitTypeDef DMA_InitStruct;
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);
//	
//	DMA_InitStruct.DMA_Channel = DMA_Channel_0;
//  DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)(&ADC1->DR);
//  DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)(Data);
//  DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
//  DMA_InitStruct.DMA_BufferSize = BUFFER;
//  DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//  DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
//  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
//  DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
//	
//  DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
//	
//  DMA_InitStruct.DMA_Priority = DMA_Priority_Low;
//  DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
//  DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
//  DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
//  DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
//  DMA_Init(DMA2_Stream0, &DMA_InitStruct);

//  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
//	NVIC_InitTypeDef NVIC_InitStructure;
//  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
//	DMA_Cmd(DMA2_Stream0, ENABLE);
//}
void ADC_Config()
{
  GPIO_InitTypeDef GPIO_InitStruct;
  ADC_InitTypeDef ADC_InitStruct;
  ADC_CommonInitTypeDef ADC_CommonInitStruct;
  DMA_InitTypeDef DMA_InitStruct;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
  ADC_DeInit();
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
  
  DMA_InitStruct.DMA_Channel = DMA_Channel_0;
  DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)(&ADC1->DR);
  DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)(Data);
  DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStruct.DMA_BufferSize = BUFFER;
  DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	
  DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	
  DMA_InitStruct.DMA_Priority = DMA_Priority_Low;
  DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStruct);

  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
	NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_2 ;
  GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* 设置ADC工作在独立模式 */
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);

  ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div4;
  ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStruct);

  ADC_StructInit(&ADC_InitStruct);
  ADC_InitStruct.ADC_ScanConvMode = DISABLE;
  ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_RisingFalling;
  ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T8_TRGO;
  ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStruct.ADC_NbrOfConversion = 1;
	
  ADC_Init(ADC1,&ADC_InitStruct);
  
  ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_15Cycles);

//这里顺序还是蛮重要的  
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
  ADC_DMACmd(ADC1, ENABLE);

  DMA_Cmd(DMA2_Stream0, ENABLE);
  ADC_Cmd(ADC1, ENABLE);
}

void My_TIM_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;//初始化时基单元
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	TIM_InternalClockConfig(TIM8);
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period=Sample_Time-1;//20kHz的采样频率
	TIM_TimeBaseInitStructure.TIM_Prescaler=PSC_Value-1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM8,&TIM_TimeBaseInitStructure);
	TIM_InternalClockConfig(TIM1);
	TIM_TimeBaseInitStructure.TIM_Period=65536-1;
	TIM_TimeBaseInitStructure.TIM_Prescaler=168-1;
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStructure);
	TIM_SetCounter(TIM1,0);
	TIM_SelectOutputTrigger(TIM8,TIM_TRGOSource_Update);//更新事件为输出触发
	
	TIM_Cmd(TIM8,ENABLE);
	TIM_Cmd(TIM1,DISABLE);
}
uint8_t flag=0;
void DMA2_Stream0_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_Stream0,DMA_IT_TCIF0)==SET)
	{		
		TIM_Cmd(TIM8,DISABLE);//停止触发
		TIM_SetCounter(TIM8,0);
		flag_dma_it_tc=1;
//		if(flag==0)
//		{
//			TIM_Cmd(TIM1,ENABLE);
//			flag=1;
//		}
//		else
//		{
//			TIM_Cmd(TIM1,DISABLE);
//			time_tim1=TIM1->CNT;
//			LCD_ShowNum(0,200,time_tim1,5,12);
//			TIM1->CNT=0;
//			flag=0;
//		}
//		TIM_Cmd(TIM8,ENABLE);
		DMA_ClearITPendingBit(DMA2_Stream0,DMA_IT_TCIF0);
	}
}
void Data_Process()
{
	if(flag_dma_it_tc==1)
	{
		flag_dma_it_tc=0;//恢复标志位
		/*本循环为滤波算法,应该可以算得上是限幅均值滤波*/
		for(int i=0;i<BUFFER;i++)
		{
			if(Data[i]<=min)
				min=Data[i];
			if(Data[i]>=max)
				max=Data[i];//选取最大最小值
			//最大最小值最好在滤波前测出
			
			
				if(i < BUFFER - 1 && abs(Data[i] - Data[i + 1]) < A)//限幅滤波
				{
            Data[i] = Data[i + 1];  // 替换当前数据为下一个数据
        }		
				if(i<BUFFER-1&&abs(Data[i]-Data[i+1])<abs(last_max-last_min)/2.0)//均值滤波，同时防止他对方波的影响，均值滤波会使得方波变形
				{
					if(i<BUFFER-N+1)
					{
						sum=0;
						for(int j=0;j<N;j++)
						{
							sum+=Data[i+j];
						}
						Data[i]=sum/N;
					}
				}
				if(Data[i]<=sub_min)
				{
					sub_min=Data[i];
				}
				if(Data[i]>=sub_max)
				{
					sub_max=Data[i];
				}
		}
		middle=(max+min)/2.0;
		last_max=sub_max;
		last_min=sub_min;//记录上一次的最大最小值
		LCD_ShowNum(0,24,max*3300/4095,5,12);
		LCD_ShowNum(0,48,min*3300/4095,5,12);
		
		for(int i=0;i<BUFFER;i++)
		{
			if(abs(max-Data[i])<1000)
			{
				flag_enter_max++;//用于波形识别
				flag_exit_max++;
				if(flag_exit_max==1)
				{
					flag_first_max_p++;
				}
				if(flag_exit_min!=0)
				{
					flag_exit_min=0;
				}
			}
			if(abs(Data[i]-min)<1000)
			{
				flag_enter_min++;
				flag_exit_min++;
				if(flag_exit_min==1)
				{
					flag_first_min_p++;
				}
				if(flag_exit_max!=0)
				{
					flag_exit_max=0;
				}
			}
//		 if(abs(Data[i]-middle)<800)
//			{
//				flag_first_middle_p++;
//				if(flag_enter_middle==1)
//				{
//					flag_first_middle_p=i;
//				}
//			}			
			printf("%f\n",Data[i]*3.3/4095);//printf函数十分消耗时间，可能会存在问题

			
//			LCD_ShowNum(0,100,flag_enter_max,5,12);
//			LCD_ShowNum(0,112,flag_enter_min,5,12);
//			LCD_ShowNum(0,124,flag_enter_middle,5,12);
//			LCD_ShowNum(0,148,middle,5,12);

			/*我这么写的目的主要是为了保证截取到一个完整的半周期再做数据处理*/
			/*前面三个都不等于0，说明了我们现在已经截取到一整个周期了*/
			/*后面两个个都大于50，说明已经出了判别区（最大最小值区）现在可以做数据处理了*/
//			if(flag_enter_max!=0&&flag_enter_middle!=0&&flag_enter_min!=0)//&&abs(Data[i]-max)>50&&abs(Data[i]-min)>50
//			{
//						flag_first_max_p=(flag_first_max_p+flag_enter_max-1+flag_first_max_p)/2.0;
//						flag_first_min_p=(flag_first_min_p+flag_enter_min-1+flag_first_min_p)/2.0;//获取下标位置
////						flag_first_middle_p=(flag_first_middle_p+flag_enter_middle-1+flag_first_middle_p)/2.0;//获取下标位置(中点)
//			}
		}
//		if(abs(Data[(flag_first_max_p+flag_first_middle_p)/2]-(max+middle)/2)<300)
//		{
//			Wave_Shape_Triangle_P++;
//		}
//		else if((abs(Data[(flag_first_max_p+flag_first_middle_p)/2]-(max+middle)/2))<800)
//		{
//			Wave_Shape_Sin_P++;
//		}
//		else
//		{
//			Wave_Shape_Rectangle_P++;
//		}
//		half_T=abs(flag_first_max_p-flag_first_min_p)*Sample_Time;//单位为微秒
//		T=2*half_T;
//		f=100000.0/T;//单位为Hz
		LCD_ShowNum(0,0,1000000/((8191.0/(flag_first_max_p-1))),6,12);//通过定时器计算准确的时间
		/*恢复初始化，为下次做准备*/
		flag_first_max_p=0;//计算频率
		flag_first_min_p=0;
		flag_exit_max=0;
		flag_exit_min=0;
//		half_T=0;
//		T=0;
//		f=0;

		/*初始化*/
		max=0;
		min=4095;
		sub_max=0;
		sub_min=4095;
		/*此部分是波形识别*/
//		if(Wave_Shape_Rectangle_P>Wave_Shape_Sin_P&&Wave_Shape_Rectangle_P>Wave_Shape_Triangle_P)
//		{
//			Wave_Shape=1;
//		}
//		else if(Wave_Shape_Sin_P>Wave_Shape_Rectangle_P&&Wave_Shape_Sin_P>Wave_Shape_Triangle_P)
//		{
//			Wave_Shape=2;
//		}
//		else if(Wave_Shape_Triangle_P>Wave_Shape_Sin_P&&Wave_Shape_Triangle_P>Wave_Shape_Rectangle_P)
//		{
//			Wave_Shape=3;
//		}
		if(abs(BUFFER-flag_enter_max-flag_enter_min)<10)
		{
			LCD_ShowNum(0,100,2,1,12);
		}
		else if(abs(2200-flag_enter_max-flag_enter_min)<100)
		{
			LCD_ShowNum(0,100,3,1,12);
		}
		else if(abs(2800-flag_enter_max-flag_enter_min)<500)//以上为经验值实测
		{
			LCD_ShowNum(0,100,5,1,12);
		}
		LCD_ShowNum(0,80,flag_enter_max+flag_enter_min,5,12);//显示波形
		flag_enter_max=0;
		flag_enter_min=0;
		/*我们发现一个细节，就是中间ADC可是没停的，但是无所谓，因为我们输出的是数组里的数据*/
		TIM_Cmd(TIM8,ENABLE);//开启DMA，等待下一个数据段
	}
}





void Data_Process_Two()
{
	if(flag_dma_it_tc==1)
	{
		flag_dma_it_tc=0;//恢复标志位
		/*本循环为滤波算法,应该可以算得上是限幅均值滤波*/
		for(int i=0;i<BUFFER;i++)
		{
			if(Data[i]<=min)
				min=Data[i];
			if(Data[i]>=max)
				max=Data[i];//选取最大最小值
			//最大最小值最好在滤波前测出
			
			
				if(i < BUFFER - 1 && abs(Data[i] - Data[i + 1]) < A)//限幅滤波
				{
            Data[i] = Data[i + 1];  // 替换当前数据为下一个数据
        }		
				if(i<BUFFER-1&&abs(Data[i]-Data[i+1])<abs(last_max-last_min)/2.0)//均值滤波，同时防止他对方波的影响，均值滤波会使得方波变形
				{
					if(i<BUFFER-N+1)
					{
						sum=0;
						for(int j=0;j<N;j++)
						{
							sum+=Data[i+j];
						}
						Data[i]=sum/N;
					}
				}
				if(Data[i]<=sub_min)
				{
					sub_min=Data[i];
				}
				if(Data[i]>=sub_max)
				{
					sub_max=Data[i];
				}
		}
		middle=(max+min)/2.0;
		last_max=sub_max;
		last_min=sub_min;//记录上一次的最大最小值
		LCD_ShowNum(75,12,max*3300/4095,5,12);
		LCD_ShowNum(75,24,min*3300/4095,5,12);
		
		for(int i=0;i<BUFFER;i++)
		{
			if(abs(max-Data[i])<1000)
			{
				flag_enter_max++;//用于波形识别
				flag_exit_max++;
				if(flag_exit_max==1)
				{
					flag_first_max_p++;
					flag_first_i=i;
					flag_stop_pos_i=i;
							if(flag_first_time==0)
							{
								flag_start_pos_i=flag_first_i;//(flag_first_i+flag_first_i+flag_exit_max-1)/2;//(flag_exit_max*flag_first_i+flag_exit_max-1)/flag_exit_max;//取中间值，提高准确度
								flag_first_time=1;
							}
					if(flag_exit_min!=0)
					{
						flag_exit_min=0;
					}
				}
			}
			if(abs(Data[i]-min)<1000)
			{
				flag_enter_min++;
				flag_exit_min++;
				if(flag_exit_min==1)
				{
					flag_first_min_p++;
						if(flag_exit_max!=0)
						{
//							if(flag_first_time==0)
//							{
//								flag_start_pos_i=flag_first_i;//(flag_first_i+flag_first_i+flag_exit_max-1)/2;//(flag_exit_max*flag_first_i+flag_exit_max-1)/flag_exit_max;//取中间值，提高准确度
//								flag_first_time=1;
//							}
							flag_exit_max=0;
						}

				}
			}
//			if(abs(Data[i]-max)>1000&&abs(Data[i]-min)>1000&&flag_exit_max!=0)
//			{
////				for(int j=0;j<flag_exit_max;j++)
////				{
////					
////				}
//				flag_stop_pos_i=flag_first_i;//(flag_first_i+flag_first_i+flag_exit_max-1)/2;//(flag_exit_max*flag_first_i+flag_exit_max-1)/flag_exit_max;
//			}
//		 if(abs(Data[i]-middle)<800)
//			{
//				flag_first_middle_p++;
//				if(flag_enter_middle==1)
//				{
//					flag_first_middle_p=i;
//				}
//			}			
			printf("%f\n",Data[i]*3.3/4095);//printf函数十分消耗时间，可能会存在问题

			
//			LCD_ShowNum(0,100,flag_enter_max,5,12);
//			LCD_ShowNum(0,112,flag_enter_min,5,12);
//			LCD_ShowNum(0,124,flag_enter_middle,5,12);
//			LCD_ShowNum(0,148,middle,5,12);

			/*我这么写的目的主要是为了保证截取到一个完整的半周期再做数据处理*/
			/*前面三个都不等于0，说明了我们现在已经截取到一整个周期了*/
			/*后面两个个都大于50，说明已经出了判别区（最大最小值区）现在可以做数据处理了*/
//			if(flag_enter_max!=0&&flag_enter_middle!=0&&flag_enter_min!=0)//&&abs(Data[i]-max)>50&&abs(Data[i]-min)>50
//			{
//						flag_first_max_p=(flag_first_max_p+flag_enter_max-1+flag_first_max_p)/2.0;
//						flag_first_min_p=(flag_first_min_p+flag_enter_min-1+flag_first_min_p)/2.0;//获取下标位置
////						flag_first_middle_p=(flag_first_middle_p+flag_enter_middle-1+flag_first_middle_p)/2.0;//获取下标位置(中点)
//			}
		}
//		if(abs(Data[(flag_first_max_p+flag_first_middle_p)/2]-(max+middle)/2)<300)
//		{
//			Wave_Shape_Triangle_P++;
//		}
//		else if((abs(Data[(flag_first_max_p+flag_first_middle_p)/2]-(max+middle)/2))<800)
//		{
//			Wave_Shape_Sin_P++;
//		}
//		else
//		{
//			Wave_Shape_Rectangle_P++;
//		}
//		half_T=abs(flag_first_max_p-flag_first_min_p)*Sample_Time;//单位为微秒
//		T=2*half_T;
//		f=100000.0/T;//单位为Hz
		f=1000000/(((8191.0/(BUFFER))*(flag_stop_pos_i-flag_start_pos_i))/(flag_first_max_p-1));
		LCD_ShowNum(75,0,f,5,12);//通过定时器计算准确的时间
		/*恢复初始化，为下次做准备*/
		flag_first_max_p=0;//计算频率
		flag_first_min_p=0;
		flag_exit_max=0;
		flag_exit_min=0;
//		half_T=0;
//		T=0;
//		f=0;

		/*初始化*/
		max=0;
		min=4095;
		sub_max=0;
		sub_min=4095;
		/*此部分是波形识别*/
//		if(Wave_Shape_Rectangle_P>Wave_Shape_Sin_P&&Wave_Shape_Rectangle_P>Wave_Shape_Triangle_P)
//		{
//			Wave_Shape=1;
//		}
//		else if(Wave_Shape_Sin_P>Wave_Shape_Rectangle_P&&Wave_Shape_Sin_P>Wave_Shape_Triangle_P)
//		{
//			Wave_Shape=2;
//		}
//		else if(Wave_Shape_Triangle_P>Wave_Shape_Sin_P&&Wave_Shape_Triangle_P>Wave_Shape_Rectangle_P)
//		{
//			Wave_Shape=3;
//		}
  if(f<=25000)
	{
		if(abs(BUFFER-flag_enter_max-flag_enter_min)<10)
		{
			LCD_ShowString(0,48,36,12,12,"Rec");
		}
		else if(abs(2200-flag_enter_max-flag_enter_min)<100)
		{
			LCD_ShowString(0,48,36,12,12,"Tri");
		}
		else if(abs(2800-flag_enter_max-flag_enter_min)<500)//以上为经验值实测
		{
			LCD_ShowString(0,48,36,12,12,"Sin");
		}
	}
	else if(f>25000)
	{
		if(abs(BUFFER-flag_enter_max-flag_enter_min)<10)
		{
			LCD_ShowString(0,48,36,12,12,"Rec");
		}
//		LCD_ShowNum(0,200,flag_enter_max+flag_enter_min,5,12);
		else if(abs(2800-flag_enter_max-flag_enter_min)<100)
		{
			LCD_ShowString(0,48,36,12,12,"Sin");

		}
		else if(abs(2200-flag_enter_max-flag_enter_min)<200)
		{
			LCD_ShowString(0,48,36,12,12,"Tri");
		}
	}
		flag_enter_max=0;
		flag_enter_min=0;
		/*我们发现一个细节，就是中间ADC可是没停的，但是无所谓，因为我们输出的是数组里的数据*/
		TIM_Cmd(TIM8,ENABLE);//开启DMA，等待下一个数据段
	}
}







/*以下为多通道的部分，涉及配置，尝试,思路和单通道几乎一致*/

void ADC_Config_For_Multi_Channel()
{
  GPIO_InitTypeDef GPIO_InitStruct;
  ADC_InitTypeDef ADC_InitStruct;
  ADC_CommonInitTypeDef ADC_CommonInitStruct;
  DMA_InitTypeDef DMA_InitStruct;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
  ADC_DeInit();
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
  
  DMA_InitStruct.DMA_Channel = DMA_Channel_0;
  DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)(&ADC1->DR);
  DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)(data2);
  DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStruct.DMA_BufferSize = BUFFER*2;//多通道按照我的思路是交错存储的
  DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	
  DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	
  DMA_InitStruct.DMA_Priority = DMA_Priority_Low;
  DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;//一般为4个数据单元
  DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStruct);

  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
	NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_2 ;
  GPIO_Init(GPIOC, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_0;//对应的可能是通道8
  GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* 设置ADC工作在独立模式 */
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);
	
  ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div4;
  ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
  ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStruct);


  ADC_StructInit(&ADC_InitStruct);
  ADC_InitStruct.ADC_ScanConvMode = ENABLE;//由于开启了多通道，所以开启扫描模式
  ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_RisingFalling;
  ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T8_TRGO;//这边统统不用管
  ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStruct.ADC_NbrOfConversion = 2;
	
  ADC_Init(ADC1,&ADC_InitStruct);
  
  ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_15Cycles);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_8,2,ADC_SampleTime_15Cycles);	

//这里顺序还是蛮重要的  
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
  ADC_DMACmd(ADC1, ENABLE);

  DMA_Cmd(DMA2_Stream0, ENABLE);
  ADC_Cmd(ADC1, ENABLE);
}

uint16_t max_channel2=0;
uint16_t min_channel2=0;
uint16_t last_max_channel2=0;
uint16_t last_min_channel2=0;
uint16_t sub_min_channel2=0;
uint16_t sub_max_channel2=0;
uint16_t flag_enter_max_channel2=0;
uint16_t flag_exit_max_channel2=0;
uint16_t flag_enter_min_channel2=0;
uint16_t flag_exit_min_channel2=0;
uint16_t flag_first_max_p_channel2=0;
uint16_t flag_first_i_channel2=0;
uint16_t flag_stop_pos_i_channel2=0;
uint16_t flag_first_time_channel2=0;
uint16_t flag_start_pos_i_channel2=0;
uint16_t flag_first_min_p_channel2=0;
uint16_t f_channel2=0;
void Data_Process_Two_For_Multi_Channel(void)
{
	if(flag_dma_it_tc==1)
	{
		flag_dma_it_tc=0;//恢复标志位
		/*本循环为滤波算法,应该可以算得上是限幅均值滤波*/
		for(int i=0;i<BUFFER;i++)
		{
			if(data2[i*2]<=min)
				min=data2[i*2];
			if(data2[i*2]>=max)
				max=data2[i*2];//选取最大最小值
			//最大最小值最好在滤波前测出
			if(data2[i*2+1]<=min_channel2)
				min_channel2=data2[i*2+1];
			if(data2[i*2+1]>=max_channel2)
				max_channel2=data2[i*2+1];

				if(i < BUFFER - 1 && abs(data2[i*2] - data2[(i + 1)*2]) < A)//限幅滤波
				{
            data2[i*2] = data2[(i + 1)*2];  // 替换当前数据为下一个数据
        }	
				if(i < BUFFER - 1 && abs(data2[i*2+1] - data2[(i + 1)*2+1]) < A)//限幅滤波
				{
						data2[i*2+1] = data2[(i + 1)*2+1];  // 替换当前数据为下一个数据
        }				
				if(i<BUFFER-1)//均值滤波，同时防止他对方波的影响，均值滤波会使得方波变形
				{
					if(abs(data2[i*2]-data2[(i+1)*2])<abs(last_max-last_min)/2.0)
					{
						if(i<BUFFER-N+1)
						{
							sum=0;
							for(int j=0;j<N;j++)
							{
								sum+=data2[(i+j)*2];
							}
							data2[i*2]=sum/N;
						}
					}
					if(abs(data2[i*2+1]-data2[(i+1)*2+1])<abs(last_max_channel2-last_min_channel2)/2.0)
					{
						if(i<BUFFER-N+1)
						{
							sum=0;
							for(int j=0;j<N;j++)
							{
								sum+=data2[(i+j)*2+1];
							}
							data2[i*2+1]=sum/N;
						}
					}
				}
				if(data2[i*2]<=sub_min)
				{
					sub_min=data2[i*2];
				}
				if(data2[i*2]>=sub_max)
				{
					sub_max=data2[i*2];
				}
				if(data2[i*2+1]<=sub_min_channel2)
				{
					sub_min_channel2=data2[i*2+1];
				}
				if(data2[i*2+1]>=sub_max_channel2)
				{
					sub_max_channel2=data2[i*2+1];
				}

		}
//		middle=(max+min)/2.0;
		last_max=sub_max;
		last_min=sub_min;//记录上一次的最大最小值
		last_max_channel2=sub_max_channel2;
		last_min_channel2=sub_min_channel2;
		
		LCD_ShowNum(75,12,max*3300/4095,5,12);
		LCD_ShowNum(75,24,min*3300/4095,5,12);
		LCD_ShowNum(75,36,max_channel2*3300/4095,5,12);
		LCD_ShowNum(75,48,min_channel2*3300/4095,5,12);
		
		for(int i=0;i<BUFFER;i++)
		{
			if(abs(max-data2[i*2])<1000)
			{
				flag_enter_max++;//用于波形识别
				flag_exit_max++;
				if(flag_exit_max==1)
				{
					flag_first_max_p++;
					flag_first_i=i;
					flag_stop_pos_i=i;
					if(flag_first_time==0)
					{
						flag_start_pos_i=flag_first_i;//(flag_first_i+flag_first_i+flag_exit_max-1)/2;//(flag_exit_max*flag_first_i+flag_exit_max-1)/flag_exit_max;//取中间值，提高准确度
						flag_first_time=1;
					}
					if(flag_exit_min!=0)
					{
						flag_exit_min=0;
					}
				}
			}
			if(abs(data2[i*2]-min)<1000)
			{
				flag_enter_min++;
				flag_exit_min++;
				if(flag_exit_min==1)
				{
					flag_first_min_p++;
						if(flag_exit_max!=0)
						{
//							if(flag_first_time==0)
//							{
//								flag_start_pos_i=flag_first_i;//(flag_first_i+flag_first_i+flag_exit_max-1)/2;//(flag_exit_max*flag_first_i+flag_exit_max-1)/flag_exit_max;//取中间值，提高准确度
//								flag_first_time=1;
//							}
							flag_exit_max=0;
						}

				}
			}
			
			
			if(abs(max_channel2-data2[i*2+1])<1000)
			{
				flag_enter_max_channel2++;//用于波形识别
				flag_exit_max_channel2++;
				if(flag_exit_max_channel2==1)
				{
					flag_first_max_p_channel2++;
					flag_first_i_channel2=i;
					flag_stop_pos_i_channel2=i;
					if(flag_first_time_channel2==0)
					{
						flag_start_pos_i_channel2=flag_first_i_channel2;//(flag_first_i+flag_first_i+flag_exit_max-1)/2;//(flag_exit_max*flag_first_i+flag_exit_max-1)/flag_exit_max;//取中间值，提高准确度
						flag_first_time_channel2=1;
					}
					if(flag_exit_min_channel2!=0)
					{
						flag_exit_min_channel2=0;
					}
				}
			}
			if(abs(data2[i*2+1]-min_channel2)<1000)
			{
				flag_enter_min_channel2++;
				flag_exit_min_channel2++;
				if(flag_exit_min_channel2==1)
				{
					flag_first_min_p_channel2++;
						if(flag_exit_max_channel2!=0)
						{
//							if(flag_first_time==0)
//							{
//								flag_start_pos_i=flag_first_i;//(flag_first_i+flag_first_i+flag_exit_max-1)/2;//(flag_exit_max*flag_first_i+flag_exit_max-1)/flag_exit_max;//取中间值，提高准确度
//								flag_first_time=1;
//							}
							flag_exit_max_channel2=0;
						}

				}
			}
			
			
			
			
			
			
			

			
			
//			                    _ooOoo_                         			//
//                         o8888888o                              //
//                         88" . "88                              //
//                         (| ^_^ |)                              //
//                         O\  =  /O                              //
//                      ____/`---'\____                           //
//                    .'  \\|     |//  `.                         //
//                   /  \\|||  :  |||//  \                        //
//                  /  _||||| -:- |||||-  \                       //
//                  |   | \\\  -  /// |   |                       //
//                  | \_|  ''\---/''  |   |                       //
//                  \  .-\__  `-`  ___/-. /                       //
//                ___`. .'  /--.--\  `. . ___                     //
//              ."" '<  `.___\_<|>_/___.'  >'"".                  //
//            | | :  `- \`.;`\ _ /`;.`/ - ` : | |                 //
//            \  \ `-.   \_ __\ /__ _/   .-` /  /                 //
//      ========`-.____`-.___\_____/___.-`____.-'========         //
//                           `=---='                              //
//      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^        //
//       佛祖保佑       学长保佑     上帝保佑      能跑就行     	//
														/*求过|求过*/

			
/**
 * 頂頂頂頂頂頂頂頂頂　頂頂頂頂頂頂頂頂頂
 * 頂頂頂頂頂頂頂　　　　　頂頂　　　　　
 * 　　　頂頂　　　頂頂頂頂頂頂頂頂頂頂頂
 * 　　　頂頂　　　頂頂頂頂頂頂頂頂頂頂頂
 * 　　　頂頂　　　頂頂　　　　　　　頂頂
 * 　　　頂頂　　　頂頂　　頂頂頂　　頂頂
 * 　　　頂頂　　　頂頂　　頂頂頂　　頂頂
 * 　　　頂頂　　　頂頂　　頂頂頂　　頂頂
 * 　　　頂頂　　　頂頂　　頂頂頂　　頂頂
 * 　　　頂頂　　　　　　　頂頂頂　
 * 　　　頂頂　　　　　　頂頂　頂頂　頂頂
 * 　頂頂頂頂　　　頂頂頂頂頂　頂頂頂頂頂
 * 　頂頂頂頂　　　頂頂頂頂　　　頂頂頂頂
 */
			
			
			
			
			
			
//			if(abs(Data[i]-max)>1000&&abs(Data[i]-min)>1000&&flag_exit_max!=0)
//			{
////				for(int j=0;j<flag_exit_max;j++)
////				{
////					
////				}
//				flag_stop_pos_i=flag_first_i;//(flag_first_i+flag_first_i+flag_exit_max-1)/2;//(flag_exit_max*flag_first_i+flag_exit_max-1)/flag_exit_max;
//			}
//		 if(abs(Data[i]-middle)<800)
//			{
//				flag_first_middle_p++;
//				if(flag_enter_middle==1)
//				{
//					flag_first_middle_p=i;
//				}
//			}			
			printf("%f,%f\n",data2[i*2]*3.3/4095,data2[i*2+1]*3.3/4095);//printf函数十分消耗时间，可能会存在问题

			
//			LCD_ShowNum(0,100,flag_enter_max,5,12);
//			LCD_ShowNum(0,112,flag_enter_min,5,12);
//			LCD_ShowNum(0,124,flag_enter_middle,5,12);
//			LCD_ShowNum(0,148,middle,5,12);

			/*我这么写的目的主要是为了保证截取到一个完整的半周期再做数据处理*/
			/*前面三个都不等于0，说明了我们现在已经截取到一整个周期了*/
			/*后面两个个都大于50，说明已经出了判别区（最大最小值区）现在可以做数据处理了*/
//			if(flag_enter_max!=0&&flag_enter_middle!=0&&flag_enter_min!=0)//&&abs(Data[i]-max)>50&&abs(Data[i]-min)>50
//			{
//						flag_first_max_p=(flag_first_max_p+flag_enter_max-1+flag_first_max_p)/2.0;
//						flag_first_min_p=(flag_first_min_p+flag_enter_min-1+flag_first_min_p)/2.0;//获取下标位置
////						flag_first_middle_p=(flag_first_middle_p+flag_enter_middle-1+flag_first_middle_p)/2.0;//获取下标位置(中点)
//			}
		}
//		if(abs(Data[(flag_first_max_p+flag_first_middle_p)/2]-(max+middle)/2)<300)
//		{
//			Wave_Shape_Triangle_P++;
//		}
//		else if((abs(Data[(flag_first_max_p+flag_first_middle_p)/2]-(max+middle)/2))<800)
//		{
//			Wave_Shape_Sin_P++;
//		}
//		else
//		{
//			Wave_Shape_Rectangle_P++;
//		}
//		half_T=abs(flag_first_max_p-flag_first_min_p)*Sample_Time;//单位为微秒
//		T=2*half_T;
//		f=100000.0/T;//单位为Hz
		f=1000000/(((12288.0/(BUFFER*2))*((flag_stop_pos_i-flag_start_pos_i)*2))/(flag_first_max_p-1));
		f_channel2=1000000/(((12288.0/(BUFFER*2))*((flag_stop_pos_i_channel2-flag_start_pos_i_channel2)*2))/(flag_first_max_p_channel2-1));
		LCD_ShowNum(75,0,f,5,12);//通过定时器计算准确的时间
		LCD_ShowNum(75,200,f_channel2,5,12);//通过定时器计算准确的时间
		/*恢复初始化，为下次做准备*/
		flag_first_max_p=0;//计算频率
		flag_first_min_p=0;
		flag_exit_max=0;
		flag_exit_min=0;
		flag_first_max_p_channel2=0;
		flag_first_min_p_channel2=0;
		flag_exit_max_channel2=0;
		flag_exit_min_channel2=0;
//		half_T=0;
//		T=0;
//		f=0;

		/*初始化*/
		max=0;
		max_channel2=0;
		min=4095;
		min_channel2=4095;
		sub_max=0;
		sub_max_channel2=0;
		sub_min=4095;
		sub_min_channel2=4095;
		/*此部分是波形识别*/
//		if(Wave_Shape_Rectangle_P>Wave_Shape_Sin_P&&Wave_Shape_Rectangle_P>Wave_Shape_Triangle_P)
//		{
//			Wave_Shape=1;
//		}
//		else if(Wave_Shape_Sin_P>Wave_Shape_Rectangle_P&&Wave_Shape_Sin_P>Wave_Shape_Triangle_P)
//		{
//			Wave_Shape=2;
//		}
//		else if(Wave_Shape_Triangle_P>Wave_Shape_Sin_P&&Wave_Shape_Triangle_P>Wave_Shape_Rectangle_P)
//		{
//			Wave_Shape=3;
//		}
  if(f<=25000)
	{
		if(abs(BUFFER-flag_enter_max-flag_enter_min)<10)
		{
			LCD_ShowString(0,48,36,12,12,"Rec");
		}
		else if(abs(2200-flag_enter_max-flag_enter_min)<100)
		{
			LCD_ShowString(0,48,36,12,12,"Tri");
		}
		else if(abs(2800-flag_enter_max-flag_enter_min)<500)//以上为经验值实测
		{
			LCD_ShowString(0,48,36,12,12,"Sin");
		}
	}
	else if(f>25000)
	{
		if(abs(BUFFER-flag_enter_max-flag_enter_min)<10)
		{
			LCD_ShowString(0,48,36,12,12,"Rec");
		}
//		LCD_ShowNum(0,200,flag_enter_max+flag_enter_min,5,12);
		else if(abs(2800-flag_enter_max-flag_enter_min)<100)
		{
			LCD_ShowString(0,48,36,12,12,"Sin");

		}
		else if(abs(2200-flag_enter_max-flag_enter_min)<200)
		{
			LCD_ShowString(0,48,36,12,12,"Tri");
		}
	}
	
	 if(f_channel2<=25000)
	{
		if(abs(BUFFER-flag_enter_max_channel2-flag_enter_min_channel2)<10)
		{
			LCD_ShowString(0,300,156,12,12,"Rec_channel2");
		}
		else if(abs(2000-flag_enter_max_channel2-flag_enter_min_channel2)<100)
		{
			LCD_ShowString(0,300,156,12,12,"Tri_channel2");
		}
		else if(abs(2800-flag_enter_max_channel2-flag_enter_min_channel2)<500)//以上为经验值实测
		{
			LCD_ShowString(0,300,156,12,12,"Sin_channel2");
		}
	}
	else if(f_channel2>25000)
	{
		if(abs(BUFFER-flag_enter_max_channel2-flag_enter_min_channel2)<10)
		{
			LCD_ShowString(0,300,156,12,12,"Rec_channel2");
		}
//		LCD_ShowNum(0,200,flag_enter_max+flag_enter_min,5,12);
		else if(abs(2800-flag_enter_max_channel2-flag_enter_min_channel2)<500)
		{
			LCD_ShowString(0,300,156,12,12,"Sin_channel2");
		}
		else if(abs(2200-flag_enter_max_channel2-flag_enter_min_channel2)<200)
		{
			LCD_ShowString(0,300,156,12,12,"Tri_channel2");
		}
					LCD_ShowNum(0,200,flag_enter_max_channel2+flag_enter_min_channel2,5,12);
	}

		flag_enter_max=0;
		flag_enter_max_channel2=0;
		flag_enter_min=0;
		flag_enter_min_channel2=0;
		/*我们发现一个细节，就是中间ADC可是没停的，但是无所谓，因为我们输出的是数组里的数据*/
		TIM_Cmd(TIM8,ENABLE);//开启DMA，等待下一个数据段
	}
}


