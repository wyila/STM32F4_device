#ifndef	 _RS485_H_
#define	 _RS485_H_

#define RX_OT_CHAR     (4)
#define BUS_IDLE_WAIT  (8)

int config_tty(int fd);
int treadn(int fd, unsigned char *buf, int nbytes, unsigned int timout);
int writen(int fd, unsigned char *buf, int nbytes);

#endif
