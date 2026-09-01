#ifndef	__BOARD_H__
#define __BOARD_H__

#include "ti_msp_dl_config.h"


#ifndef u8
#define u8 uint8_t
#endif

#ifndef u16
#define u16 uint16_t
#endif

#ifndef u32
#define u32 uint32_t
#endif




void board_init(void);

void delay_us(unsigned long __us);
void delay_ms(unsigned long ms);
void delay_1us(unsigned long __us);
void delay_1ms(unsigned long ms);


void uart0_send_char(char ch);
void uart0_send_string(char* str);





void Deal_Usart_Data(void);
void send_control_data(u8 adjust,u8 aData,u8 dData);
void Deal_IR_Usart(u8 rxtemp);








#endif
