#ifndef _USART_H
#define _USART_H
#include "sys.h"
#include "stdio.h"

#define USART_REC_LEN  			256  	//定义最大接收字节数 256

extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
extern u16 USART_RX_STA;         		//接收状态标记
extern u16 USART_RX_LEN;                //接收了多少个数据


//如果想串口中断接收，请不要注释以下宏定义
void usart_init(u32 bound);

#endif