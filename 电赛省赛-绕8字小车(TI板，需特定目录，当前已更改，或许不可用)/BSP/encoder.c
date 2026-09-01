#include "encoder.h"
#include "ti_msp_dl_config.h"
#include "board.h"
#include "stdio.h"
#include "oled.h"



//void GROUP1_IRQHandler(void)
//{
//    
//    switch( DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1))
//    {
//        //处理按键中断
//        case KEY_INT_IIDX:
//            // 按键1处理
//            if(DL_GPIO_readPins(KEY_PORT, KEY_PIN_23_PIN)>0)
//            {
//                // 添加按键消抖逻辑（推荐）
//                delay_ms(10);
//				key_num=1;
//				
//				OLED_ShowString(0,48,(uint8_t *)"mode:",16,1);
//	            OLED_ShowNum(48,48,key_num,4,16,1);
//	            OLED_Refresh();
//				
//				
//              				
//		      DL_GPIO_clearInterruptStatus(KEY_PORT,KEY_PIN_23_PIN);	
//            }
//            // 按键2处理
//            if(DL_GPIO_readPins(KEY_PORT, KEY_PIN_24_PIN) == 0)
//            {
//                // 按键2处理代码
//				delay_ms(10);
//				key_num=2;
//				
//			   DL_GPIO_clearInterruptStatus(KEY_PORT,KEY_PIN_24_PIN);	
//            }
//		

//            break;
//            

//        default:
//            // 处理未知中断或清除残留标志
//            break;
//		
//    }	
//}







