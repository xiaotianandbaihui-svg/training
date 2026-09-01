#include <stm32f4xx.h>
#include "pid.h"
#include "My_Stack.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "key.h"
#include <stm32f4xx_gpio.h>
#include "lcd.h"
#include "delay.h"
#include <stdio.h>
#include <stdlib.h>

/*本文件是核心操作控制区*/
#define Turn_Left_Length 31.0//这里理论上，是一个小于0的值
#define Turn_Right_Length 31.0//这里理论上，是一个大于0的值
#define Spin_Length 62.0      //这里正负都行 
typedef enum
{
	Go_Staight=1,
	Go_Left,
	Go_Right,
	Spin
}DIR;
typedef struct node
{
	char straight;/*直行标志*/
	char left;/*左转标志*/
	char right;/*右转标志*/
	char spin;/*调头标志*/
	char camera;/*需要摄像头识别数字*/

	int go_straight_length;/*直行距离*/
	struct node* next_front;/*下一个节点（如果直行）*/
	struct node* next_left;/*下一个节点(如果左转)*/
	struct node* next_right;/*下一个节点（如果右转）*/

	char id;/*记录节点标号*/
}tri_node;

extern My_Operation_Block Stack[20];/*首元素的length记录当前栈中的元素个数*/
extern My_Operation_Block* Stack_Bottom;/*栈底*/
extern My_Operation_Block* Stack_Top;/*栈顶*/
extern tri_node* Pre_2 ;
extern tri_node* Now_2 ;/*这里调节使用那种模式*/

extern tri_node* Pre_1 ;
extern tri_node* Now_1 ;/*这里调节使用那种模式*/

struct STATE_PACK_1
{
	uint8_t GO_Straight;/*走直线标志*/
	uint8_t Turn_90;/*转弯90度*/
	uint8_t Turn_180;/*原地掉头*/
	uint8_t Turn_Success;/*转弯或者掉头成功*/
	uint8_t Arrive_Target;/*到达目标位置*/
	uint8_t Spin_Success;/*掉头成功*/
};
typedef struct
{
	double Location_Kp;
	double Location_Ki;
	double Location_Kd;
	double Location_Error;
	double Location_Last_Error;
	double Location_Integral;
	double Location_Target_Val;
	double Location_Actual_Val;
	double Location_Out;
	
	
	double Location_Out_Limit;
	double Location_Int_Limit;
	/*这里是我们的位置环*/

	
	double Velocity_Kp;
	double Velocity_Ki;
	double Velocity_Kd;
	double Velocity_Error;
	double Velocity_Last_Error;
	double Velocity_Integral;
	double Velocity_Target_Val;
	double Velocity_Actual_Val;

	double Velocity_Out;
	
	double Velocity_Int_Limit;
	double Velocity_Out_Limit;
	/*这里是我们的速度环*/
	
}PID_InitTypeDef;




extern struct STATE_PACK_1	state_pack;
extern PID_InitTypeDef My_PID_1;
extern PID_InitTypeDef My_PID_2;
/*----------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************************************************************/
/*----------------------------------------------------------------------------------------------------------------------------*/

/*元操作：右转，左转，直行，原地掉头*/
/*每一个操作后都会把对应的执行操作成功标志位置为1*/
extern u8 Res;
void Turn_Left()
{
	if(state_pack.Arrive_Target==0&&state_pack.Spin_Success==0&&state_pack.Turn_Success==0&&state_pack.GO_Straight==0&&state_pack.Turn_180==0&&state_pack.Turn_90==0)/*现在没有任何操作才能进行下一步*/
	{
		printf("4");
		Res=54;
		My_PID_1.Location_Target_Val=Turn_Left_Length;
		My_PID_2.Location_Target_Val=-Turn_Left_Length;
		state_pack.Turn_90=1;
	}
}



void Turn_Right()
{
	if(state_pack.Arrive_Target==0&&state_pack.Spin_Success==0&&state_pack.Turn_Success==0&&state_pack.GO_Straight==0&&state_pack.Turn_180==0&&state_pack.Turn_90==0)/*现在没有任何操作才能进行下一步*/
	{
		printf("4");
		Res=54;
		My_PID_1.Location_Target_Val=-Turn_Right_Length;
		My_PID_2.Location_Target_Val=Turn_Right_Length;
		state_pack.Turn_90=1;
	}
}



void Go_Straight_(int16_t straight_length)
{
	if(state_pack.Arrive_Target==0&&state_pack.Spin_Success==0&&state_pack.Turn_Success==0&&state_pack.GO_Straight==0&&state_pack.Turn_180==0&&state_pack.Turn_90==0)/*现在没有任何操作才能进行下一步*/
	{
		printf("2");		
		Res=54;
		My_PID_1.Location_Target_Val=straight_length;
		My_PID_2.Location_Target_Val=straight_length;	
		state_pack.GO_Straight=1;
	}
}



void Spin_()
{
	if(state_pack.Arrive_Target==0&&state_pack.Spin_Success==0&&state_pack.Turn_Success==0&&state_pack.GO_Straight==0&&state_pack.Turn_180==0&&state_pack.Turn_90==0)/*现在没有任何操作才能进行下一步*/
	{
		printf("4");
		Res=54;
		My_PID_1.Location_Target_Val=Spin_Length;
		My_PID_2.Location_Target_Val=-Spin_Length;
		state_pack.Turn_180=1;
	}
}


/*----------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************************************************************/
/*----------------------------------------------------------------------------------------------------------------------------*/



typedef struct 
{
	 uint8_t start;/*开始启动*/
	uint8_t processing;/*小车在路上*/
	uint8_t arrive_target;/*小车到达药房*/
	uint8_t re_start;/*小车返回开始*/
	uint8_t arrive_start;/*小车到达起点*/
}state_pack_start_end;

state_pack_start_end state_pack_2=
{
	.start=0,
	.processing=0,
	.arrive_target=0,
	.re_start=0,
	.arrive_start=0
};



/*----------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************************************************************/
/*----------------------------------------------------------------------------------------------------------------------------*/
extern uint8_t counter;//这个值用来记录进入中断的次数，为保障在摄像头操作的时候，一定是在判断数字
extern uint8_t Res;//串口一函数引入来的两个值，这个值用来记录左转右转直行 或者是循迹调整强度
uint8_t mode=3;//第一题，还是第2,3题
/*Res解析
*（1）对于识别数字模式：
*返回字符3：下一个节点处直行
*返回字符2：下一个节点处左转
*返回字符4：下一个节点处右转
*/

/*----------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************************************************************/
/*----------------------------------------------------------------------------------------------------------------------------*/
char recieve_buffer[20]={0};

char findMostFrequent(char arr[], char size) /*选取出现次数最多的元素*/
{
    int max_count = 0;
    char most_frequent = arr[0];

    for (int i = 0; i < size; i++) 
	{
        int count = 0;
        for (int j = 0; j < size; j++) 
		{
            if (arr[i] == arr[j]) {
                count++;
            }
        }
        if (count >= max_count) 
		{
            max_count = count;
            most_frequent = arr[i];
        }
    }
    return most_frequent;
}

uint8_t will_get_num=0;
void Get_Num()
{
	will_get_num=1;
	memset(recieve_buffer,0,sizeof(char)*20);
		if(state_pack_2.start==1)
			printf("3");
		else if(state_pack_2.start==0)
			printf("1");
	while(will_get_num!=0)
	{
		/*获得数字完毕？*/
	}

    Res=findMostFrequent(recieve_buffer,20);
}

/*摄像头操作*/

void Camera_Opeartion(tri_node* this)
{
	Get_Num();
	if (Res == '3')
	{
		this->straight = 1;
	}
	else if (Res == '2')
	{
		this->left = 1;
	}
	else if (Res == '4')
	{
		this->right = 1;
	}
}

void Wake_Camera()
{
//	if(mode==1)
//	{
//		if((Pre_2->straight==1&&Pre_2->camera!=1))
//		{
//				printf("2");
//				Res=56;
//		}
//		else
//		{
//				printf("4");
//				Res=56;/*表示直行，其实要不要无所谓，摄像头不发数据了也不会做出调整了*/
//		}
//	}
//	else if(mode==0)
//	{
//		if(Pre_1->straight&&Pre_1->camera!=1)
//		{
//			printf("2");
//							Res=56;
//		}
//		else
//		{
//			printf("4");
//			Res=56;/*表示直行，其实要不要无所谓，摄像头不发数据了也不会做出调整了*/
//		}
//	}
}



void State_Judge()
{
	if (state_pack_2.arrive_target == 0)
	{
		if (mode == 1)
		{
			if (state_pack.Arrive_Target == 1 || state_pack.Spin_Success == 1 || state_pack.Turn_Success == 1 || state_pack_2.start == 1)//任何一个操作完成，都要进行节点判断
			{
				Shut_Down_PID();
				/*关闭PID*/
				state_pack.Arrive_Target = 0;
				state_pack.Spin_Success = 0;
				state_pack.Turn_Success = 0;
				counter=0;

				if (state_pack_2.start == 1)
				{
					Get_Num();
					printf("%c",100+Res);/*回传得到的目标值*/
					/*其实不稳定*/
					while(Res!=120);
					state_pack_2.start = 0;
					/*并且读取当前节点*/
				}
				if (Now_2->camera == 1)
				{
					Camera_Opeartion(Now_2->next_front);
				}


				if (Now_2->straight == 1)
				{
					Pre_2 = Now_2;
					Now_2 = Now_2->next_front;
					Push(Go_Staight, Pre_2->go_straight_length);
					/*进行直行操作*/
					/*这里我们假设一瞬间完成*/
					Go_Straight_(Pre_2->go_straight_length);
				}
				else if (Now_2->right == 1)
				{
					Pre_2 = Now_2;
					Now_2 = Now_2->next_right;
					Push(Go_Left, 0);
					/*进行右转操作，假设一瞬间完成*/
					Turn_Right();
				}
				else if (Now_2->left == 1)
				{
					Pre_2 = Now_2;
					Now_2 = Now_2->next_left;
					Push(Go_Right, 0);
					/*进行左转操作，假设一瞬间完成*/
					Turn_Left();
				}

				if (Now_2->spin == 1)
				{
					state_pack_2.arrive_target = 1;
					state_pack_2.processing = 0;
					Push(Spin, 0);//调头操作入栈
					Stack[1].length-=28;
					
				}
				Wake_Camera();
				Restart_PID();
			}
		}
		else if (mode == 0)
		{
			if (state_pack.Arrive_Target == 1 || state_pack.Spin_Success == 1 || state_pack.Turn_Success == 1 || state_pack_2.start == 1)//任何一个操作完成，都要进行节点判断
			{
				Shut_Down_PID();
				state_pack.Arrive_Target = 0;
				state_pack.Spin_Success = 0;
				state_pack.Turn_Success = 0;
				counter=0;

				if (state_pack_2.start == 1)
				{
					Get_Num();
					state_pack_2.start = 0;
					if (Res == '1')
						Now_1->next_front->left = 1;
					else if (Res == '2')
						Now_1->next_front->right = 1;
				}



				if (Now_1->straight == 1)
				{
					Pre_1 = Now_1;
					Now_1 = Now_1->next_front;
					Push(Go_Staight, Pre_1->go_straight_length);
					/*进行直行操作*/
					/*这里我们假设一瞬间完成*/
					Go_Straight_(Pre_1->go_straight_length);
				}
				else if (Now_1->right == 1)
				{
					Pre_1 = Now_1;
					Now_1 = Now_1->next_right;
					Push(Go_Left, 0);
					/*进行右转操作，假设一瞬间完成*/
					Turn_Right();
				}
				else if (Now_1->left == 1)
				{
					Pre_1 = Now_1;
					Now_1 = Now_1->next_left;
					Push(Go_Right, 0);
					/*进行左转操作，假设一瞬间完成*/
					Turn_Left();
				}

				if (Now_1->spin == 1)
				{
					state_pack_2.arrive_target = 1;
					state_pack_2.processing = 0;
					Push(Spin, 0);//调头操作入栈
					Stack[1].length-=28;
				}
				Wake_Camera();
				Restart_PID();
			}

		}
	}
}

extern tri_node My_Map_1[6] ;/*近端病房用*/
extern tri_node My_Map_2[23] ;/*中部和远端病房用*/


extern tri_node* Pre_2 ;
extern tri_node* Now_2 ;/*这里调节使用那种模式*/

extern tri_node* Pre_1 ;
extern tri_node* Now_1;/*这里调节使用那种模式*/

void Back_To_Souce()
{

	if (state_pack_2.re_start == 1)
	{
		/*关闭PID*/
		Shut_Down_PID();
		state_pack.Arrive_Target = 0;
		state_pack_2.processing = 1;
		state_pack_2.re_start = 0;
		POP();
		Spin_();
		/*调头*/

		Restart_PID();
		/*开启PID*/
	}
	else if (state_pack_2.processing == 1 && state_pack_2.arrive_target == 1)/*这里的Processing标志就是检查药是不是拿走了*/
	{
		if (state_pack.Arrive_Target == 1 || state_pack.Spin_Success == 1 || state_pack.Turn_Success == 1)//任何一个操作完成，都要进行节点判断
		{
			Shut_Down_PID();
			state_pack.Arrive_Target = 0;
			state_pack.Spin_Success = 0;
			state_pack.Turn_Success = 0;

			if (Stack[0].length == 0)/**/
			{
				state_pack_2.processing = 0;
				state_pack_2.arrive_target=0;
				state_pack_2.arrive_start = 1;/*到达原点*/
			}
			else
			{
				POP();
				if (Stack_Top->dir == 1)
				{
					//printf("Go_Straight,%d\n", Stack_Top->length);
					/*执行对应操作*/
					/*这里模拟，假设一瞬间完成*/
					Go_Straight_(Stack_Top->length);
				}
				else if (Stack_Top->dir == 2)
				{
					//printf("Go_Left\n");
					/*执行对应操作*/
					Turn_Left();
				}
				else if (Stack_Top->dir == 3)
				{
					//printf("Go_Right\n");
					/*执行对应操作*/
					Turn_Right();
				}
			}
			Restart_PID();
		}
	}
}


/*去往药房*/
void Start_Car_To_Destination()
{
	/*如果检测到药品放到车上了，那么就*/
	state_pack_2.start=1;//表示开始
}


/*回到出发点*/
void Start_Car_To_Source()
{
	state_pack_2.re_start=1;
	state_pack_2.processing=1;
}



void Item_Detect_GPIO_Init()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOF,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOF,&GPIO_InitStructure);
}
void ALL_In_This(void);

typedef struct
{
	double Kp;
	double Ki;
	double Kd;
	double Error;
	double Last_Error;
	double Integral;
	double Target_Val;
	double Actual_Val;
	double Out;
	
	
	double Out_Limit;
	double Int_Limit;
}PID_Mini;
extern PID_Mini XunjiPID;
void Item_Detect()
{
//	if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==1)/*假设高电平就是药品放下了*/
//	{
//		GPIO_ResetBits(GPIOC,GPIO_Pin_2);//绿灯熄灭
//		GPIO_SetBits(GPIOC,GPIO_Pin_1);/*红灯亮*/		
//		ALL_In_This();
//		/*点灯*/
//	}
//	
	if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==0&&state_pack_2.arrive_target==1&&state_pack_2.processing!=1)/*药品拿起了状态，且到达了目标点*/
	{
		LCD_ShowNum(200,200,state_pack_2.arrive_target,2,16);
		GPIO_ResetBits(GPIOC,GPIO_Pin_1);/*红灯熄灭*/
		delay_ms(1000);
		Start_Car_To_Source();/*药品一旦拿起，就要返航了*/
	}
	
	if(state_pack_2.arrive_start==1)
	{
		GPIO_SetBits(GPIOF,GPIO_Pin_0);/*点亮绿灯*/
		
		
		state_pack_2.arrive_target=0;//所有标志位至此，全部归零，只是剩下到达标志，这里也可以顺带清零

		
		/*这里就是结束后，摄像头要做什么的*/
				 printf("2");/*告诉摄像头操作结束，现在进行空操作*/
				 mode=3;//模式复原
	
				/*复位地图与指针*/
			   Reset_Map_1();
				 Reset_Map_2();
				 Pre_2 = &My_Map_2[0];
				 Now_2 = &My_Map_2[0];/*这里调节使用那种模式*/

				 Pre_1 = &My_Map_1[0];
				 Now_1 = &My_Map_1[0];/*这里调节使用那种模式*/
		/*为了保险，这里再次清除所有标志位*/
			state_pack.Arrive_Target=0;
			state_pack.GO_Straight=0;
			state_pack.Spin_Success=0;
			state_pack.Turn_180=0;
			state_pack.Turn_90=0;
			state_pack.Turn_Success=0;
			
			state_pack_2.arrive_target=0;
			state_pack_2.processing=0;
			state_pack_2.re_start=0;
			state_pack_2.start=0;
			Init_Stack();
			
			XunjiPID.Error=0;
			XunjiPID.Integral=0;
			XunjiPID.Last_Error=0;

			/*旨在初始化*/
			
			
			state_pack_2.arrive_start=0;	
	}
}

/*开始操作*/

void ALL_In_This()
{
//int key_num=KEY_Scan(0);
//if(key_num==1)/*如果有按键按下且为按键0按下*/
//{
//	state_pack_2.arrive_start=0;
//	mode=0;//设置为为模式1，也就是说我们现在要开始去近端药房
//	state_pack_2.start=1;//表示开始了
//	Restart_PID();
//}
//else if(key_num==2)
//{
//	state_pack_2.arrive_start=0;
//	mode=1;
//	state_pack_2.start=1;//表示在模式2开始
//	Restart_PID();
//}

}































































































/*接下来要实现的就是节点状态写死+具体操作了。这个暂时搁置下，学摄像头*/
/*
*发送1：摄像头要识别数字(运行过程中)
*发送2：摄像头要进行寻迹操作
*发送3：摄像头要进行目标值的读取
*/
/*该函数暂时废弃*/
void Process_To_Target()
{
	if(state_pack.Arrive_Target==1||state_pack.Spin_Success==1||state_pack.Turn_Success==1||state_pack_2.start==1)//任何一个操作完成，都要进行节点判断
	{
		Shut_Down_PID();//首先关闭PID
		counter=0;//复位
		state_pack.Arrive_Target=0;
		state_pack.Spin_Success=0;
		state_pack.Turn_Success=0;


		/*所有标志全部清零，理论上，当前状态包1的所有标志全部为0，状态包2只能有processing为1其他全部为0*/
		/*同时摄像头会被while函数堵死*/
		
		
		
		
		
		
		
		if(state_pack_2.start==1)
		{
			state_pack_2.start=0;
			while(counter<2)
			{
				printf("%d",3);
			}
			state_pack_2.processing=1;
			if(mode==0)/*第一题*/
			{
				if(Now_1->straight==1)/*表明当前要往前走*/
				{
					Pre_1 = Now_1;
					Now_1 = Now_1->next_front;
					if(Res=='1')
						Now_1->left=1;
					else if(Res=='2')
						Now_1->right=1;
					Go_Straight_(First_Step_Mode_1);
				}
			}
			else if(mode==1)/*第二三题*/
			{
					Pre_2=Now_2;
					Now_2=Now_2->next_front;
					Go_Straight_(First_Step_Mode_2);
			}
		}
		else
		{
			/*首先检查摄像头是否需要为检测数字模式*/
			if(Now_1->camera==1||Now_2->camera==1)/*表示当前要识别数字才可以*/
			{
				while(counter<2)
				{
					printf("%d",1);
				}
				if(Res=='3')/*直行*/
				{
					/*第一题节点处不可能直行*/
					/*所以只讨论第二三题即可*/
					Now_2->next_front->straight=1;//下一个节点要直走，这是由Res得到的
				}
				else if(Res=='2')
				{
					if(mode==0)
					Now_1->next_front->left=1;
					else if(mode==1)
						Now_2->next_front->left=1;
				}
				else if(Res=='4')
				{
					if(mode==0)
						Now_1->next_front->right=1;
					else if(mode==1)
						Now_2->next_front->right=1;
				}
			}
			
			/*检查往哪个方向走*/
			if(mode==0&&Now_1->spin!=1)
			{
				if(Pre_1->straight==1)
					Push(Go_Staight,Pre_1->go_straight_length);
				else if(Pre_1->left==1)
					Push(Go_Left,0);
				else if(Pre_1->right==1)
					Push(Go_Right,0);
				/*把上一步的操作入栈*/
				
				if(Now_1->straight==1)
				{
					Go_Straight_(Now_1->go_straight_length);
					Pre_1=Now_1;
					Now_1=Now_1->next_front;
				}
				else if(Now_1->left==1)
				{
				Turn_Left();
					Pre_1=Now_1;
					Now_1=Now_1->next_left;
				}
				else if(Now_1->right==1)
				{
					Turn_Right();
					Pre_1=Now_1;
					Now_1=Now_1->next_right;
				}
				
			}
			else if(mode==0&&Now_1->spin==1)
			{
				if(Pre_1->straight==1)
					Push(Go_Staight,Pre_1->go_straight_length);
				else if(Pre_1->left==1)
					Push(Go_Left,0);
				else if(Pre_1->right==1)
					Push(Go_Right,0);
				/*把上一步的操作入栈*/
				Push(Spin,0);
				state_pack_2.processing=0;
				state_pack_2.arrive_target=1;/*表达我已经到了目标位置了*/
			}

			
			if(mode==1&&Now_2->spin!=1)
			{
				if(Pre_2->straight==1)
					Push(Go_Staight,Pre_2->go_straight_length);
				else if(Pre_2->left==1)
					Push(Go_Left,0);
				else if(Pre_2->right==1)
					Push(Go_Right,0);
				/*把上一步的操作入栈*/
				
				if(Now_2->straight==1)
				{
					Go_Straight_(Now_2->go_straight_length);
					Pre_2=Now_2;
					Now_2=Now_2->next_front;
				}
				else if(Now_2->left==1)
				{
				Turn_Left();
					Pre_2=Now_2;
					Now_2=Now_2->next_left;
				}
				else if(Now_2->right==1)
				{
					Turn_Right();
					Pre_2=Now_2;
					Now_2=Now_2->next_right;
				}
			}
			else if(mode==1&&Now_2->spin==1)
			{
				if(Pre_2->straight==1)
					Push(Go_Staight,Pre_2->go_straight_length);
				else if(Pre_2->left==1)
					Push(Go_Left,0);
				else if(Pre_2->right==1)
					Push(Go_Right,0);
				/*把上一步的操作入栈*/
				Push(Spin,0);
				state_pack_2.processing=0;
				state_pack_2.arrive_target=1;
			}
		
		}
		/*这里可能会出现卡死的情况*/
		if(Pre_2->straight==1||Pre_1->straight==1)
		printf("%d",2);//相当于触发一下，告诉摄像头，你要开始循迹模式了。之后摄像头传回的数据由中断自动触发
		else
		{
			printf("%d",4);//转弯调头这些的，摄像头不参与
			Res='0';//不进行任何操作
		}

		if(state_pack_2.arrive_target==0||state_pack_2.arrive_start==0)
		Restart_PID();//重新开启PID(但是重新开启的条件就是还没有到达指定位置)
	}
}




