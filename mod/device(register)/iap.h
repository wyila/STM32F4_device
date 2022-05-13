#ifndef __IAP_H
#define __IAP_H

typedef void (*user_app)(void);

#define SOH     0x01    //xmodem 128 字节方式发送
#define STX     0x02    //xmodem 128 字节方式发送
#define EOT     0x04    //发送结束
#define ACK     0x06    //应答标志
#define NAK     0x15    //非应答，接收的数据错误
#define CAN     0x18    //取消发送
#define CRC16   0x43    //使用CRC16校验

void JUMP_TO_BOOTLOADER(void);

unsigned short XmodemCrc16(unsigned char *ptr, int len);
void iap_load_app(unsigned int appxaddr);
#endif

