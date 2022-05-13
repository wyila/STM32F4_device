#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "flash.h"
#include "tim.h"
#include "i2c.h"
#include "bootloader.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    unsigned char update;
    HAL_Init();                     //初始化HAL库
    Stm32_Clock_Init(360,25,2,8);   //设置时钟,180Mhz
    delay_init(180);                //初始化延时函数
    LED_Init();                     //初始化LED
    uart_init(115200);              //初始化串口
    TIM3_Init(10000, 9000);         //定时器时钟为90MHz
    IIC_Init();
    GPIOC->ODR |= GPIO_PIN_13;//LED亮
    IIC_ReadData(0x00, &update, 1);
    
    if(PBin(0))
        update = 1;
    
    
    printf("bootloader\r\n");
    printf("update data: 0x%02x\r\n", update);

    if(!update)
    {
        update = 0;
        IIC_WriteData(0x00, &update, 1);
        iap_load_app(APP_ADDR);
    }
	while(1)
	{
        APP_update();
        delay_ms(100);
        GPIOC->ODR ^= GPIO_PIN_13;
    }
 }





