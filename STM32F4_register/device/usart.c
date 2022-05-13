#include "usart.h"
#include "delay.h"

//记得改中断函数、使能的时钟！！！！！
#define USART_GPIO  GPIOB //一般在同一组GPIO
#define USART_TX    GPIO_PIN_10
#define USART_RX    GPIO_PIN_11
#define USART       USART3

/**************************************************************/
//加入以下代码,支持printf函数,而不需要选择use MicroLIB
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
	int handle;
};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}
//重定义fputc函数
int fputc(int ch, FILE *f)
{
	while((USART->SR & 0X40) == 0);//循环发送,直到发送完毕
	USART->DR = (u8) ch;
	return ch;
}
/***************************************************************/

//注意,读取USARTx->SR能避免莫名其妙的错误
unsigned char USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.
unsigned short USART_RX_LEN = 0;
unsigned short USART_RX_STA = 0;           //接收状态标记

//初始化IO 串口1
//bound:波特率
void usart_init(u32 bound)
{
    float temp;
    unsigned short mantissa;
    unsigned short fraction;
    
    temp = (float) APB1_CLK / (bound * 16);//得到USARTDIV@OVER8=0
    mantissa = temp;//整数
    fraction = (temp - mantissa) * 16;//小数
    mantissa <<= 4;
    mantissa += fraction;
    
    RCC->AHB1ENR |= 0x02;//使能GPIOB时钟
    RCC->APB1ENR |= 1 << 18;//使能USART3时钟
    
    USART_GPIO->MODER &= ~(0xF << (10 * 2));//清除PB10、PB11状态
    USART_GPIO->MODER |= 0X0A << (10 * 2);//复用输出
    
    USART_GPIO->OSPEEDR &= ~(0xF << (10 * 2));//清除PB10、PB11状态
    USART_GPIO->OSPEEDR |= 0X0A << (10 * 2);//SPEED 50MHz
    
    USART_GPIO->PUPDR &= ~(0x03 << (10 * 2));//清除状态
    USART_GPIO->PUPDR |= 0x5 << (10 * 2);//上拉
    
    USART_GPIO->AFR[1] &= ~(0xFF << ((10 - 8) * 4));//清除状态
    USART_GPIO->AFR[1] |= (0x77 << ((10 - 8) * 4));//复用7
    
    USART->BRR = mantissa;//设置波特率
    USART->CR1 &= ~(1 << 15);//16倍过采样
    USART->CR1 |= 0x3 << 2;//串口接收、发送使能
    USART->CR1 |= 1 << 5;//使能接收中断
    USART->CR1 |= 1 << 4;//使能接收空闲中断
    MY_NVIC_SET(2, 0, USART3_IRQn);//配置优先级
    USART->CR1 |= 1 << 13;//开启串口
}



//串口中断服务程序
//更换串口后这里记得改
void USART3_IRQHandler(void)
{
    if(USART->SR & 0x20)//接收到数据
    {
        USART->SR &= ~0x20;//清除中断标志
        USART_RX_BUF[USART_RX_LEN] = USART->DR;
        USART_RX_LEN++;
    }
    else if(USART->SR & 0x10)//空闲中断的时候说明串口没数据了
    {
        USART->SR;
        USART->DR;//这两句不知道有啥用
        USART_RX_STA = 1;//置"1",表示接收完成
    }
}






