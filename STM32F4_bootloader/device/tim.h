#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"

extern int TIM3_IT_STATUS;

void TIM3_Init(u16 arr,u16 psc);
#endif

