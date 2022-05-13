#ifndef __TIM_H
#define __TIM_H
#include "sys.h"

extern int TIM3_IT_STATUS;
extern int TIM4_IT_STATUS;
extern int TIM6_IT_STATUS;
extern int TIM7_IT_STATUS;
extern unsigned int TIM4_interrupt_cnt;
extern unsigned int TIM6_interrupt_cnt;
extern unsigned int TIM7_interrupt_cnt;
void TIM_Init(void);

void TIM3_Init(u16 arr, u16 psc);
void TIM4_Init(u16 arr, u16 psc);
void TIM6_Init(u16 arr, u16 psc);
void TIM7_Init(u16 arr, u16 psc);
#endif

