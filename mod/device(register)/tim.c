#include "tim.h"
#include "led.h"

int TIM6_IT_STATUS = 0;

//定时器时钟为90MHz
//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!(定时器3挂在APB1上，时钟为HCLK/2)
void TIM6_Init(u16 arr,u16 psc)
{
    RCC->APB1ENR |= 1 << 4;//使能TIM6时钟
    TIM6->ARR = arr;//自动装载值
    TIM6->PSC = psc;//分频系数
    TIM6->CR1 &= ~0x310;//向上计数，不分频
    TIM6->DIER |= 1;//允许更新事件
    TIM6_IT_STATUS = 0;
    TIM6->CR1 |= 1;//开启定时器
    MY_NVIC_SET(4, 0, TIM6_DAC_IRQn);//设置中断优先级
}



//定时器6中断服务函数
void TIM6_DAC_IRQHandler(void)
{
    if(TIM6->SR & 0X0001)//溢出中断
	{
		TIM6_IT_STATUS = 1;
        LED0 = !LED0;
        TIM6->SR &= ~0x01;//清除中断标志位
	}
}




