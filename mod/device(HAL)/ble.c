#include "sys.h"
#include "ble.h"
#include "usart.h"
#include "tim.h"
#include <stdio.h>
#include <string.h>


void BLE_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOB_CLK_ENABLE();			//使能GPIOB时钟
    __HAL_RCC_USART3_CLK_ENABLE();			//使能USART3时钟

    GPIO_Initure.Pin = GPIO_PIN_4;	//PA1、PA4
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;    //通用推挽输出
    GPIO_Initure.Pull = GPIO_NOPULL;			//no pull
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;		//高速
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);
    
    GPIO_Initure.Pin = GPIO_PIN_1;
    GPIO_Initure.Mode = GPIO_MODE_INPUT;    //浮空输入
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);
    
    TIM4_Init(9999, SystemCoreClock / 2 / 10000 - 1);//初始化定时器4
    TIM4->CR1 &= ~0x01;//关闭定时器
    scan_bound();
    BLE_SEND_CMD(BLE_CFG, NULL);//进入配置状态
    BLE_wait(BLE_ACK_CFG);
    BLE_SEND_CMD(BLE_ENTM, NULL);//进入透传模式
    BLE_wait(BLE_ACK);
}

int BLE_SEND_CMD(u8 *CMD, u8 *VAL)
{
    unsigned char cmd[32] = {0};
    if(CMD == NULL)
        return 0;
    else if(!memcmp(CMD, BLE_CFG, 4))//进入配置命令比较特殊
    {
        USART2_SendData((u8 *)BLE_CFG, 4);
        return 1;
    }
    if(VAL == NULL)
        sprintf((char *)cmd, "%s\r\n", CMD);
    else
        sprintf((char *)cmd, "%s%s\r\n", CMD, VAL);
    USART2_SendData(cmd, strlen((char *)cmd));
    
    return 1;
}

int BLE_wait(unsigned char *ack)
{
    TIM4->CNT = 0;
    TIM4->CR1 |= 0x01;//打开定时器
    while(!USART2_RX_STA)
        if(TIM4->CNT > 4999)//定时器频率10000Hz
        {//超时
            TIM4->CR1 &= ~0x01;//关闭定时器
            printf("BLE timeout\r\n");
            return 0;
        }
    TIM4->CR1 &= ~0x01;//关闭定时器
    USART2_RX_STA = 0;
    USART2_RX_LEN = 0;
    if(!memcmp(USART2_RX_BUF, ack, strlen((char *)ack)))
        return 1;
    return 0;
}

int scan_bound(void)//初始化
{
    unsigned int bound[16] = {115200, 921600, 460800, 256000, \
                              230400, 128000, 76800 , 57600 , \
                              43000 , 38400 , 19200 , 14400 , \
                              9600  , 4800  , 2400  , 1200 
    };
    
    int i;
    for(i = 0; i < 16; i++)
    {
        usart2_init(bound[i]);
        BLE_SEND_CMD(BLE_RESET, NULL);//复位
        ignore_ack();
        BLE_SEND_CMD(BLE_CFG, NULL);//进入配置
        if(BLE_wait(BLE_ACK_CFG))
        {
            printf("BLE bound: %d\r\n", bound[i]);
            if(bound[i] != 115200)
            {
                BLE_SEND_CMD(BLE_UART, (u8 *)"115200,8,0,1");
                BLE_wait((u8 *)"\r\n+115200,8,0,1\r\nOK\r\n");
                printf("set bound 115200\r\n");
            }
            break;
        }
    }
    BLE_SEND_CMD(BLE_RESET, NULL);
    return BLE_wait(BLE_ACK);
}

void ignore_ack(void)
{
    TIM4->CNT = 0;
    TIM4->CR1 |= 0x01;//打开定时器
    while(!USART2_RX_STA)
        if(TIM4->CNT > 1999)//定时器频率10000Hz
            break;
    TIM4->CR1 &= ~0x01;//关闭定时器
    if(USART2_RX_STA)
    {
        USART2_RX_STA = 0;
        USART2_RX_LEN = 0;
    }
}












