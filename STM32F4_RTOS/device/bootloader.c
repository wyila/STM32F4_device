#include "bootloader.h"
#include "sys.h"
#include "flash.h"
#include "usart.h"
#include "delay.h"
#include "tim.h"
#include <string.h>
#include <stdio.h>

user_app app_addr;

//数据包信息格式
//bit 0~3:数据包大小;
//bit 4~8数据包版本（'v'开头，eg:v1.1.1 ）;
//bit else:芯片型号
int APP_update(void)
{
    unsigned char Xmodem_val[2] = {0};
    unsigned int cnt = 0;
    unsigned short res = 0;
    unsigned int app_addr = APP_ADDR;
    unsigned int app_size = 0;
    unsigned char app_ver[6] = {0};
    unsigned char CHIP[14] = "STM32F429IGTx";
    unsigned int i;
    unsigned char *Packet_cache = (unsigned char *)0x20004000;
    unsigned char packet_info[128] = {0};

    for(i = 0;i < 0x2C000; i++)
        Packet_cache[i] = 0xFF;
    printf("C");
    __HAL_RCC_TIM3_CLK_ENABLE();//使能TIM3时钟
    while(1)
    {
        if(TIM3_IT_STATUS)
        {
            TIM3_IT_STATUS = 0;
            printf("C");
            cnt ++;
        }
        if(USART_RX_STA)//接收到数据包的信息
        {
            //固件包大小
            app_size |= USART_RX_BUF[0] << 24;
            app_size |= USART_RX_BUF[0] << 16;
            app_size |= USART_RX_BUF[0] << 8;
            app_size |= USART_RX_BUF[0];

            if(app_size > (FLASH_SIZE - 0x8000))
            {
                uart_send_byte(CAN);
                printf("Firmware package too large!!!!");
                return 0;
            }

            //固件包版本
            for(i = 0; i < 5; i++)
                app_ver[i] = USART_RX_BUF[i + 4];
            //芯片型号
            for(i = 9; i < USART_RX_LEN; i++)
                CHIP[i - 9] = USART_RX_BUF[i];
            //复位
            for(i = 0; i < USART_RX_LEN; i++)
                USART_RX_BUF[i] = 0;
            sprintf((char *)packet_info, "Packet version: %s  Packet size:%d  Chip: %s", app_ver, app_size, CHIP);
            USART_RX_LEN = 0;
            USART_RX_STA = 0;
            printf("C");
            break;
        }
        if(cnt >= 60)//超时
        {
            __HAL_RCC_TIM3_CLK_DISABLE();
            return 0;
        }
    }

    __HAL_RCC_TIM3_CLK_DISABLE();//关闭定时器3

    cnt = 0;
    while(1)
    {
        if(USART_RX_STA)
        {
            if(USART_RX_LEN == 1)//接收到结束标志
            {
                USART_RX_LEN = 0;
                USART_RX_STA = 0;
                if(USART_RX_BUF[0] == EOT)
                {
                    uart_send_byte(ACK);
                    break;
                }
            }
            //正常接收数据
            res = XmodemCrc16(&USART_RX_BUF[3], 128);
            Xmodem_val[0] = res & 0xff;
            Xmodem_val[1] = (res >> 8) & 0xff;
            if((Xmodem_val[0] == USART_RX_BUF[132]) && (Xmodem_val[1] == USART_RX_BUF[133]))
            {   //验证通过
                //放入缓存
                for(i = 0; i < 128; i++)
                {
                    Packet_cache[cnt] = USART_RX_BUF[i + 3];
                    cnt ++;
                }
                if(cnt > (0x30000 - 128 - 0x4000))//缓存溢出
                {
                    //如果缓存溢出，直接将数据写入flash
                    Write_FLASH(app_addr, (unsigned int *)Packet_cache, cnt / 4);
                    app_addr += (0x30000 - 0x4000);
                    cnt = 0;
                }
                uart_send_byte(ACK);//回复
            }
            else
                uart_send_byte(NAK);//回复错误信号，重新发送这一帧数据
            USART_RX_LEN = 0;
            USART_RX_STA = 0;
        }
        //全部接收完成
        Write_FLASH(app_addr, (unsigned int *)Packet_cache, (cnt / 4) + 1);
        //将固件包信息写入相应地址
        Write_FLASH(APP_ADDR - 128, (unsigned int *)Packet_cache, 128 / 4);
    }

    return 1;
}


//跳转到应用程序段
//appxaddr:用户代码起始地址.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr) & 0x2FFE0000) == 0x20000000)	//检查栈顶地址是否合法.
	{
		app_addr=(user_app) * (vu32*)(appxaddr + 4);		//用户代码区第二个字为程序开始地址(复位地址)
		MSR_MSP(*(vu32*)appxaddr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
        app_addr();									//跳转到APP.
	}
}

unsigned short XmodemCrc16(unsigned char *ptr, int len)
{
    unsigned int i;
    unsigned short crc = 0x0000;

    while(len--)
    {
        crc ^= (unsigned short)(*ptr++) << 8;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}







