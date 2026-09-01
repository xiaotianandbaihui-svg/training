#ifndef	__ENCODER_H__
#define __ENCODER_H__


#include "ti_msp_dl_config.h"

typedef enum 
{
    FORWARD,  // 正向
    REVERSAL  // 反向
} ENCODER_DIR;

typedef struct 
{
	volatile long long temp_count;    //保存实时计数值
    int count;         				  //根据定时器时间更新的计数值
    ENCODER_DIR dir;            	  //旋转方向
} ENCODER_RES;




void encoder_init(void);
int get_encoder_count(void);
int get_encoder1_count(void);
ENCODER_DIR get_encoder_dir(void);
ENCODER_DIR get_encoder1_dir(void);
void encoder_update(void);

void wheel1_get();
void wheel2_get();

















#endif

