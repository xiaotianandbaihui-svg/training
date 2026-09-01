#ifndef __PID_H
#define __PID_H 	


typedef struct 
{
	float Target;  //目标值   
	float Actual;  //检测到的实际值
	float Out;     //输出值
	
	int	Err0;    //当前误差
	int Err1;    //上一次误差
	int ErrInt;  //误差积分
	int ErrIntThreshold;  //用于积分分离 防止积分离谱数据
	int ErrIntMax;        //用于积分限幅
	
	
	float Kp;
	float Ki;
	float Kd;
	
	int OutMax;   
	int OutMin;   
	                
	                
} PID_t;


void PID_Update(PID_t *p);
void PID_Clear(PID_t *p);

//PID_t car_speed1;
//PID_t car_speed2;

#endif













