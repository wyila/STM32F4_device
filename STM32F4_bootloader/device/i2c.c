#include "i2c.h"
#include "delay.h"

void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOH_CLK_ENABLE();   //使能GPIOH时钟

    //PH4,5初始化设置
    GPIO_Initure.Pin = GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull = GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed = GPIO_SPEED_FAST;     //快速
    HAL_GPIO_Init(GPIOH, &GPIO_Initure);

    IIC_SDA = 1;
    IIC_SCL = 1;
}

//产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	IIC_SDA = 1;
	IIC_SCL = 1;
	delay_us(4);
 	IIC_SDA = 0;//START:when CLK is high,DATA change form high to low
	delay_us(4);
	IIC_SCL = 0;//钳住I2C总线，准备发送或接收数据
}

//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL = 0;
	IIC_SDA = 0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL = 1;
	delay_us(4);
	IIC_SDA = 1;//发送I2C总线结束信号
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime = 0;
	SDA_IN();      //SDA设置为输入
	IIC_SDA = 1;
    delay_us(2);
	IIC_SCL = 1;
    delay_us(2);
	while(READ_SDA)
	{
		ucErrTime ++;
		if(ucErrTime > 250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL = 0;//时钟输出0
    delay_us(1);
	return 0;
}

//产生ACK应答
void IIC_Ack(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 0;
	delay_us(2);
	IIC_SCL = 1;
	delay_us(2);
	IIC_SCL = 0;
    delay_us(2);
}

//不产生ACK应答
void IIC_NAck(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 1;
	delay_us(2);
	IIC_SCL = 1;
	delay_us(2);
	IIC_SCL = 0;
    delay_us(2);
}

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
void IIC_Send_Byte(u8 data)
{
    u8 i;
	SDA_OUT();
    //IIC_SCL = 0;//拉低时钟开始数据传输
    
    for(i = 0; i < 8; i++)
    {
        IIC_SDA = (data >> (7 - i)) & 0x01;
        delay_us(1);
        
        IIC_SCL = 1;
        delay_us(3);
        IIC_SCL = 0;
        delay_us(2);
    }
}

//读1个字节，ack = 1时，发送ACK，ack = 0，发送nACK
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i, receive = 0;
	SDA_IN();//SDA设置为输入
    
    for(i = 0; i < 8; i++)
	{
        IIC_SCL = 0;
        delay_us(2);
		IIC_SCL = 1;
        receive <<= 1;
        if(READ_SDA)
            receive |= 1;
		delay_us(1);
    }
    if(!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK
    return receive;
}

int IIC_ReadData(unsigned char subaddr, unsigned char *data, unsigned int len)
{
    unsigned int i;
    //发送设备地址以及要读的数据地址,相当于写数据
    IIC_Start();
    IIC_Send_Byte(IIC_ADDR << 1);//write
    IIC_Wait_Ack();
    IIC_Send_Byte(subaddr);//read addr
    IIC_Wait_Ack();
    
    IIC_Start();
    IIC_Send_Byte((IIC_ADDR << 1) | 1);//read
    IIC_Wait_Ack();
    for(i = 0; i < len - 1; i++)
        data[i] = IIC_Read_Byte(1);
    data[i] = IIC_Read_Byte(0);
    i++;
    IIC_Stop();//停止
    return i;
}

int IIC_WriteData(unsigned char subaddr, unsigned char *data, int len)
{
    unsigned int i;
    IIC_Start();
    IIC_Send_Byte(IIC_ADDR << 1);//write
    IIC_Wait_Ack();
    IIC_Send_Byte(subaddr);//write addr
    IIC_Wait_Ack();
    
    for(i = 0; i < len; i++)
    {
        IIC_Send_Byte(data[i]);
        if(IIC_Wait_Ack())
            return i;
    }
    IIC_Stop();//停止
    return 0;
}
