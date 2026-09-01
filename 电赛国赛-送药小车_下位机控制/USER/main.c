#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"
#include "lcd.h"
#include "key.h"  
#include "touch.h" 
#include "pid.h"
#include "Encoder.h"
#include "pwm.h"
#include "Op_CORE.h"
#include "My_Stack.h"
#include "exti.h"

struct STATE_PACK_1
{
	uint8_t GO_Straight;/*走直线标志*/
	uint8_t Turn_90;/*转弯90度*/
	uint8_t Turn_180;/*原地掉头*/
	uint8_t Turn_Success;/*转弯或者掉头成功*/
	uint8_t Arrive_Target;/*到达目标位置*/
	uint8_t Spin_Success;/*掉头成功*/
};
extern struct STATE_PACK_1	state_pack;


u16 pwm_oc[4]={0,0,0,0};//初始化四路OC
int16_t pwm[4]={0,0,0,0};
extern int16_t temp5;
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200
	//uart_init_2(115200);
	
	LED_Init();					//初始化LED 
 	LCD_Init();					//LCD初始化 
	KEY_Init(); 				//按键初始化  
	//tp_dev.init();				//触摸屏初始化
	
	TIM8_PWM_Init_(1000-1,168-1,pwm_oc);
	TIM5_PWM_Init_(1000-1,84-1,pwm_oc);
	Related_GPIO_Pin_Init();
  	TIM3_Encoder_Init(); 
  	TIM4_Encoder_Init();
	TIM_PID_Others_Init();
	Foward_Back_Control_GPIO_Init();
	Reset_Map_2();
	Reset_Map_1();
	Init_Stack();
	Item_Detect_GPIO_Init();
	EXTIX_Init();//给按钮的
	
//		Restart_PID();
		//delay_ms(2000);//等待摄像头初始化
	//state_pack.GO_Straight=1;
	//Foward_Back(pwm);
	printf("2");
	while(1)
	{ 
		//LCD_ShowNum(1,1,2,3,16);

		Item_Detect(); 
		State_Judge();
		Back_To_Souce();
//		u8 temp=0;
//		temp=KEY_Scan(0);
//		if(temp==1)
//			Go_Straight_(150);
	//		temp5+=30;
	//	else if(temp==2)
	//	temp5-=30;

	}
}


