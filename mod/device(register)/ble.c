#include "sys.h"
#include "ble.h"
#include "delay.h"
#include "tim.h"
#include <stdio.h>
#include <string.h>

unsigned int USART2_RX_LEN = 0;
unsigned char USART2_RX_STA = 0;//接收状态
unsigned char USART2_RX_BUF[256] = {0};//接收缓冲

//初始化IO 串口1
//bound:波特率
void usart2_init(u32 bound)
{
    float temp;
    unsigned short mantissa;
    unsigned short fraction;
    
    temp = (float) APB1_CLK / (bound * 16);//得到USARTDIV@OVER8=0
    mantissa = temp;
    fraction = (temp - mantissa) * 16;
    mantissa <<= 4;
    mantissa += fraction;
    
    RCC->AHB1ENR |= 0x01;//使能GPIOA时钟
    RCC->APB1ENR |= 1 << 17;//使能USART2时钟
    
    GPIOA->MODER &= ~(0xF << (2 * 2));//清除PA2、PA3状态
    GPIOA->MODER |= 0X0A << (2 * 2);//复用输出
    
    GPIOA->OSPEEDR &= ~(0xF << (2 * 2));//清除PA2、PA3状态
    GPIOA->OSPEEDR |= 0X0A << (2 * 2);//SPEED 50MHz
    
    GPIOA->PUPDR &= ~(0x03 << (2 * 2));//清除状态
    GPIOA->PUPDR |= 0x5 << (2 * 2);//上拉
    
    GPIOA->AFR[0] &= ~(0xFF << (2 * 4));//清除状态
    GPIOA->AFR[0] |= (0x77 << (2 * 4));//复用7
    
    USART2->BRR = mantissa;//设置波特率
    USART2->CR1 &= ~(1 << 15);//16倍过采样
    USART2->CR1 |= 0x3 << 2;//串口接收、发送使能
    USART2->CR1 |= 1 << 5;//使能接收中断
    USART2->CR1 |= 1 << 4;//使能接收空闲中断
    MY_NVIC_SET(4, 0, USART2_IRQn);//配置优先级
    USART2->CR1 |= 1 << 13;//开启串口
}

void USART2_SendData(unsigned char *data, unsigned int len)
{
    unsigned int i;
    for(i = 0; i < len; i++)
    {
        while((USART2->SR & 0X40) == 0);//等待上一个字符发送完成
        USART2->DR = (unsigned char)data[i];
    }
}

//串口中断服务程序
//更换串口后这里记得改
void USART2_IRQHandler(void)
{
    if(USART2->SR & 0x20)//接收到数据
    {
        USART2->SR &= ~0x20;//清除中断标志
        USART2_RX_BUF[USART2_RX_LEN] = USART2->DR;
        USART2_RX_LEN++;
    }
    else if(USART2->SR & 0x10)//空闲中断的时候说明串口没数据了
    {
        USART2->SR;
        USART2->DR;//这两句不知道有啥用
        USART2_RX_STA = 1;//置"1",表示接收完成
    }
}

void BLE_Init(void)
{    
    //GPIO_INIT
    RCC->AHB1ENR |= 1;//使能GPIOA时钟
    GPIOA->MODER &= ~(0x03 << (1 * 2));//输入模式
    GPIOA->OSPEEDR &= ~(0x03 << (13 * 2));//清空状态
    GPIOA->OSPEEDR |= 0x10 << (13 * 2);//快速模式 50MHz
    GPIOA->PUPDR &= ~(0x03 << (13 * 2));//无上下拉
    
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
    unsigned int time = 0;
    while(!USART2_RX_STA)
    {
        if(time > (SystemCoreClock / 2))
        {//超时
            TIM4->CR1 &= ~0x01;//关闭定时器
            printf("BLE timeout\r\n");
            return 0;
        }
        time ++;
    }
    
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
    delay_ms(200);
    if(USART2_RX_STA)
    {
        USART2_RX_STA = 0;
        USART2_RX_LEN = 0;
    }
}












