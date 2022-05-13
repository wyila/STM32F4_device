#include "led.h"


//初始化PB1为输出.并使能时钟
//LED IO初始化
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOC_CLK_ENABLE();           //开启GPIOB时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    GPIO_Initure.Pin=GPIO_PIN_13; //PC13,0
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOC,&GPIO_Initure);
    
    
    GPIO_Initure.Pull=GPIO_NOPULL;
    GPIO_Initure.Pin=GPIO_PIN_0;
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
}
