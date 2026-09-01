#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"

void TIM8_PWM_Init_(u16 arr,u16 psc,u16* oc_count);
void TIM5_PWM_Init_(u16 arr,u16 psc,u16* oc_count);
void Foward_Back(int16_t* Speed);
void Foward_Back_Control_GPIO_Init(void);
/*
*四个电机所用引脚
*PA0
*PA1
*PA2
*PA3
*PC6
*PC7
*PC8
*PC9
*/
#endif
