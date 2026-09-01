#include <stm32f4xx.h>
#include <stdio.h>
#include <string.h>
#include "My_Stack.h"





My_Operation_Block Stack[20];/*首元素的length记录当前栈中的元素个数*/
My_Operation_Block* Stack_Bottom=&Stack[1];/*栈底*/
My_Operation_Block* Stack_Top=&Stack[1];/*栈顶*/



void Init_Stack()
{
	memset(Stack,0,sizeof(My_Operation_Block)*20);
}

/*入栈*/
void Push(uint8_t dir,int16_t length)
{
	Stack[0].length=Stack[0].length+1;/*元素自增*/
	Stack_Top->length=length;//(记录长度)
	Stack_Top->dir=dir;//记录方向
	Stack_Top++;//栈顶指针向后移动
	
	/*合并同类项*/
	if(Stack[Stack[0].length-1].dir == Stack[Stack[0].length].dir)
	{
		Stack[Stack[0].length - 1].length += Stack[Stack[0].length].length;
		Stack_Top--;
		Stack[0].length--;
	}	
}

/*出栈*/
void POP()
{
	if(Stack[0].length>0||Stack_Top<Stack_Bottom)
	{
		Stack_Top--;
		Stack[0].length--;
	}
}


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
/*地图*/

tri_node My_Map_1[6] = { 0 };/*近端病房用*/
tri_node My_Map_2[23] = { 0 };/*中部和远端病房用*/


tri_node* Pre_2 = &My_Map_2[0];
tri_node* Now_2 = &My_Map_2[0];/*这里调节使用那种模式*/

tri_node* Pre_1 = &My_Map_1[0];
tri_node* Now_1 = &My_Map_1[0];/*这里调节使用那种模式*/


void Reset_Map_2()
{
	memset(My_Map_2,0,sizeof(tri_node)*23);


	for (int i = 0; i < 23; i++)
		My_Map_2[i].id = i;

	My_Map_2[0].straight = 1;
	My_Map_2[0].go_straight_length = First_Step_Mode_2;
	My_Map_2[0].next_front = &My_Map_2[1];

	My_Map_2[1].camera = 1;/*这里需要摄像头识别数字*/
	My_Map_2[1].straight = 1;
	My_Map_2[1].go_straight_length = Second_Step_Mode_2;
	My_Map_2[1].next_front = &My_Map_2[2];



	My_Map_2[2].next_front = &My_Map_2[3];
	My_Map_2[2].next_left = &My_Map_2[4];
	My_Map_2[2].next_right = &My_Map_2[5];
	My_Map_2[2].go_straight_length = Fourteen_Step_Mode_2;

	My_Map_2[4].next_front = &My_Map_2[6];
	My_Map_2[4].go_straight_length = Three_Step_Mode_2;
	My_Map_2[4].straight = 1;

	My_Map_2[6].spin = 1;

	My_Map_2[5].next_front = &My_Map_2[7];
	My_Map_2[5].go_straight_length = Four_Step_Mode_2;
	My_Map_2[5].straight = 1;

	My_Map_2[7].spin = 1;

	My_Map_2[3].camera = 1;/*这里需要摄像头识别数字*/
	My_Map_2[3].straight = 1;
	My_Map_2[3].go_straight_length = Five_Step_Mode_2;
	My_Map_2[3].next_front = &My_Map_2[8];

	My_Map_2[8].next_left = &My_Map_2[16];
	My_Map_2[8].next_right = &My_Map_2[9];

	My_Map_2[9].next_front = &My_Map_2[10];
	My_Map_2[9].go_straight_length = Six_Step_Mode_2;
	My_Map_2[9].straight = 1;

	My_Map_2[10].next_front = &My_Map_2[11];
	My_Map_2[10].go_straight_length = Nine_Step_Mode_2;
	My_Map_2[10].straight = 1;
	My_Map_2[10].camera = 1;

	My_Map_2[16].next_front = &My_Map_2[17];
	My_Map_2[16].go_straight_length = Seven_Step_Mode_2;
	My_Map_2[16].straight = 1;

	My_Map_2[17].next_front = &My_Map_2[18];
	My_Map_2[17].go_straight_length = Eight_Step_Mode_2;
	My_Map_2[17].straight = 1;
	My_Map_2[17].camera = 1;

	My_Map_2[11].next_left = &My_Map_2[14];
	My_Map_2[11].next_right = &My_Map_2[12];

	My_Map_2[18].next_left = &My_Map_2[21];
	My_Map_2[18].next_right = &My_Map_2[19];


	My_Map_2[19].next_front = &My_Map_2[20];
	My_Map_2[19].go_straight_length = Twelve_Step_Mode_2;
	My_Map_2[19].straight = 1;

	My_Map_2[21].next_front = &My_Map_2[22];
	My_Map_2[21].go_straight_length = Thirteen_Step_Mode_2;
	My_Map_2[21].straight = 1;

	My_Map_2[12].next_front = &My_Map_2[13];
	My_Map_2[12].go_straight_length = Eleven_Step_Mode_2;
	My_Map_2[12].straight = 1;

	My_Map_2[14].next_front = &My_Map_2[15];
	My_Map_2[14].go_straight_length = Ten_Step_Mode_2;
	My_Map_2[14].straight = 1;
	My_Map_2[13].spin = 1;
	My_Map_2[15].spin = 1;
	My_Map_2[20].spin = 1;
	My_Map_2[22].spin = 1;


	///*调试用*/
	//My_Map_2[2].straight = 1;
	//My_Map_2[8].left = 1;
	//My_Map_2[11].left = 0;
	//My_Map_2[18].right = 1;
}


void Reset_Map_1()
{
	memset(My_Map_1,0,sizeof(tri_node)*6);

	My_Map_1[0].straight = 1;
	My_Map_1[0].go_straight_length = First_Step_Mode_1;
	My_Map_1[0].next_front = &My_Map_1[1];

	My_Map_1[1].next_left = &My_Map_1[2];
	My_Map_1[1].next_right = &My_Map_1[4];

	My_Map_1[4].straight = 1;
	My_Map_1[4].go_straight_length = Second_Step_Mode_1;
	My_Map_1[4].next_front = &My_Map_1[5];

	My_Map_1[2].straight = 1;
	My_Map_1[2].go_straight_length = Third_Step_Mode_1;
	My_Map_1[2].next_front = &My_Map_1[3];

	My_Map_1[5].spin = 1;
	My_Map_1[3].spin = 1;


	for (int i = 0; i < 5; i++)
		My_Map_1[i].id = i;

	///*调试用*/

	//My_Map_1[1].left = 1;
}

