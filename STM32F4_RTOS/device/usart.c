#include "usart.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t StartTask_Handler1;
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
u8 USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.
u16 USART_RX_LEN = 0;
u16 USART_RX_STA = 0;           //接收状态标记

u8 aRxBuffer[RXBUFFERSIZE];//HAL库使用的串口接收缓冲
UART_HandleTypeDef UART_Handler; //UART句柄

//发送一个字节
void uart_send_byte(unsigned char data)
{
    while((USART->SR & 0X40) == 0);//发送成功则退出循环,由硬件置"1"
    USART->DR = (unsigned char)data;
}

//初始化IO 串口1
//bound:波特率
void uart_init(u32 bound)
{
	//UART 初始化设置
	UART_Handler.Instance = USART;				    //USART3
	UART_Handler.Init.BaudRate = bound;			    //波特率
	UART_Handler.Init.WordLength = UART_WORDLENGTH_8B;  //字长为8位数据格式
	UART_Handler.Init.StopBits = UART_STOPBITS_1;       //一个停止位
	UART_Handler.Init.Parity = UART_PARITY_NONE;        //无奇偶校验位
	UART_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;  //无硬件流控
	UART_Handler.Init.Mode = UART_MODE_TX_RX;           //收发模式
	HAL_UART_Init(&UART_Handler);					    //HAL_UART_Init()会使能UART1

    //该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量
	HAL_UART_Receive_IT(&UART_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE);
    USART->CR1 |= (1 << 4);//使能空闲中断
}

//UART底层初始化，时钟使能，引脚配置，中断配置
//此函数会被HAL_UART_Init()调用
//huart:串口句柄
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure;

	if(huart->Instance == USART)//如果是串口3，进行串口3 MSP初始化
	{
        //更换串口后这里记得改
		__HAL_RCC_GPIOB_CLK_ENABLE();			//使能GPIOB时钟
		__HAL_RCC_USART3_CLK_ENABLE();			//使能USART3时钟

		GPIO_Initure.Pin = USART_TX;		    	//PB10
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;		//复用推挽输出
		GPIO_Initure.Pull = GPIO_PULLUP;			//上拉
		GPIO_Initure.Speed = GPIO_SPEED_FAST;		//高速
		GPIO_Initure.Alternate = GPIO_AF7_USART3;	//复用为USART3
		HAL_GPIO_Init(USART_GPIO, &GPIO_Initure);         //初始化PB10

		GPIO_Initure.Pin = USART_RX; 			//PB11
		HAL_GPIO_Init(USART_GPIO, &GPIO_Initure);	//初始化PB11

        //更换串口后这里记得改
		HAL_NVIC_EnableIRQ(USART3_IRQn);				//使能USART2中断通道
		HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);		//抢占优先级5，子优先级3
	}
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
        xTaskResumeFromISR(StartTask_Handler1);//恢复任务1
    }
    /*
	unsinged int timeout=0;

	HAL_UART_IRQHandler(&UART2_Handler);	//调用HAL库中断处理公用函数

	timeout=0;
    while (HAL_UART_GetState(&UART2_Handler) != HAL_UART_STATE_READY)//等待就绪
	{
	 timeout++;////超时处理
     if(timeout>HAL_MAX_DELAY) break;

	}

	timeout=0;
	while(HAL_UART_Receive_IT(&UART2_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE) != HAL_OK)//一次处理完成之后，重新开启中断并设置RxXferCount为1
	{
	 timeout++; //超时处理
	 if(timeout>HAL_MAX_DELAY) break;
	}
    */
}
/*
中断进行完之后，并不会直接退出，而是会进入中断回调函数中
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	if(huart->Instance == USART2)//如果是串口2
	{
		if((USART_RX_STA & 0x8000) == 0)//接收未完成
		{
			if(USART_RX_STA & 0x4000)//接收到了0x0d
			{
				if(aRxBuffer[0] != 0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了
			}
			else //还没收到0X0D
			{
				if(aRxBuffer[0]==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=aRxBuffer[0] ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收
				}
			}
		}

	}

}
*/
/*下面代码我们直接把中断控制逻辑写在中断服务函数内部。*/
/*


//串口1中断服务程序
void USART1_IRQHandler(void)
{
	unsigned char Res;
#if SYSTEM_SUPPORT_OS	 	//使用OS
	OSIntEnter();
#endif
	if((__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_RXNE)!=RESET))  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
        HAL_UART_Receive(&UART1_Handler,&Res,1,1000);
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了
			}
			else //还没收到0X0D
			{
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收
				}
			}
		}
	}
	HAL_UART_IRQHandler(&UART1_Handler);
#if SYSTEM_SUPPORT_OS	 	//使用OS
	OSIntExit();
#endif
}
#endif
*/





