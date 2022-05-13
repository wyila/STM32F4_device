/****************************************************************************
 *
 * rs485 --The software used to participate in chassis rs485 bus
 *
 * Copyright (c) 2009-2019 OpenVox Communication Co.,Ltd.
 * All Rights Reserved. 
 * Author: jiajun.chen <jiajun.chen@openvox.cn>
 *
 * @file rs485.c
 * \brief Only one source file for application - participate in chassis rs485 bus.
 * \details Used serial port driver rs485, application can be slave or master, half duplex, 
 *          simulate token-ring, follow customize protocol communicate with other devices, 
 *          all above functions in this source file
 *
 * Revision:
 *   2020-09-20 16:21 init version, support slave/master device
 * 
 ******************************************************************************/
#include "app.h"
#include "bsp_i58250.h"
#include "rs485.h"
#include "gpio_sysfs.h"

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

/**
  * @brief  : config_tty(fd, 115200, 8, 'N', 1)
  * @param  : fd, baud, databits, parity, stopbits
  * @retval : ret: 0:ok, -1:fail
  */
int config_tty(int fd)
{
    struct termios tty_opt;
    int flag = 0;
    
    flag = fcntl(fd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);
    
    tcgetattr(fd, &tty_opt);

    tty_opt.c_cflag = CS8 | CLOCAL | CREAD;

    /* Set into raw, no echo mode */
    tty_opt.c_iflag = IGNPAR;
    tty_opt.c_lflag = 0;
    tty_opt.c_oflag = 0;

    cfsetospeed(&tty_opt, (speed_t)B115200);
    cfsetispeed(&tty_opt, (speed_t)B115200);

    tcsetattr(fd, TCSANOW, &tty_opt);
    usleep(50 * 1000);  //most USB serial port drivers don't support flushing properly, sleep may cure it
    tcflush(fd, TCIOFLUSH);
    ioctl(fd, TCFLSH, 2);
    return 0;
}

/**
  * @brief  : receive com port data
  * @param  : fd, recvbuf, recvnum, interval timeout
  * @retval : ret: <0:fail, other:readed num
  */
int treadn(int fd, unsigned char *buf, int nbytes, unsigned int timout)
{
    int nleft;  /* remain need */
    int nread;  /* -1 or have readed */
    int ret;
    struct pollfd tty_fds;
    
    if (fd < 0)
        return -1;
    
    tty_fds.fd = fd;
    tty_fds.events = POLLIN;
    
    nleft = nbytes;
    
    ret = poll(&tty_fds, 1, -1);  /* wait data arrived */
    if(!ret)
        return -2;
    dbg_print(2, "\nstart_ret = %d\n", ret);
    while(nleft > 0)
    {
        ret = poll(&tty_fds, 1, timout);
        if(!ret)
        {
            nread = -1;
        }
        else
        {
            if(tty_fds.revents & POLLIN)
                nread = read(fd, buf, nleft);
            else
                nread = -2;
        }
        dbg_print(2, "nread = %d\n", nread);
        if(nread < 0)
        {
             if(nleft == nbytes)
                return -3; /* error, not readed, return -1 */
             else
                break;      /* timeout, return amount readed so far */
        }else if(nread == 0)
            break;          /* EOF */
        nleft -= nread;
        buf += nread;
    }
    
    return (nbytes - nleft);      /* return >= 0 */
}

/**
  * @brief  : sent out com port data
  * @param  : fd, send buf, buf len
  * @retval : ret: <0:fail, other:writed num
  */
int writen(int fd, unsigned char *buf, int nbytes)
{
    int ret = 0;
    GPIOWrite(RS485_TXEN_GPIO, HIGH);
    usleep(1000);
    ret = write(fd, buf, nbytes);
    tcdrain(fd);
    usleep(1000);
    GPIOWrite(RS485_TXEN_GPIO, LOW);
    return ret;
}
