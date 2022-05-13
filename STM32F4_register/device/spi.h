#ifndef __SPI_H
#define __SPI_H

void SPI5_Init(void);
void SPI5_SetSpeed(unsigned char SpeedSet);
unsigned char SPI5_ReadWriteByte(unsigned char TxData);

#endif


