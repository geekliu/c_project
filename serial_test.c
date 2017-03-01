#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

#define DEBUG(msg)	printf("%s\n",msg);

int serial_test(char *dev_path)
{
	int	fd = -1;
	struct	termios	options;
	char	*send = NULL;
	char	receive[10];
	int	send_len = 0;
	int 	read_len = 0;
	int 	n = 0;
	fd_set	readset;
	struct	timeval	timeout;

	/* open serial device */
	fd = open(dev_path,O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0){
		perror("open faild");
	}
	/* set serial parameter */
	tcgetattr(fd, &options);
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~CSIZE;
	options.c_cflag &= ~CRTSCTS;
	options.c_cflag |= CS8;
	options.c_cflag &= ~CSTOPB;	
	options.c_iflag |= IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	cfsetospeed(&options,B115200);	
	tcflush(fd,TCIFLUSH);
	tcsetattr(fd,TCSANOW,&options);
	
	/* send data to serial device */
	send = "send test";
	send_len = 0;
	send_len = write(fd,send,10);
	if (send_len != 10){
		DEBUG("send error!");
		return -1;
	}
	printf("%s send	= %s\n",dev_path,send);
	/* receive date from serial device */
	FD_ZERO(&readset);
	FD_SET(fd, &readset);
	
	timeout.tv_sec = 0;
	timeout.tv_usec = 20000;
	
	read_len = 0;
	while (select(fd+1,&readset,NULL,NULL,&timeout) > 0){
		n = read(fd, receive+read_len, 10 - read_len);
		if (n < 0){
			DEBUG("read error");
			return -1;
		}
		read_len += n; 	
	}	
	
	if (read_len < 10){
		DEBUG("receive error");
		return -1;
	}	
	
	/* compare send and receive */
	printf("%s receive	= %s\n",dev_path,receive);
	if (strcmp(send,receive) == 0){
		DEBUG("compare success!");
		close(fd);
		return 0;
	}
	else{
		DEBUG("compare error!");
		close(fd);
		return -1;
	}		
}

#if 1
int main()
{
	serial_test("/dev/ttyUSB0");
	return 0;
}
#endif
