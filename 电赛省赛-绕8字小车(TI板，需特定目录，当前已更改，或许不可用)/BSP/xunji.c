#include "xunji.h"
#include "ti_msp_dl_config.h"
#include "board.h"
#include "stdio.h"
#include "oled.h"


//X1 B6
//X2 B7
//X3 B8
//X4 B9
//X5 B18
//X6 B19
//X7 B20
//X8 B24


//黑线灯亮 低电平
//白色灯灭 高电平
//即X=1    出界
//X=0      在线上


void track_deal_four(int *s1,int *s2,int *s3,int *s4,int *s5,int *s6,int *s7,int *s8)
{
	*s1 = DL_GPIO_readPins(xunji_X1_PORT,xunji_X1_PIN) > 0 ? 1 : 0;;
	*s2 = DL_GPIO_readPins(xunji_X2_PORT,xunji_X2_PIN) > 0 ? 1 : 0;;
	*s3 = DL_GPIO_readPins(xunji_X3_PORT,xunji_X3_PIN) > 0 ? 1 : 0;;
	*s4 = DL_GPIO_readPins(xunji_X4_PORT,xunji_X4_PIN) > 0 ? 1 : 0;;
						 
	*s5 = DL_GPIO_readPins(xunji_X5_PORT,xunji_X5_PIN) > 0 ? 1 : 0;;
	*s6 = DL_GPIO_readPins(xunji_X6_PORT,xunji_X6_PIN) > 0 ? 1 : 0;;
	*s7 = DL_GPIO_readPins(xunji_X7_PORT,xunji_X7_PIN) > 0 ? 1 : 0;;
	*s8 = DL_GPIO_readPins(xunji_X8_PORT,xunji_X8_PIN) > 0 ? 1 : 0;;
}

//send_control_data(0,0,1);//发送接收数字型数据
//每20ms调用一次
//int car_track(void)
//{
//	
//	int error;
//	int x1,x2,x3,x4,x5,x6,x7,x8;
//	
//	track_deal_four(&x1,&x2,&x3,&x4,&x5,&x6,&x7,&x8);
//					
//			
//		if(x1==0)
//		{
//			error=25;
//		}
//		else if(x2==0)
//		{
//			error=20;	
//		}
//		else if(x3==0)
//		{
//			error=15;		
//		}
//		else if(x4==0)
//		{
//			error=5;	
//		}
//		else if(x5==0)
//		{
//			error=-5;		
//		}
//		else if(x6==0)
//		{
//			error=-15;
//		
//		}
//		else if(x7==0)
//		{
//			error=-20;	
//		}
//		else if(x8==0)
//		{
//			error=-25;	
//		}
//		else 
//		{
//			error=0;
//		}
//	return error;
//	
//}


int car_track(void)
{
	
	int error;
	int x1,x2,x3,x4,x5,x6,x7,x8;
	
	track_deal_four(&x1,&x2,&x3,&x4,&x5,&x6,&x7,&x8);
					
			
		if(x1==0)
		{
			error=25;
		}
		
	     else if(x8==0)
		{
			error=-25;	
		}
		
		
		else if(x2==0)
		{
			error=20;	
		}
		else if(x7==0)
		{
			error=-20;	
		}
		
		else if(x3==0)
		{
			error=15;		
		}
		else if(x6==0)
		{
			error=-15;
		
		}	
		else if(x4==0)
		{
			error=5;	
		}
		else if(x5==0)
		{
			error=-5;		
		}
		
		else 
		{
			error=0;
		}
	return error;
	
}













int car_track_last(void)
{
	
	int error;
	int x1,x2,x3,x4,x5,x6,x7,x8;
	
	track_deal_four(&x1,&x2,&x3,&x4,&x5,&x6,&x7,&x8);
					
	    if(x4==0)
		{
			error=5;	
		}
		else if(x5==0)
		{
			error=-5;		
		}
		else if(x3==0)
		{
			error=7;		
		}
		else if(x6==0)
		{
			error=-7;	
		}
		else if(x2==0)
		{
			error=10;	
		}
		else if(x7==0)
		{
			error=-10;	
		}
		else if(x1==0)
		{
			error=15;
		}
		else if(x8==0)
		{
			error=-15;	
		}
		else 
		{
			error=0;
		}
	return error;
	
}




//是否检测到黑线
//检测到黑线返回1
//否则返回0
int on_line(void)
{
   int x1,x2,x3,x4,x5,x6,x7,x8;
	
	
	track_deal_four(&x1,&x2,&x3,&x4,&x5,&x6,&x7,&x8);
	
	
	if(x1==0||x2==0||x3==0||x4==0||x5==0||x6==0||x7==0||x8==0)
	{
		return 1;
	}
	else 
	{
		return 0;
	}

}


int on_line_x(void)
{
	int x1,x2,x3,x4,x5,x6,x7,x8;	
	track_deal_four(&x1,&x2,&x3,&x4,&x5,&x6,&x7,&x8);
	if(x1==0||x2==0||x3==0||x4==0)
	{
		return 1;
	}
	else 
	{
		return 0;
	
	}	
}


int on_line_y(void)
{
	int x1,x2,x3,x4,x5,x6,x7,x8;	
	track_deal_four(&x1,&x2,&x3,&x4,&x5,&x6,&x7,&x8);
	if(x5==0||x6==0||x7==0||x8==0)
	{
		return 1;
	}
	else 
	{
		return 0;
	
	}
	
}


















