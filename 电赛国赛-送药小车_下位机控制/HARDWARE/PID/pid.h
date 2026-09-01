#ifndef PID_H
#define PID_H

void TIM_PID_Others_Init(void);
void Shut_Down_PID(void);
void Restart_PID(void);
void Foward_Back(int16_t* Speed);
#endif
