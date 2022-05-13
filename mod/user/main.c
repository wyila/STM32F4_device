#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include <stdio.h>


/*下面主函数是使用HAL库函数实现控制IO口输出*/

//int main(void)
//{
//    HAL_Init();                     //初始化HAL库
//    Stm32_Clock_Init(360,25,2,8);   //设置时钟,180Mhz
//    delay_init(180);                //初始化延时函数
//    LED_Init();                     //初始化LED
//    while(1)
//    {
//        HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_RESET); //LED0对应引脚拉低，亮，等同于LED0(0)
//
//        delay_ms(500);										//延时500ms
//        HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);   //LED0对应引脚拉高，灭，等同于LED0(1)
//
//        delay_ms(500);                                      //延时500ms
//    }
//}
//


/*下面主函数使用位带操作实现：*/

//int main(void)
//{
//    HAL_Init();                     //初始化HAL库
//    Stm32_Clock_Init(360,25,2,8);   //设置时钟,180Mhz
//    delay_init(180);                //初始化延时函数
//    LED_Init();                     //初始化LED    while(1)
//	{
//     LED0=0;			     //LED0亮
//
//		 delay_ms(500);
//		 LED0=1;				//LED0灭

//		 delay_ms(500);
//	 }
//}




/*
下面主函数使用直接操作结存器方式实现跑马灯
*/

int main(void)
{
    HAL_Init();                     //初始化HAL库
    Stm32_Clock_Init(360,25,2,8);   //设置时钟,180Mhz
    delay_init(180);                //初始化延时函数
    LED_Init();                     //初始化LED
    uart_init(115200);              //初始化串口
    GPIOC->ODR |= GPIO_PIN_13;
	while(1)
	{
        if(USART_RX_STA)
        {
            printf("You send data is: %s", USART_RX_BUF);
            USART_RX_LEN = 0;
            GPIOC->ODR ^= GPIO_PIN_13;
            USART_RX_STA = 0;
        }

        delay_ms(200);
    }
 }





