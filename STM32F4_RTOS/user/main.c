#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmd.h"
#include "tim.h"
#include "i2c.h"
#include "sdram.h"
#include "w25qxx.h"
#include "key.h"

#include <string.h>
#include <stdio.h>

TaskHandle_t StartTask_Handler1;
void start_task1(void *Paramenters)
{
    printf("usart3 task\r\n");
    portENABLE_INTERRUPTS();//打开中断
    SDRAM_Init();//内部调用延时函数，所以放到这里初始化
    while(1)
    {
        if(USART_RX_STA)
        {
            cmd_proc(USART_RX_BUF, USART_RX_LEN);
            //printf("your send data is: %s\r\n", USART_RX_BUF);
            memset(USART_RX_BUF, 0, USART_RX_LEN);
            USART_RX_LEN = 0;
            USART_RX_STA = 0;
        }
        vTaskSuspend(StartTask_Handler1);//任务挂起
    }
}

TaskHandle_t StartTask_Handler2;
void start_task2(void *Paramenters)
{
    while(1)
    {
        GPIOC->ODR ^= GPIO_PIN_13;
        vTaskDelay(500);
    }
}

int main(void)
{
    SCB->VTOR = APP_ADDR;
    HAL_Init();                     //初始化HAL库
    Stm32_Clock_Init(360,25,2,8);   //设置时钟,180Mhz
    delay_init(180);                //初始化延时函数
    uart_init(115200);
    LED_Init();                     //初始化LED
    W25QXX_Init();
    Key_init();
    
    //TIM_Init();
    
    
    printf("This is RTOS\r\n");
    
    xTaskCreate((TaskFunction_t) start_task1,   //任务函数
                (const char*) "task_1",         //任务名称
                (u16)256,                       //任务栈大小
                (void *)NULL,                   //传递给任务函数的参数
                (UBaseType_t)1,                 //任务优先级
                (TaskHandle_t *)&StartTask_Handler1);//任务句柄
    
    xTaskCreate((TaskFunction_t) start_task2,   //任务函数
                (const char*) "task_2",         //任务名称
                (u16)256,                       //任务栈大小
                (void *)NULL,                   //传递给任务函数的参数
                (UBaseType_t)1,                 //任务优先级
                (TaskHandle_t *)&StartTask_Handler2);//任务句柄
    
    vTaskStartScheduler();//开始任务调度
    
    while(1)
    {
        if(USART_RX_STA)
        {
            printf("You send data is: %s", USART_RX_BUF);
            USART_RX_LEN = 0;
            GPIOC->ODR ^= GPIO_PIN_13;
            USART_RX_STA = 0;
        }
    }
    /*
    
    //uart_init(115200);              //初始化串口
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

        //delay_ms(200);
    }
    */
    
 }





