#ifndef __MOTOR_H
#define __MOTOR_H



//定义PID的结构体
//内环PID为角速度环 用PI控制
//外环PID为角度环   用PD控制

typedef struct 
{
	float Target;  //目标值   角度的目标值自己是给的 角速度的目标值为角度环的输出
	float Actual;  //检测到的实际值
	float Out;     //输出值
	
	float Err0;    //当前误差
	float Err1;    //上一次误差
	float ErrInt;  //误差积分
	
	float ErrIntThreshold;  //用于积分分离 防止积分离谱数据
	float ErrIntMax;        //用于积分限幅
	
	
	
	float Kp;
	float Ki;
	float Kd;
	
	float OutMax;   //输出最大值
	float OutMin;   //由于这里不涉及到速度的正反转，只用PWM来表示，所以不用考虑最小值,可能也需要
	                //对称的两个风扇的转动相当于轮子的正反转
	                //角度和角速度也有正有负
} PID_t;


//要定义四个结构体变量
//俯仰角的角度和角速度
//翻滚角的角度和角速度


void PID_TIM_Init(void);
void PID_Update(PID_t *p);
void PID_Clear(PID_t *p);

void test_self_angularSpeed(void);  //角速度 零偏校准
void test_self_angle(void);         //角度   零漂校准
void Get_Gyro(void);                 //获取角速度

void Recieve_And_Send_IT(void);


#endif











