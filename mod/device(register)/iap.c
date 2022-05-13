#include "iap.h"
#include "sys.h"
#include "usart.h"

user_app app_addr;

void JUMP_TO_BOOTLOADER(void)
{
    printf("jump to bootloader\r\n");
    iap_load_app(0x08000000);
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







