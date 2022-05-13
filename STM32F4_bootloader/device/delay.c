#include "delay.h"
#include "sys.h"
#define SYS_CLOCK   180000000
/*
主要寄存器：
SysTick->CTRL, --控制和状态寄存器
SysTick->LOAD, --重装载寄存器
SysTick->VAL, --当前值寄存器
SysTick->CALIB, --校准值寄存器
*/
u32 SysTick_cnt = 0;//进入中断次数

//初始化延迟函数
//SYSTICK的时钟固定为AHB时钟
//SYSCLK:系统时钟频率
void delay_init(u8 SYSCLK)
{
    //HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);//SysTick频率为HCLK
	//SysTick_Config(SystemCoreClock / 1000000);
    SysTick->CTRL |= 0x6;//0b 0000 0000 0000 0000 0110
}


//延时nus
//nus为要延时的us数.
//nus:0~190887435(最大值即2^32/fac_us@fac_us=22.5)
void delay_us(u32 us)
{
    SysTick->VAL = 0;
    SysTick->LOAD = SYS_CLOCK / 1000000 - 1;//1us中断一次
    SysTick->CTRL |= 0x01;//开启定时器
    
    while(1)
    {
        if(SysTick_cnt >= us)
        {
            SysTick_cnt = 0;
            break;
        }
    }
    SysTick->CTRL ^= 0x01;//关闭定时器
}

//延时nms
//nms:要延时的ms数
void delay_ms(u16 ms)
{
    SysTick->VAL = 0;
    SysTick->LOAD = SYS_CLOCK / 1000 - 1;//1ms中断一次
    SysTick->CTRL |= 0x01;//开启定时器
    
    while(1)
    {
        if(SysTick_cnt >= ms)
        {
            SysTick_cnt = 0;
            break;
        }
    }
    
    SysTick->CTRL ^= 0x01;//关闭定时器
}

/*
 * SysTick中断函数
 */
void SysTick_Handler(void)
{
    SysTick_cnt ++;
}

