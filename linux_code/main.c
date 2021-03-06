#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysinfo.h>

int fd = -1;

//USB 配置函数
void config_tty(int fd)
{
    struct termios tty;
    int flag = 0;
    
    flag = fcntl(fd, F_GETFL);
    flag &= ~O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);
    
    tcgetattr(fd, &tty);

    cfsetospeed(&tty, (speed_t)B115200);
    cfsetispeed(&tty, (speed_t)B115200);

    tty.c_cflag = CS8 | CLOCAL | CREAD;

    /* Set into raw, no echo mode */
    tty.c_iflag = IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VTIME] = 5;
    tty.c_cc[VMIN] = 0;

    tcsetattr(fd, TCSANOW, &tty);
    usleep(50 * 1000);  //most USB serial port drivers don't support flushing properly, sleep may cure it
    tcflush(fd, TCIOFLUSH);
    ioctl(fd, TCFLSH, 2);
}
  
int main(int argc, char **argv)
{
  char buf[16] = {0};
  char *USB_BUS = NULL;
  FILE *fp = NULL;
  
  
  if(argc == 1)
  {
  	fp = popen("ls /dev/ttyUSB*", "r");//查找串口设备
    if(fgets(buf, sizeof(buf), fp))
    {
      strtok(buf, "\n");
      USB_BUS = (char *)malloc(strlen(buf) + 5 + 1);
      sprintf(USB_BUS, "/dev/%s", buf);
      if((fd = open(USB_BUS, O_RDWR)) == -1)
      {
      	printf("open \"%s\" bus error!!!\n", USB_BUS);
      	return 0;
      }
    }
    else
    {
    	printf("USB midline cannot be found!!\n");
      return 0;
    }
  }
  else
  {
  	fp = open(argv[1], O_RDWR);
  	if(fp == -1)
		{
			printf("USB bus \"%s\" open error!!!\n", argv[1]);
			return 0;
		}
  }
  
  printf("open %s success!!!\n");
  
  printf("STM32F429 Upper computer program\n");
  
  return 0;
}
  