#ifndef ADC_DMA_H
#define ADC_DMA_H
#include <stm32f4xx.h>
#define DATA_BLOCK 		2048
#define Record_Time 	100000		//总录音时长在此处2h+

void Init_ADC_DMA_Part(uint32_t m);
void Re_Init(void);
void Record_Suspend_Start(void);
void DAC_Part_Init(void);
void Key_For_DAC(void);
#endif
