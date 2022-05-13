#include "led.h"


//初始化PB1为输出.并使能时钟
//LED IO初始化
void LED_Init(void)
{
    RCC->AHB1ENR |= 1 << 2;//使能GPIOC时钟
    
    GPIOC->MODER &= ~(0x03 << (13 * 2));//清空状态
    GPIOC->MODER |= 0x01 << (13 * 2);//通用推挽输出
    
    GPIOC->OSPEEDR &= ~(0x03 << (13 * 2));//清空状态
    GPIOC->OSPEEDR |= 0x10 << (13 * 2);//快速模式50MHz
    
    GPIOC->PUPDR &= ~(0x03 << (13 * 2));//无上下拉
    
    GPIOC->ODR |= PIN13;
}
