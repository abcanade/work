#ifndef __GPIO_H
#define __GPIO_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 

#define PWM_LED PBout(8)
#define SR501 PBin(6)

#define KEY1 PBin(12)
#define KEY2 PBin(13)
#define KEY3 PBin(14)
#define KEY4 PBin(15)

void KEY_GPIO_Init(void);//≥ı ºªØ
	 				    
#endif

