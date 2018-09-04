/********************************************************************
* 		copyright (C) 2015-2019 MYIR Tech Limited. All rights reserved
*			 @file: rs485_test.c
* 		  @Created: 2015-3-4
* 	  	   @Author: Kevin Su(kevin.su@myirtech.com)
* 	  @Description: test the RS485 function of sending and receiving 
*	  @Modify Date: 2015-3-4
*********************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <linux/serial.h>
#include <linux/types.h>
#include <asm/ioctls.h>
#include "../common.h"

#ifndef TIOCSRS485
	#define TIOCSRS485     0x542F
#endif

#ifndef TIOCGRS485
	#define TIOCGRS485     0x542E
#endif

#define SPEED	B115200

int open_port(const char *dev)
{
	int fd = -1;

	fd = open(dev, O_RDWR | O_NOCTTY);
	if ( fd < 0) {
		perror("open uart port");
	}
	return fd;
}

int config_port(const int fd)
{
	struct termios config = {0};
	struct serial_rs485 rs485_config = {0};
	int ret = -1;
	
	bzero(&rs485_config, sizeof(rs485_config));
	// Get RS485 mode
	ret = ioctl(fd, TIOCGRS485, &rs485_config);
	if (ret) {
		perror("ioctl TIOCGRS485");
		return ret;
	}
	
	// Set to RS485 mode
	printf("Set port to RS485 mode\r\n");
	rs485_config.delay_rts_before_send = 4;
	rs485_config.flags |= SER_RS485_ENABLED|SER_RS485_RTS_AFTER_SEND;
	ret = ioctl(fd, TIOCSRS485, &rs485_config);
	if (ret) {
		perror("ioctl TIOCSRS485");
		return ret;
	}
	
	ret = tcgetattr(fd, &config);
	if ( ret ) {
		perror("tcgetattr");
		return ret;
	}
	
	// Set baud rate
	cfsetispeed(&config, SPEED);
	cfsetospeed(&config, SPEED);
	
	// Set data bits to 8
	config.c_cflag &= ~CSIZE;
	config.c_cflag |= CS8;
	
	// Set 1 stop bit
	config.c_cflag &= ~CSTOPB;
	
	// Disable parity
	config.c_cflag &= ~PARENB;
	
	// Set Baud rate(for compatible)
	config.c_cflag &= ~CBAUD;
	config.c_cflag |= SPEED;

	// Set timeout
	config.c_cc[VMIN] = 1;
	config.c_cc[VTIME] = 0;
	
	// Disable RTSCTS
	config.c_cflag &= ~CRTSCTS;
	
	// Enable the receive and set local mode
	config.c_cflag |= CLOCAL | CREAD;
	
	// Original input mode
	config.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	
	// Original output mode
	config.c_oflag &= ~OPOST;

	tcflush(fd, TCIOFLUSH);	

	ret = tcsetattr(fd, TCSANOW, &config);
	if ( ret ) {
		perror("tcsetattr");
		return ret;
	}

	return 0;
}

int rs485_write(const char * dev)
{
	int ret = -1, byte_sent = 0;
	int fd = -1;
	int i=100;
	char * test_str = "0123456789";
	const int str_len = strlen(test_str);
	char rev_buf[100] = {0};
	unsigned int packets = 0;
	
	fd = open_port(dev);
	if (fd < 0) {
		return fd;
	}

	if (ret = config_port(fd)) {
		return ret;
	}

		printf("Program run in MASTER mode.\r\n\n");
		do{
		memset(rev_buf, 0, sizeof(rev_buf));
		byte_sent = 0;

		ret = write(fd, test_str, str_len);
		if (ret != str_len) {
			printf("[ERROR] Send data fail! ret: %d, expected: %d\r\n", ret, str_len);
			close(fd);
			return -1;
		}
		/* wait data te be sent out */
		ret = tcdrain(fd);
		
		/* wait for shift register to be empty */
/*		printf("[%8d] SEND: <%s>\r\n", packets++, test_str);
*/
		usleep(50);

		ret = 0;
	}while(i--);
	
	return ret;
}

