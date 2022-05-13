#include "sys.h"
#include "key.h"

#define EXIT_KEY 0

void Key_init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOA_CLK_ENABLE();           //开启GPIOB时钟

#if EXIT_KEY
    GPIO_Initure.Pin=GPIO_PIN_0;                //PA0
    GPIO_Initure.Mode=GPIO_MODE_IT_RISING;      //上升沿触发
    GPIO_Initure.Pull=GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
    //中断线0-PA0
    HAL_NVIC_SetPriority(EXTI0_IRQn,2,0);       //抢占优先级为2，子优先级为0
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);             //使能中断线0
#else
    GPIO_Initure.Pin = GPIO_PIN_0; //PA0
    GPIO_Initure.Mode = GPIO_MODE_INPUT;  //输入模式
    GPIO_Initure.Pull = GPIO_NOPULL;          //浮空
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);
#endif    
}

//中断服务函数
void EXTI0_IRQHandler(void)
{
    printf("Key EXTI\r\n");
    EXTI->PR |= 1;//清除中断
}
