#ifndef ADC_TEST_H
#define ADC_TEST_H

#define BUFFER 4096
#define A 50 //限幅滤波的幅值限制
#define N 2 //N>1

#define Sample_Time 2//单位微秒
#define PSC_Value 84


//void MyADC_Init(void);
//void MyDMA_Init(void);
void ADC_Config(void);
void My_TIM_Init(void);
void Data_Process(void);
extern uint16_t data1[BUFFER];
extern uint16_t data2[BUFFER*2];
extern uint8_t flag_dma_it_tc;
void Data_Process_Two(void);
void ADC_Config_For_Multi_Channel(void);
void Data_Process_Two_For_Multi_Channel(void);
#endif
