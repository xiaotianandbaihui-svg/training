#ifndef MY_STACK_H
#define MY_STACK_H
#include <stm32f4xx.h>
typedef struct 
{
	uint8_t dir;//1:直行，2:左转，3:右转,4:原地掉头
	int16_t length;//走的距离
}My_Operation_Block;


#define First_Step_Mode_1 85
#define Second_Step_Mode_1 35
#define Third_Step_Mode_1 35

#define First_Step_Mode_2 145
#define Second_Step_Mode_2 30
#define Three_Step_Mode_2 38
#define Four_Step_Mode_2 38
#define Five_Step_Mode_2 30
#define Six_Step_Mode_2 64
#define Seven_Step_Mode_2 64
#define Eight_Step_Mode_2 29
#define Nine_Step_Mode_2 29
#define Ten_Step_Mode_2 33
#define Eleven_Step_Mode_2 33
#define Twelve_Step_Mode_2 33
#define Thirteen_Step_Mode_2 33
#define Fourteen_Step_Mode_2 63
#define pi 3.1415926



void Init_Stack(void);
void Push(uint8_t dir,int16_t length);
void POP(void);
void Reset_Map_1(void);
void Reset_Map_2(void);

#endif

