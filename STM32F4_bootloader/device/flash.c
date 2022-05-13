#include "sys.h"
#include "flash.h"
#include "delay.h"

/***************************************************************************
  先说原理，FLASH写入数据其实就是对FLASH置零操作，FLASH被置"0"后不能被置"1"，
用逻辑关系来说就是旧数据与上新数据(old & new)，因此，如果想写入的地方有数据
就需要进行复位，把原本为"0"的值复位为"1"，也就是擦除操作。
  其实不需要进行擦除也可以写入数据，但是要保证原本的数据能够转变为写入的值。
  例如,FLASH中原本保存的数据为0x06(0b0110)你往这个位置写入0x2(0b0010)，可以看
到，状态改变的地方只有第三位(0x06&0x02=0x02)，因此是可以正常写入的。但是如果
是下面的情况old_data = 0x06(0b0110), new_data = 0xc(0b1100),那么根据逻辑"与"
写入后，实际存在FLASH中的值就是0x06 & 0x0c = 0x04(0b0100)，这就是为什么需要
擦除旧数据的原因
*****************************************************************************/

//读取指定地址的字(32位数据)
//faddr:读地址
//返回值:对应数据.
u32 STMFLASH_ReadWord(u32 faddr)
{
	return *(vu32*)faddr;
}

//获取某个地址所在的flash扇区
//addr:flash地址
//返回值:0~11,即addr所在的扇区
u8 GetFlashSector(u32 addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return FLASH_SECTOR_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_SECTOR_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_SECTOR_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_SECTOR_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_SECTOR_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_SECTOR_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_SECTOR_6;
	else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_SECTOR_7;
	else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_SECTOR_8;
	else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_SECTOR_9;
	else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_SECTOR_10;
	return FLASH_SECTOR_11;
}

int FLASH_WriteWord(u32 addr, u32 data)
{
    FLASH->CR &= ~FLASH_CR_PSIZE;//清除PSIZE原来的设置
    FLASH->CR |= 0x00000200; //设置为32bit宽,确保VCC=2.7~3.6V之间
    FLASH->CR |= FLASH_CR_PG;//编程使能
    *(vu32*)addr = data;	//写入数据
    return FLASH_WaitForLastOperation(FLASH_WAITETIME);
}

//从指定地址开始写入指定长度的数据
//特别注意:因为STM32F4的扇区实在太大,没办法本地保存扇区数据,所以本函数
//         写地址如果非0XFF,那么会先擦除整个扇区且不保存扇区数据.所以
//         写非0XFF的地址,将导致整个扇区数据丢失.建议写之前确保扇区里
//         没有重要数据,最好是整个扇区先擦除了,然后慢慢往后写.
//该函数对OTP区域也有效!可以用来写OTP区!
//OTP区域地址范围:0X1FFF7800~0X1FFF7A0F(注意：最后16字节，用于OTP数据块锁定，别乱写！！)
//WriteAddr:起始地址(此地址必须为4的倍数!!)
//pBuffer:数据指针
//NumToWrite:字(32位)数(就是要写入的32位数据的个数.)

void Write_FLASH(u32 WriteAddr, u32 *data, u32 Num)
{
    unsigned char sector[2];//用于判断写入的起始地址和结束地址是不是在同一个扇区
    unsigned char data_num = 0;//擦除的扇区数
    FLASH_EraseInitTypeDef FlashEraseInit;
	HAL_StatusTypeDef FlashStatus = HAL_OK;
	u32 SectorError = 0;
	u32 addrx = 0;
	u32 endaddr = 0;
    
	if(WriteAddr < STM32_FLASH_BASE || WriteAddr % 4)
        return;	//非法地址
    
    addrx = WriteAddr;              //写入的起始地址
	endaddr = WriteAddr + Num * 4;	//写入的结束地址
    
    if(endaddr > STM32_FLASH_END)
        return;	//非法地址
    sector[0] = GetFlashSector(addrx);
    sector[1] = GetFlashSector(endaddr);
    
    FlashEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;//擦除类型，扇区擦除
    FlashEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;//电压范围，VCC=2.7~3.6V之间!!
    
    HAL_FLASH_Unlock();     //解锁
    
    while(addrx < endaddr)//假如写入的区域没有其他数据存在，就不用清掉整个扇区
    {
        if(STMFLASH_ReadWord(addrx) != 0xFFFFFFFF)
        {
            sector[0] = GetFlashSector(addrx);
            FlashEraseInit.Sector = sector[0];      //要擦除的扇区
            FlashEraseInit.NbSectors = 1;  //一次只擦除一个扇区
            if(HAL_FLASHEx_Erase(&FlashEraseInit, &SectorError) != HAL_OK)
                break;//发生错误
            if(sector[0] == sector[1])//同一个扇区
                break;//说明后面的数据被清理完了，不用再管，直接退出
        }
        addrx += 4;
        FLASH_WaitForLastOperation(FLASH_WAITETIME);//等待上次操作完成
    }
    
    //等待擦除操作完成
    FlashStatus = FLASH_WaitForLastOperation(FLASH_WAITETIME);
    data_num = 0;
    if(FlashStatus == HAL_OK)
    {
        while(WriteAddr < endaddr)
        {
            //写入数据
            if(FLASH_WriteWord(WriteAddr, data[data_num]) != HAL_OK)
                break;//写入异常
            data_num ++;
            WriteAddr += 4;
        }
    }
    HAL_FLASH_Lock();       //上锁
}

void STMFLASH_Write(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite)
{
    FLASH_EraseInitTypeDef FlashEraseInit;
	HAL_StatusTypeDef FlashStatus = HAL_OK;
	u32 SectorError = 0;
	u32 addrx = 0;
	u32 endaddr = 0;
    
	if(WriteAddr < STM32_FLASH_BASE || WriteAddr % 4)
        return;	//非法地址
    
    addrx = WriteAddr;				        //写入的起始地址
	endaddr = WriteAddr + NumToWrite * 4;	//写入的结束地址
    
    if(endaddr > STM32_FLASH_END)
        return;	//非法地址

    HAL_FLASH_Unlock();             //解锁
	if(addrx < 0X1FFF0000)
	{
		while(addrx<endaddr)		//扫清一切障碍.(对非FFFFFFFF的地方,先擦除)
		{
			 if(STMFLASH_ReadWord(addrx) != 0XFFFFFFFF)//有非0XFFFFFFFF的地方,要擦除这个扇区
			{
				FlashEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;       //擦除类型，扇区擦除
				FlashEraseInit.Sector = GetFlashSector(addrx);   //要擦除的扇区
				FlashEraseInit.NbSectors = 1;                             //一次只擦除一个扇区
				FlashEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;      //电压范围，VCC=2.7~3.6V之间!!
				if(HAL_FLASHEx_Erase(&FlashEraseInit, &SectorError) != HAL_OK)
				{
					break;//发生错误了
				}
				}else addrx+=4;
				FLASH_WaitForLastOperation(FLASH_WAITETIME);                //等待上次操作完成
		}
	}
	FlashStatus=FLASH_WaitForLastOperation(FLASH_WAITETIME);            //等待上次操作完成
	if(FlashStatus==HAL_OK)
	{
		 while(WriteAddr<endaddr)//写数据
		 {
			if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,WriteAddr,*pBuffer)!=HAL_OK)//写入数据
			{
				break;	//写入异常
			}
			WriteAddr+=4;
			pBuffer++;
		}
	}
	HAL_FLASH_Lock();           //上锁
}

//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToRead:字(32位)数
void STMFLASH_Read(u32 ReadAddr,u32 *pBuffer,u32 NumToRead)
{
	u32 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadWord(ReadAddr);//读取4个字节.
		ReadAddr+=4;//偏移4个字节.
	}
}

//////////////////////////////////////////测试用///////////////////////////////////////////
//WriteAddr:起始地址
//WriteData:要写入的数据
void Test_Write(u32 WriteAddr,u32 WriteData)
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//写入一个字
}



