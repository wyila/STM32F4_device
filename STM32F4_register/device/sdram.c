#include "sdram.h"
#include "delay.h"

//向SDRAM发送命令
//bankx:0,向BANK5上面的SDRAM发送指令
//      1,向BANK6上面的SDRAM发送指令
//cmd:指令(0,正常模式/1,时钟配置使能/2,预充电所有存储区/3,自动刷新/4,加载模式寄存器/5,自刷新/6,掉电)
//refresh:自刷新次数(cmd=3时有效)
//regval:模式寄存器的定义
//返回值:0,正常;1,失败.
u8 SDRAM_Send_Cmd(u8 bankx, u8 cmd, u8 refresh, u16 regval)
{
	u32 retry = 0;
	u32 tempreg = 0;
	tempreg |= cmd << 0;			//设置指令
	tempreg |= 1 <<(4 - bankx);		//设置发送指令到bank5还是6
	tempreg |= refresh << 5;		//设置自刷新次数
	tempreg |= regval << 9;			//设置模式寄存器的值
	FMC_Bank5_6->SDCMR = tempreg;	//配置寄存器
	while((FMC_Bank5_6->SDSR & (1 << 5)))//等待指令发送完成
	{
		retry ++;
		if(retry > 0X1FFFFF)
            return 1;
	}
	return 0;
}

//SDRAM初始化
void SDRAM_Init(void)
{
	u32 sdctrlreg = 0, sdtimereg = 0;
	u16 mregval = 0;
    volatile u32 reg_addr = 0;
    
    
	RCC->AHB3ENR |= 1;//使能FMC时钟
	RCC->AHB1ENR |= 0X1F << 2;//使能PC/PD/PE/PF/PG时钟

    //GPIOC 0 2 3
    GPIOC->MODER &= ~0xF3;//clear status
    GPIOC->MODER |= 0xA2;//AF mode
    GPIOC->OSPEEDR |= 0xF3;//speed 100MHz
    GPIOC->PUPDR &= ~0xF3;
    GPIOC->PUPDR |= 0x51;//上拉
    GPIOC->AFR[0] &= ~(0xFF0F);
    GPIOC->AFR[0] |= 0x0000CC0C;//AF12
    
    //GPIOD 0 1 8 9 10 14 15
    GPIOD->MODER &= 0x0FC0FFF0;//clear status
    GPIOD->MODER |= 0xA02A000A;//AF mode
    GPIOD->OSPEEDR |= 0xF03F000F;//speed 100MHz
    GPIOD->PUPDR &= 0x0FC0FFF0;
    GPIOD->PUPDR |= 0x50150005;//上拉
    GPIOD->AFR[0] &= ~(0xFF);//PD0 1
    GPIOD->AFR[0] |= 0x000000CC;//AF12
    GPIOD->AFR[1] &= 0x00FFF000;//PD8 9 10 14 15
    GPIOD->AFR[1] |= 0xCC000CCC;//AF12
    
    //GPIOE 0 1 7 8 9 10 11 12 13 14 15
    GPIOE->MODER &= 0x00003FF0;
    GPIOE->MODER |= 0xAAAA800A;//AF mode
    GPIOE->OSPEEDR |= 0xFFFFC00F;//Speed 100MHz
    GPIOE->PUPDR &= 0x00003FF0;
    GPIOE->PUPDR |= 0x55554005;//up
    GPIOE->AFR[0] &= 0x0FFFFF00;//Pin 0 1 7
    GPIOE->AFR[0] |= 0xC00000CC;//AF12
    GPIOE->AFR[1] &= 0x00000000;//Pin 8 9 10 11 12 13 14 15
    GPIOE->AFR[1] |= 0xCCCCCCCC;
    
    //GPIOF 0 1 2 3 4 5 11 12 13 14 15
    GPIOF->MODER &= 0x003FF000;
    GPIOF->MODER |= 0xAA800AAA;//AF mode
    GPIOF->OSPEEDR |= 0xFFC00FFF;//Speed 100MHz
    GPIOF->PUPDR &= 0x003FF000;
    GPIOF->PUPDR |= 0x55400555;//up
    GPIOF->AFR[0] &= 0xFF000000;//Pin 0 1 2 3 4 5
    GPIOF->AFR[0] |= 0x00CCCCCC;//AF12
    GPIOF->AFR[1] &= 0x00000FFF;//Pin 11 12 13 14 15
    GPIOF->AFR[1] |= 0xCCCCC000;
    
    //GPIOG 0 1 2 4 5 8 15
    GPIOG->MODER &= 0x3FFCF0C0;
    GPIOG->MODER |= 0x80020A2A;//AF mode
    GPIOG->OSPEEDR |= 0xC0030F3F;//Speed 100MHz
    GPIOG->PUPDR &= 0x3FFCF0C0;
    GPIOG->PUPDR |= 0x40010515;//up
    GPIOG->AFR[0] &= 0xFF00F000;//Pin 0 1 2 4 5
    GPIOG->AFR[0] |= 0x00CC0CCC;//AF12
    GPIOG->AFR[1] &= 0x0FFFFFF0;//Pin 8 15
    GPIOG->AFR[1] |= 0xC000000C;
    
 	sdctrlreg |= 1 << 0;//9位列地址
	sdctrlreg |= 2 << 2;//13位行地址
	sdctrlreg |= 1 << 4;//16位数据位宽
	sdctrlreg |= 1 << 6;//4个内部存区(4 BANKS)
	sdctrlreg |= 2 << 7;//2个CAS延迟
	sdctrlreg |= 0 << 9;//允许写访问
	sdctrlreg |= 2 << 10;//SDRAM时钟=HCLK/2=192M/2=96M=10.4ns
	sdctrlreg |= 1 << 12;//使能突发访问
	sdctrlreg |= 0 << 13;//读通道延迟0个HCLK
 	FMC_Bank5_6->SDCR[0] = sdctrlreg;//设置FMC BANK5 SDRAM控制寄存器(BANK5和6用于管理SDRAM).

	sdtimereg |= 1 << 0;//加载模式寄存器到激活时间的延迟为2个时钟周期
	sdtimereg |= 6 << 4;//退出自刷新延迟为7个时钟周期
	sdtimereg |= 5 << 8;//自刷新时间为6个时钟周期
	sdtimereg |= 5 << 12;//行循环延迟为6个时钟周期
	sdtimereg |= 1 << 16;//恢复延迟为2个时钟周期
	sdtimereg |= 1 << 20;//行预充电延迟为2个时钟周期
	sdtimereg |= 1 << 24;//行到列延迟为2个时钟周期
 	FMC_Bank5_6->SDTR[0] = sdtimereg;//设置FMC BANK5 SDRAM时序寄存器

	SDRAM_Send_Cmd(0, 1, 0, 0);//时钟配置使能
	delay_us(500);//至少延迟200us.
	SDRAM_Send_Cmd(0, 2, 0, 0);//对所有存储区预充电
	SDRAM_Send_Cmd(0, 3, 8, 0);//设置自刷新次数
	mregval |= 1 << 0;//设置突发长度:1(可以是1/2/4/8)
	mregval |= 0 << 3;//设置突发类型:连续(可以是连续/交错)
	mregval |= 2 << 4;//设置CAS值:2(可以是2/3)
	mregval |= 0 << 7;//设置操作模式:0,标准模式
	mregval |= 1 << 9;//设置突发写模式:1,单点访问
	SDRAM_Send_Cmd(0, 4, 0, mregval);//设置SDRAM的模式寄存器

	//刷新频率计数器(以SDCLK频率计数),计算方法:
	//COUNT=SDRAM刷新周期/行数-20=SDRAM刷新周期(us)*SDCLK频率(Mhz)/行数
	//我们使用的SDRAM刷新周期为64ms,SDCLK=192/2=96Mhz,行数为8192(2^13).
	//所以,COUNT=64*1000*96/8192-20=730
	FMC_Bank5_6->SDRTR = 730 << 1;//设置刷新频率计数器
}

//在指定地址(WriteAddr+Bank5_SDRAM_ADDR)开始,连续写入n个字节.
//pBuffer:字节指针
//WriteAddr:要写入的地址
//n:要写入的字节数
void FMC_SDRAM_WriteBuffer(u8 *pBuffer, u32 WriteAddr, u32 n)
{
	while(n--)
	{
		*(vu8*)(Bank5_SDRAM_ADDR+WriteAddr) = *pBuffer;
		WriteAddr++;
		pBuffer++;
	}
}

//在指定地址((WriteAddr+Bank5_SDRAM_ADDR))开始,连续读出n个字节.
//pBuffer:字节指针
//ReadAddr:要读出的起始地址
//n:要写入的字节数
void FMC_SDRAM_ReadBuffer(u8 *pBuffer, u32 ReadAddr, u32 n)
{
	while(n--)
	{
		*pBuffer++ = *(vu8*)(Bank5_SDRAM_ADDR+ReadAddr);
		ReadAddr++;
	}
}






























