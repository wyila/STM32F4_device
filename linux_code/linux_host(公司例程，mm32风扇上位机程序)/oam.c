/****************************************************************************
 *
 * app --The software used to participate in chassis rs485 bus
 *
 * Copyright (c) 2009-2020 OpenVox Communication Co.,Ltd.
 * All Rights Reserved. 
 * Author: jiajun.chen <jiajun.chen@openvox.cn>
 *
 * @file app.c
 * \brief Only one source file for application - participate in chassis rs485 bus.
 * \details Used serial port driver rs485, application can be slave or master, half duplex, 
 *          simulate token-ring, follow customize protocol communicate with other devices, 
 *          all above functions in this source file
 *
 * Revision:
 *   2020-09-20 16:21 init version, support slave/master device
 * 
 ******************************************************************************/
#include "oam.h"

#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <inttypes.h>
#include <pthread.h>

#define APP_NAME        "OAM bus rs485"
#define APP_VER_MAIN    (1)
#define APP_VER_SUB     (0)

#define RS485_TXEN_GPIO      (441)   /*rs485 send enable gpio */

#define dbg_printf(debug, format, ...)   do{if(debug) printf(format, ##__VA_ARGS__);}while(0)

#define CHASSIS_INFO_FILE         "/tmp/info"
FILE *fd_info = NULL;

board_info_t board[8] = {{{0}}};

unsigned char own_dev = DEV_BPL;

int fd_tty = 0;
int debug = 1;

char test_start_time[128] = {0};
char test_end_time[128] = {0};

/* crc8 table: Maxim 0x131 */
static const unsigned char crc8_table[] =
{
    0x00,0x5E,0xBC,0xE2,0x61,0x3F,0xDD,0x83,0xC2,0x9C,0x7E,0x20,0xA3,0xFD,0x1F,0x41,
    0x9D,0xC3,0x21,0x7F,0xFC,0xA2,0x40,0x1E,0x5F,0x01,0xE3,0xBD,0x3E,0x60,0x82,0xDC,
    0x23,0x7D,0x9F,0xC1,0x42,0x1C,0xFE,0xA0,0xE1,0xBF,0x5D,0x03,0x80,0xDE,0x3C,0x62,
    0xBE,0xE0,0x02,0x5C,0xDF,0x81,0x63,0x3D,0x7C,0x22,0xC0,0x9E,0x1D,0x43,0xA1,0xFF,
    0x46,0x18,0xFA,0xA4,0x27,0x79,0x9B,0xC5,0x84,0xDA,0x38,0x66,0xE5,0xBB,0x59,0x07,
    0xDB,0x85,0x67,0x39,0xBA,0xE4,0x06,0x58,0x19,0x47,0xA5,0xFB,0x78,0x26,0xC4,0x9A,
    0x65,0x3B,0xD9,0x87,0x04,0x5A,0xB8,0xE6,0xA7,0xF9,0x1B,0x45,0xC6,0x98,0x7A,0x24,
    0xF8,0xA6,0x44,0x1A,0x99,0xC7,0x25,0x7B,0x3A,0x64,0x86,0xD8,0x5B,0x05,0xE7,0xB9,
    0x8C,0xD2,0x30,0x6E,0xED,0xB3,0x51,0x0F,0x4E,0x10,0xF2,0xAC,0x2F,0x71,0x93,0xCD,
    0x11,0x4F,0xAD,0xF3,0x70,0x2E,0xCC,0x92,0xD3,0x8D,0x6F,0x31,0xB2,0xEC,0x0E,0x50,
    0xAF,0xF1,0x13,0x4D,0xCE,0x90,0x72,0x2C,0x6D,0x33,0xD1,0x8F,0x0C,0x52,0xB0,0xEE,
    0x32,0x6C,0x8E,0xD0,0x53,0x0D,0xEF,0xB1,0xF0,0xAE,0x4C,0x12,0x91,0xCF,0x2D,0x73,
    0xCA,0x94,0x76,0x28,0xAB,0xF5,0x17,0x49,0x08,0x56,0xB4,0xEA,0x69,0x37,0xD5,0x8B,
    0x57,0x09,0xEB,0xB5,0x36,0x68,0x8A,0xD4,0x95,0xCB,0x29,0x77,0xF4,0xAA,0x48,0x16,
    0xE9,0xB7,0x55,0x0B,0x88,0xD6,0x34,0x6A,0x2B,0x75,0x97,0xC9,0x4A,0x14,0xF6,0xA8,
    0x74,0x2A,0xC8,0x96,0x15,0x4B,0xA9,0xF7,0xB6,0xE8,0x0A,0x54,0xD7,0x89,0x6B,0x35 
};

/**
  * @brief  calculate crc8 by table
  * @param  src buf, buf len
  * @retval dst crc8
  */
unsigned char calc_crc8(unsigned char *buf, unsigned int len)
{
    unsigned char crc = 0;

    while (len--)
        crc = crc8_table[crc ^ *buf++];

    return crc;
}


/**
  * @brief 打印hex数据调试信息
  * @param debug开关， hex data， hex data len
  * @retval void
  */
void print_hex(int debug, uint8_t *buff, int cnt)
{
    int i, col;
    if(!buff)
        return;
    while(0 < cnt){
        col = (16 < cnt) ? (16) : (cnt);
        for(i = 0; i < col; i++)
            dbg_printf(debug, "%02x ", buff[i]);
        for(i = col; i < 16; i++)
            dbg_printf(debug, "   ");
        dbg_printf(debug, "   ");
        for(i = 0; i < col; i++){
            if(buff[i] < 0)
                dbg_printf(debug,"?");
            else if((buff[i] < 32) || (buff[i] >= 127))
                dbg_printf(debug,".");
            else if(buff[i] == 0xff)
                dbg_printf(debug," ");
            else
                dbg_printf(debug, "%c", buff[i]);
        }
        dbg_printf(debug, "\n");
        buff += col;
        cnt -= col;
    }
}

/**
  * @brief  : print app version
  * @param  : void
  * @retval : void
  */
void app_show_version(void)
{
    printf("Application:\n");
    printf("  name : %s\n", APP_NAME);
    printf("  version : %01d.%01d\n", APP_VER_MAIN, APP_VER_SUB);
    printf("  build : %s\n", __DATE__);
}

/**
  * @brief  : signal exit handler
  * @param  : void
  * @retval : void
  */
void signal_exit_handler(int sig)
{
    close_all_file();
    
    printf("\r\napp exit\r\n");
    exit(0);
}

int set_gpio_value(int gpio, unsigned char value)
{
	char gpio_path[128] = {0};
    FILE *fd = NULL;
    
	sprintf(gpio_path, "/sys/class/gpio/gpio%d", gpio);
	if(0 != access(gpio_path, F_OK)){
        /* export */
        if((fd = fopen("/sys/class/gpio/export", "w")) == NULL)
            return -1;
        fprintf(fd, "%d", gpio);
        fclose(fd);
        usleep(1000);
        /* set direction */
		memset(gpio_path, 0, sizeof(gpio_path));
        sprintf(gpio_path, "/sys/class/gpio/gpio%d/direction", gpio);
        fd = NULL;
        if((fd = fopen(gpio_path, "w")) == NULL)
            return -1;
        fprintf(fd, "out");
        fclose(fd);
 	}
    /* set output value */
    memset(gpio_path, 0, sizeof(gpio_path));
    sprintf(gpio_path, "/sys/class/gpio/gpio%d/value", gpio);
    fd = NULL;
    if((fd = fopen(gpio_path, "w")) == NULL)
        return -1;
    fprintf(fd, "%d", value);
    fclose(fd);
    
	return 0;
}

int gpio_release(int gpio)
{
	char gpio_path[128] = {0};
    FILE *fd = NULL;
    
	sprintf(gpio_path, "/sys/class/gpio/gpio%d", gpio);
	if(0 == access(gpio_path, F_OK)){
        /* set output value */
        memset(gpio_path, 0, sizeof(gpio_path));
        sprintf(gpio_path, "/sys/class/gpio/gpio%d/value", gpio);
        if((fd = fopen(gpio_path, "w")) == NULL)
            return -1;
        fprintf(fd, "%d", 0);
        fclose(fd);
        /* export */
        fd = NULL;
        if((fd = fopen("/sys/class/gpio/unexport", "w")) == NULL)
            return -1;
        fprintf(fd, "%d", gpio);
        fclose(fd);
 	}
	return 0;
}

/**
  * @brief  : config_tty(fd, 115200, 8, 'N', 1)
  * @param  : fd, baud, databits, parity, stopbits
  * @retval : ret: 0:ok, -1:fail
  */
int config_tty(int fd)
{
    struct termios tty;
    int flag = 0;
    
    flag = fcntl(fd, F_GETFL);
    flag &= ~O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);
    
    tcgetattr(fd, &tty);

    tty.c_cflag = CS8 | CLOCAL | CREAD;

    /* Set into raw, no echo mode */
    tty.c_iflag = IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VTIME] = 2;
    tty.c_cc[VMIN] = 0;
    
    cfsetospeed(&tty, (speed_t)B115200);
    cfsetispeed(&tty, (speed_t)B115200);

    tcsetattr(fd, TCSANOW, &tty);
    
    usleep(50 * 1000);  //most USB serial port drivers don't support flushing properly, sleep may cure it
    tcflush(fd, TCIOFLUSH);
    ioctl(fd, TCFLSH, 2);
    return 0;
}

/**
  * @brief  : reset enviroment
  * @param  : void
  * @retval : void
  */
void close_all_file(void)
{
    if(fd_tty)
        close(fd_tty);
    gpio_release(RS485_TXEN_GPIO);
    
    if(fd_info)
        fclose(fd_info);
    if(0 == access(CHASSIS_INFO_FILE, F_OK))
        remove(CHASSIS_INFO_FILE);
}

/**
  * @brief  : send a read reg cmd to rs485
    host: src / dst / func / ret / reg / num / crc8
    mcu : src / dst / func / ret / reg / num / dat[num] / crc8
  * @param  : dev addr, reg, read len, pdat
  * @retval : ret, 0_ok, other_fail
  */
int rs485_read_reg(unsigned char dev, unsigned char reg, unsigned char num, unsigned char *dat)
{
    unsigned char tx[320] = {0};
    unsigned char rx[320] = {0};
    int len = 0;
    tx[P_SRC] = own_dev;
    tx[P_DST] = dev;
    tx[P_FUNC] = FREAD;
    tx[P_RET] = 0;
    tx[P_START] = reg;
    tx[P_CNT] = num;
    tx[P_CNT + 1] = calc_crc8(tx, P_CNT + 1);
    //read reg
//    tcflush(fd_tty, TCOFLUSH);
    set_gpio_value(RS485_TXEN_GPIO, 1);
    usleep(1000);
    len = write(fd_tty, tx, P_CNT + 2);
    tcdrain(fd_tty);
    dbg_printf(debug, "preset:%d\n", len);
    print_hex(debug, tx, len);
//    usleep(20 * 1000);
    set_gpio_value(RS485_TXEN_GPIO, 0);
    if(len != (P_CNT + 2))
        return -EIO;
    //get result
//    tcflush(fd_tty, TCIFLUSH);
    len = read(fd_tty, rx, P_CNT + 2 + num + 7);
    if(len < 0)
        return len;
    dbg_printf(debug, "getdat:%d\n", len);
    print_hex(debug, rx, len);
    if(len != num + 7)
        return -EIO;
    //check result
    if(rx[P_DAT + num] != calc_crc8(rx, P_DAT + num))
        return -EPROTO;
    if((rx[P_SRC] != dev) || (rx[P_DST] != own_dev))
        return -EFAULT;
    if((rx[P_FUNC] != FREAD_RE) || memcmp(&rx[P_RET], &tx[P_RET], 3))
        return -ENOSPC;
    memcpy(dat, &rx[P_DAT], num);
    return 0;
}

/**
  * @brief  : send a write reg cmd to rs485
    hsot: src / dst / func / ret / reg / num / dat[num] / crc8
    mcu : src / dst / func / ret / reg / num / crc8
  * @param  : dev addr, reg, write len, point of write data
  * @retval : result, 0_ok, other_fail
  */
char rs485_write_reg(unsigned char dev, unsigned char reg, unsigned char num, unsigned char *dat)
{
    unsigned char tx[320] = {0};
    unsigned char rx[320] = {0};
    unsigned char *prx = NULL;
    int len = 0;
    tx[P_SRC] = own_dev;
    tx[P_DST] = dev;
    tx[P_FUNC] = FWRITE;
    tx[P_RET] = 0;
    tx[P_START] = reg;
    tx[P_CNT] = num;
    memcpy(&tx[P_DAT], dat, num);
    tx[P_DAT + num] = calc_crc8(tx, P_DAT + num);
    //write reg
    set_gpio_value(RS485_TXEN_GPIO, 1);
    len = write(fd_tty, tx, P_DAT + num + 1);
    tcdrain(fd_tty);
    dbg_printf(debug, "write:\n");
    print_hex(debug, tx, len);
    set_gpio_value(RS485_TXEN_GPIO, 0);
    if(len != (P_DAT + num + 1))
        return -EIO;
    //get result
    len = read(fd_tty, rx, P_DAT + num + 1 + 7);
    dbg_printf(debug, "reply:\n");
    print_hex(debug, rx, len);
    if(len != P_DAT + num + 1 + 7)
        return -EIO;
    if(memcmp(rx, tx, P_DAT + num + 1))
        return -EIO;
    //check result
    prx = &rx[P_DAT + num + 1];
    if(prx[6] != calc_crc8(prx, 6))
        return -EPROTO;
    if((prx[P_SRC] != dev) || (prx[P_DST] != own_dev))
        return -EFAULT;
    if(memcmp(&prx[P_FUNC], tx, 4))
        return -ENOSPC;
    return 0;
}

/**
  * @brief  : main body
  * @param  : argc, argv[][]
  * @retval : ret: 0:ok, -1:fail
  */
int main(int argc, char **argv)
{
    uint8_t data[320] = {"0123456789012345678901234567890123456789"};
    int res = 0;
    
    signal(SIGTERM, signal_exit_handler);
    signal(SIGINT, signal_exit_handler);
    
    if(argc != 2){
        printf("Usage: \n");
        printf("%s </dev/ttyxxx>\n", argv[0]);
        return -1;
    }
    
    if((fd_tty = open(argv[1], O_RDWR | O_NOCTTY)) < 0) {
        printf("error opening '%s'\n", argv[1]);
        exit(-ENXIO);
    }
    config_tty(fd_tty);
    
    app_show_version();
    
    while(1)
    {
        sleep(1);
        set_gpio_value(RS485_TXEN_GPIO, 1);
        write(fd_tty, data, 31);
        tcdrain(fd_tty);
        set_gpio_value(RS485_TXEN_GPIO, 0);
//        res = rs485_read_reg(0x02, 0, 0x18, data);
        printf("\nres = %d\n", res);
//        print_hex(debug, data, 0x18);
    }
    
    close_all_file();
    return 0;
}
