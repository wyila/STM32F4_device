#include "sys.h"
#include "gpio.h"


void GPIO_test_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_Initure.Pin = GPIO_PIN_1;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;    //通用推挽输出
    GPIO_Initure.Pull = GPIO_NOPULL;            //No Pull-up or Pull-down activation
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;       //高速

    HAL_GPIO_Init(GPIOC,&GPIO_Initure);

}












