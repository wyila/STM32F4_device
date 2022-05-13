#include "spi.h"
#include "sys.h"

void SPI5_Init(void)
{
	unsigned short tempreg = 0;
	RCC->AHB1ENR |= 1 << 5;    	//使能PORTF时钟
	RCC->APB2ENR |= 1 << 20;   	//SPI5时钟使能
	
    GPIOF->MODER &= ~(0x3F << (7 * 2));//清除PF789状态
    GPIOF->MODER |= 0X2A << (7 * 2);//复用输出
    
    GPIOF->OSPEEDR |= 0X3F << (7 * 2);//SPEED 100MHz
    
    GPIOF->PUPDR &= ~(0x3F << (7 * 2));//清除状态
    GPIOF->PUPDR |= 0x15 << (7 * 2);//上拉
    
    GPIOF->AFR[0] &= ~0xF0000000;//清除状态
    GPIOF->AFR[0] |= (0x7 << (7 * 4));//PF7复用5
    GPIOF->AFR[1] &= ~0xFF;//清除状态
    GPIOF->AFR[1] |= 0x55;//PF8 PF9复用5

	//这里只针对SPI口初始化
	RCC->APB2RSTR |= 1 << 20;	//复位SPI5
	RCC->APB2RSTR &= ~(1 << 20);//停止复位SPI5
	tempreg |= 0 << 10;			//全双工模式
	tempreg |= 1 << 9;			//软件nss管理
	tempreg |= 1 << 8;
	tempreg |= 1 << 2;			//SPI主机
	tempreg |=0 << 11;			//8位数据格式
	tempreg |=1 << 1;			//空闲模式下SCK为1 CPOL=1
	tempreg |=1 << 0;			//数据采样从第2个时间边沿开始,CPHA=1
 	//对SPI5属于APB2的外设.时钟频率最大为96Mhz频率.
	tempreg |=7 << 3;			//Fsck=Fpclk1/256
	tempreg |=0 << 7;			//MSB First
	tempreg |=1 << 6;			//SPI启动
	SPI5->CR1 = tempreg; 		//设置CR1
	SPI5->I2SCFGR &= ~(1 << 11);//选择SPI模式
	SPI5_ReadWriteByte(0xff);//启动传输
}
//SPI5速度设置函数
//SpeedSet:0~7
//SPI速度=fAPB2/2^(SpeedSet+1)
//fAPB2时钟一般为90Mhz
void SPI5_SetSpeed(unsigned char SpeedSet)
{
	SpeedSet &= 0X07;			//限制范围
	SPI5->CR1 &= 0XFFC7;
	SPI5->CR1 |= SpeedSet << 3;	//设置SPI5速度
	SPI5->CR1 |= 1 << 6; 		//SPI设备使能
}
//SPI5 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
unsigned char SPI5_ReadWriteByte(unsigned char TxData)
{
	while((SPI5->SR & 1 << 1) == 0);		//等待发送区空
	SPI5->DR = TxData;	 	  		//发送一个byte
	while((SPI5->SR & 1)==0);		//等待接收完一个byte
 	return SPI5->DR;          		//返回收到的数据
}







