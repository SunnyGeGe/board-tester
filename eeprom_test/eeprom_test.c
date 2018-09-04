/********************************************************************
 *               copyright (C) 2015-2019 MYIR Tech Limited. All rights reserved
 *                        @file: serial_test.c
 *                 @Created: 2015-3-4
 *                  @Author: Kevin Su(kevin.su@myirtech.com)
 *         @Description: i2c eeprom read/write test.
 *         @Modify Date: 2015-3-4
 *********************************************************************/
#include <stdio.h>
#include <linux/types.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#define I2C_RETRIES 0x0701
#define I2C_TIMEOUT 0x0702
#define I2C_RDWR 0x0707 

#define I2C_ADDR 0x51

const static char *FIRST_STR = "MYiR EEPROM FIRST TEST STR";
const static char *SECOND_STR = "MYiR EEPROM SECOND TEST STR";

struct i2c_msg
{
	unsigned short addr;
	unsigned short flags;
#define I2C_M_TEN 0x0010
#define I2C_M_RD 0x0001
	unsigned short len;
	unsigned char *buf;
};

struct i2c_rdwr_ioctl_data
{
	struct i2c_msg *msgs;
	int nmsgs;
};

/***********main***********/
int eeprom_test(const char *dev,const int i2c_addr)
{
	int fd,ret,i;
	struct i2c_rdwr_ioctl_data e2prom_data;
	char buffer[2][32] = {0};

	fd=open(dev,O_RDWR);
	if(fd < 0) {
		perror("open error");
		return -1;
	}
	
	e2prom_data.nmsgs=2;
	e2prom_data.msgs=(struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
	if(!e2prom_data.msgs) {
		perror("malloc error");
		return -1;
	}
	ioctl(fd,I2C_TIMEOUT,1);
	ioctl(fd,I2C_RETRIES,2);

	/***write data to e2prom**/
	//printf("Write data: %S\n", FIRST_STR);
	memset(buffer, 0, sizeof(buffer));
	e2prom_data.nmsgs=1;
	e2prom_data.msgs[0].len=strlen(FIRST_STR) + 2;
	e2prom_data.msgs[0].addr=i2c_addr;
	e2prom_data.msgs[0].flags=0; //write
	e2prom_data.msgs[0].buf=buffer[0];
	
	e2prom_data.msgs[0].buf[0]=0x00;// e2prom addr[15:8] 
	e2prom_data.msgs[0].buf[1]=0x00;//e2prom addr[7:0]
	strncpy(&buffer[0][2], FIRST_STR, strlen(FIRST_STR));

	ret=ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
	if(ret<0) {
		perror("ioctl error1");
		goto __ERR;
	}

	usleep(100*1000);
	/******read data from e2prom*******/
	memset(buffer, 0, sizeof(buffer));
	e2prom_data.nmsgs=2;
	e2prom_data.msgs[0].len=2;
	e2prom_data.msgs[0].addr=i2c_addr;
	e2prom_data.msgs[0].flags=0;//write
	e2prom_data.msgs[0].buf=buffer[0];
	e2prom_data.msgs[0].buf[0]=0x00;// e2prom addr[15:8] 
	e2prom_data.msgs[0].buf[1]=0x00;//e2prom addr[7:0] 
	e2prom_data.msgs[1].len=strlen(FIRST_STR);
	e2prom_data.msgs[1].addr=i2c_addr;
	e2prom_data.msgs[1].flags=I2C_M_RD;
	e2prom_data.msgs[1].buf=buffer[1];


	ret=ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
	if(ret<0) {
		("ioctl error2");
		goto __ERR;
	}

	if (strncmp(e2prom_data.msgs[1].buf, FIRST_STR, strlen(FIRST_STR)) != 0)
		goto __ERR;
	
	printf("first  Read back: %s\n", e2prom_data.msgs[1].buf);

	/***write data to e2prom**/
	//printf("Write data: %S\n", SECOND_STR);
	memset(buffer, 0, sizeof(buffer));
	e2prom_data.nmsgs=1;
	e2prom_data.msgs[0].len=strlen(SECOND_STR) + 2;
	e2prom_data.msgs[0].addr=i2c_addr;
	e2prom_data.msgs[0].flags=0; //write
	e2prom_data.msgs[0].buf=buffer[0];
	
	e2prom_data.msgs[0].buf[0]=0x00;// e2prom addr[15:8] 
	e2prom_data.msgs[0].buf[1]=0x00;//e2prom addr[7:0]
	strncpy(&buffer[0][2], SECOND_STR, strlen(SECOND_STR));

	/*
	e2prom_data.msgs[0].buf[2]=0x55;//the data to write byte0
	e2prom_data.msgs[0].buf[3]=0x66;//the data to write byte1
	e2prom_data.msgs[0].buf[4]=0x77;//the data to write byte2
	*/
	ret=ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
	if(ret<0) {
		perror("ioctl error1");
		goto __ERR;
	}

	usleep(100*1000);
	/******read data from e2prom*******/
	memset(buffer, 0, sizeof(buffer));
	e2prom_data.nmsgs=2;
	e2prom_data.msgs[0].len=2;
	e2prom_data.msgs[0].addr=i2c_addr;
	e2prom_data.msgs[0].flags=0;//write
	e2prom_data.msgs[0].buf=buffer[0];
	e2prom_data.msgs[0].buf[0]=0x00;// e2prom addr[15:8] 
	e2prom_data.msgs[0].buf[1]=0x00;//e2prom addr[7:0] 
	e2prom_data.msgs[1].len=strlen(SECOND_STR);
	e2prom_data.msgs[1].addr=i2c_addr;
	e2prom_data.msgs[1].flags=I2C_M_RD;
	e2prom_data.msgs[1].buf=buffer[1];

	ret=ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
	if(ret<0) {
		("ioctl error2");
		goto __ERR;
	}

	if (strncmp(e2prom_data.msgs[1].buf, SECOND_STR, strlen(SECOND_STR)) != 0)
		goto __ERR;
	
	printf("second Read back: %s\n", e2prom_data.msgs[1].buf);

	free(e2prom_data.msgs);

	close(fd);

	return 0;

 __ERR:
	free(e2prom_data.msgs);
	return -1;
}

