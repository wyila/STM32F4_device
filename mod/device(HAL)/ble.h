#ifndef __BLE_H
#define __BLE_H

//0: 未连接; 1: 连接
#define BLE_STATUS_CONNECT  PAin(1)

#define BLE_ACK_CFG     (u8 *)"a+ok"//进入配置成功
#define BLE_ACK         (u8 *)"\r\nOK\r\n"//正常情况下的回复
#define BLE_NAK         (u8 *)"\r\nREEOR\r\n"


#define BLE_CFG     (u8 *)"+++a" //进入配置模式
#define BLE_ENTM    (u8 *)"AT+ENTM" //穿透模式
#define BLE_TEST    (u8 *)"AT"    //测试
#define BLE_RESET   (u8 *)"AT+RESET" //复位
#define BLE_RELOAD  (u8 *)"AT+RELOAD"//出厂设置
#define BLE_LED     (u8 *)"AT+LEDEN"//检查LED
#define BLE_HELLO   (u8 *)"AT+HELLO" //开机显示文本
#define BLE_MAC     (u8 *)"AT+MAC" //MAC地址检查、
#define BLE_VER     (u8 *)"AT+CIVER"//版本检查
#define BLE_TPL     (u8 *)"AT+TPL"//发射功率
#define BLE_UART    (u8 *)"AT+UART"//串口参数查询
#define BLE_UARTTIM (u8 *)"AT+UARTTIM"//串口打包时间
#define BLE_LINK    (u8 *)"AT+LINK"//连接状态
#define BLE_DISCON  (u8 *)"AT+DISCONN"//断开连接
#define BLE_UUID    (u8 *)"AT+UUID"//串口服务UUID查询

void BLE_Init(void);
int BLE_SEND_CMD(unsigned char *CMD, unsigned char *VAL);
int BLE_wait(unsigned char *ack);
int scan_bound(void);
void ignore_ack(void);
#endif



