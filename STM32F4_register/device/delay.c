#include "delay.h"
#include "led.h"
#include "sys.h"

unsigned int delay_interrupt_cnt = 0;

void delay_init(unsigned char sys_clock)
{
    RCC->APB1ENR |= 1;//使能TIM2时钟
    TIM2->CR1 &= ~1;//0xFFFFFFFE;//关闭定时器，需要时再开启
    TIM2->ARR = 89;//自动装载值
    TIM2->PSC = 0;//分频系数
    TIM2->CR1 &= ~0x310;//向上计数，不分频
    TIM2->DIER |= 1;//允许更新事件
    delay_interrupt_cnt = 0;
    //TIM2->CR1 |= 1;//开启定时器
    MY_NVIC_SET(3, 0, TIM2_IRQn);//设置中断优先级
}

void TIM2_IRQHandler(void)
{
    if(TIM2->SR & 0x01)
    {
        delay_interrupt_cnt ++;
        TIM2->SR &= ~(u32)0x01;
        //LED0 = !LED0;
    }
}

void delay_ms(unsigned short ms)
{
    TIM2->PSC = 899; //频率设为100KHz
    TIM2->ARR = 99;//大概一毫秒中断一次
    TIM2->CNT = 0;//计数清零
    TIM2->CR1 |= 0x01;//开启定时器
    while(delay_interrupt_cnt < ms);
    delay_interrupt_cnt = 0;
    TIM2->CR1 &= ~(u32)0x01;
    
    TIM2->PSC = 0; //不进行分频
    TIM2->ARR = 90 - 1;//大概1us中断一次
}

void delay_us(unsigned int us)
{
    TIM2->CNT = 0;
    TIM2->CR1 |= 0x01;//开启定时器
    while(delay_interrupt_cnt < us);
    delay_interrupt_cnt = 0;
    TIM2->CR1 &= ~(u32)0x1;//关掉定时器
}

/*
//定时器3中断服务程序
void TIM3_IRQHandler(void)
{
	if(TIM3->SR&0X0001)//溢出中断
	{
		LED0=!LED0;
	}
	TIM3->SR&=~(1<<0);//清除中断标志位
}
//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为45M
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<1;	//TIM3时钟使能
 	TIM3->ARR=arr;  	//设定计数器自动重装值
	TIM3->PSC=psc;  	//预分频器
	TIM3->DIER|=1<<0;   //允许更新中断
	TIM3->CR1|=0x01;    //使能定时器3
  	MY_NVIC_SET(1,3,TIM3_IRQn);	//抢占1，子优先级3，组2
}


*/


