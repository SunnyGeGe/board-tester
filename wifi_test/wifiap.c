#include <termios.h>
#include <linux/ioctl.h>
#include <linux/serial.h>
#include <asm-generic/ioctls.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>

extern int err_on_wifi;

#ifndef DEBUG
#include "../common.h"
#else
int re = 0;
typedef enum {DISABLE = 0, ENABLE} RS485_ENABLE_t;

static int open_tty(const char *tty_dev)
{
	int fd;
	fd = open(tty_dev, O_RDWR|O_NONBLOCK|O_NOCTTY);
	if(fd < 0)
		perror("open tty");

	return fd;
}

static int set_tty(int fd,
				   int bitrate,
				   int datasize,
				   char par,
				   int stop)
{
	struct termios newtio;

	
	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch (datasize) {
	case 8:
		newtio.c_cflag |= CS8;
		break;
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 6:
		newtio.c_cflag |= CS6;
		break;
	case 5:
		newtio.c_cflag |= CS5;
		break;
	default:
		newtio.c_cflag |= CS8;
		break;
	}
	
	
	switch (par) {
	case 'o':
	case 'O':
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'e':
	case 'E':
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'n':
	case 'N':
		newtio.c_cflag &= ~PARENB;
		break;
	default:
		newtio.c_cflag &= ~PARENB;
		break;
	}
	

	switch (stop) {
	case 1:
		newtio.c_cflag &= ~CSTOPB;
		break;
	case 2:
		newtio.c_cflag |= CSTOPB;
		break;
	default:
		newtio.c_cflag &= ~CSTOPB;
		break;
	}
	
	
	switch (bitrate) {
	case 0:
		cfsetospeed(&newtio, B0);
		cfsetispeed(&newtio, B0);
		break;
	case 50:
		cfsetospeed(&newtio, B50);
		cfsetispeed(&newtio, B50);
		break;
	case 75:
		cfsetospeed(&newtio, B75);
		cfsetispeed(&newtio, B75);
		break;
	case 110:
		cfsetospeed(&newtio, B110);
		cfsetispeed(&newtio, B110);
		break;
	case 134:
		cfsetospeed(&newtio, B134);
		cfsetispeed(&newtio, B134);
		break;
	case 150:
		cfsetospeed(&newtio, B150);
		cfsetispeed(&newtio, B150);
		break;
	case 200:
		cfsetospeed(&newtio, B200);
		cfsetispeed(&newtio, B200);
		break;
	case 300:
		cfsetospeed(&newtio, B300);
		cfsetispeed(&newtio, B300);
		break;
	case 600:
		cfsetospeed(&newtio, B600);
		cfsetispeed(&newtio, B600);
		break;
	case 1200:
		cfsetospeed(&newtio, B1200);
		cfsetispeed(&newtio, B1200);
		break;
	case 1800:
		cfsetospeed(&newtio, B1800);
		cfsetispeed(&newtio, B1800);
		break;
	case 2400:
		cfsetospeed(&newtio, B2400);
		cfsetispeed(&newtio, B2400);
		break;
	case 4800:
		cfsetospeed(&newtio, B4800);
		cfsetispeed(&newtio, B4800);
		break;
	case 9600:
		cfsetospeed(&newtio, B9600);
		cfsetispeed(&newtio, B9600);
		break;
	case 19200:
		cfsetospeed(&newtio, B19200);
		cfsetispeed(&newtio, B19200);
		break;
	case 38400:
		cfsetospeed(&newtio, B38400);
		cfsetispeed(&newtio, B38400);
		break;
	case 57600:
		cfsetospeed(&newtio, B57600);
		cfsetispeed(&newtio, B57600);
		break;
	case 115200:
		cfsetospeed(&newtio, B115200);
		cfsetispeed(&newtio, B115200);
		break;
	case 230400:
		cfsetospeed(&newtio, B230400);
		cfsetispeed(&newtio, B230400);
		break;
	default:
		cfsetospeed(&newtio, B115200);
		cfsetispeed(&newtio, B115200);
		break;
	}
	

	newtio.c_cc[VTIME] = 100;
	newtio.c_cc[VMIN] = 1;


	tcflush(fd, TCIFLUSH);

	if((tcsetattr(fd, TCSANOW, &newtio))!=0) {
		perror("set_tty/tcsetattr");
		return -1;
	}

	return 0;
}
#endif
int at_write(int fd,const char*str,int len)
{
	int ret,cnt=0;
	while(cnt<len)
	{
		ret = write(fd,str+cnt,1);
		if(ret<0)
			perror("faile");
	cnt+=1;
	}
	return cnt;
}

void wifi_restore(const char* tty_dev, int bitrate, int datasize, char par, int stop)
{
    int fd=0;
	int ret=0,i;
	char buf[255],buf1[10];

    char switch_mode[3] = {'a','a','a'};
	char *p="a";
	char *p2="+";

    fd = open_tty(tty_dev);
    if(fd<1)
        perror("open tty");
    if(set_tty(fd,bitrate,datasize,par,stop)<0)
        perror("set tty");
	
    write(fd,p2,1);usleep(200000);
	write(fd,p2,1);usleep(200000);
	write(fd,p2,1);sleep(1);
	write(fd,p,1);
	
    sleep(2);
	read(fd,buf,3);
	printf("%s\n",buf);
    printf("Restore WiFi module to n-ver mode\n");

	ret = at_write(fd, "AT+FVER=n\r", 10);
    memset(buf, 0, sizeof(buf));
	ret = read(fd,buf,sizeof(buf));
    sleep(4);
	
	ret=at_write(fd,"AT+RELD\r",8);
    memset(buf, 0, sizeof(buf));
	ret = read(fd,buf,sizeof(buf));
    sleep(18);
	printf("Restore response[%d]: %s\n",ret, buf);

	sleep(20);
        
    close(fd);
}


void wifi_zver_enable(const char* tty_dev, int bitrate, int datasize, char par, int stop)
{
    int fd=0;
	int ret=0,i;
	char buf[50],buf1[10];
	char cmd_zver[]="AT+FVER\r";
    char cmd_restore[]="AT+RELD\r";
    char cmd_wap[]="AT+WAP=11BGN,123456,auto\r";
	char cmd_wsssid[]="AT+WSSSID=MYIR_ELECTRONICS\r";
	char cmd_wskey[]="AT+WSKEY=WPA2PSK,AES,myir2016";
	
    char switch_mode[3] = {'a','a','a'};
	char *p="a";
	char *p2="+";
    char cmd_fvew[] = "AT+FVEW=on\r";
    char cmd_fephy[] = "AT+FEPHY=on\r";
    char cmd_lann[] = "AT+LANN=192.168.30.10,255.255.255.0\r";
    char cmd_reboot[] = "AT+Z\r";

    fd = open_tty(tty_dev);
    if(fd<1)
        perror("open tty");
    if(set_tty(fd,bitrate,datasize,par,stop)<0)
        perror("set tty");
	
    write(fd,p2,1);usleep(200000);
	write(fd,p2,1);usleep(200000);
	write(fd,p2,1);sleep(1);
	write(fd,p,1);
	
    sleep(2);
	read(fd,buf,3);
	printf("%s\n",buf);
    printf("Restore WiFi module to zver mode\n");

	ret = at_write(fd, "AT+FVER=z\r", 10);
    memset(buf, 0, sizeof(buf));
	ret = read(fd,buf,sizeof(buf));
    sleep(4);
	printf("AT+FVER=z response[%d]: %s\n",ret, buf);

	ret = at_write(fd, "AT+FEPHY=on\r", 12);
    memset(buf, 0, sizeof(buf));
	ret = read(fd,buf,sizeof(buf));
    sleep(4);
	printf("AT+FEPHY=on response[%d]: %s\n",ret, buf);
	
	ret=at_write(fd,"AT+RELD\r",8);
    memset(buf, 0, sizeof(buf));
	ret = read(fd,buf,sizeof(buf));
    sleep(18);
	printf("Restore response[%d]: %s\n",ret, buf);

    printf("ReEnter CLi-mode\n");
    write(fd,p2,1);usleep(200000);
	write(fd,p2,1);usleep(200000);
	write(fd,p2,1);sleep(1);
	write(fd,p,1);
	
	sleep(2);
    memset(buf, 0, sizeof(buf));
	read(fd,buf,sizeof(buf));
	printf(" %s\n",buf);

	ret=at_write(fd,"AT+WMODE=STA\r",13);	
	if(ret!=13)
        perror("AT+WMODE");

    memset(buf, 0, sizeof(buf));
	read(fd,buf,sizeof(buf));
	printf("%s\n",buf1);
	sleep(3);

	ret=at_write(fd,"AT+WSSSID=MYIR_ELECTRONICS\r",27);	
		if(ret != 20)
            perror("AT+WSSSID");
		sleep(4);


	ret=at_write(fd,"AT+WSKEY=WPA2PSK,AES,myir2016\r",30);	
		if(ret != 30)
            perror("AT+WSSKEY");
		sleep(4);
		
		ret=at_write(fd,"AT+LANN=192.168.30.10,255.255.255.0\r",36);	
			if(ret != 36)
				perror("AT+LANN");
			sleep(4);
		
	ret=at_write(fd,"AT+Z\r",5);	
		if(ret!=5)	perror("AT+z");
        printf("Reboot WiFi module, wait 20s\n");
		sleep(20);
        
        close(fd);

}

void wifi_ap_enable(const char* tty_dev,int bitrate,int datasize,char par,int stop)
{
    int fd=0;
	int ret=0,i;
	char buf[50],buf1[10];
    char cmd_restore[]="AT+RELD\r";
    char cmd_wap[]="AT+WAP=11BGN,123456,auto\r";
    char switch_mode[3] = {'a','a','a'};
	char *p="a";
	char *p2="+";
    char cmd_fvew[] = "AT+FVEW=on\r";
    char cmd_fephy[] = "AT+FEPHY=on\r";
    char cmd_lann[] = "AT+LANN=192.168.3.1,255.255.255.0\r";
    char cmd_reboot[] = "AT+Z\r";

    fd = open_tty(tty_dev);
    if(fd<1)
        perror("open tty");
    if(set_tty(fd,bitrate,datasize,par,stop)<0)
        perror("set tty");
	
    write(fd,p2,1);usleep(200000);
	write(fd,p2,1);usleep(200000);
	write(fd,p2,1);sleep(1);
	write(fd,p,1);
	
    sleep(2);
	read(fd,buf,3);
	printf("%s\n",buf);
    printf("Restore WiFi module\n");
	ret=at_write(fd,"AT+RELD\r",8);
    memset(buf, 0, sizeof(buf));
	ret = read(fd,buf,sizeof(buf));
    sleep(18);
	printf("Restore response[%d]: %s\n",ret, buf);

    printf("ReEnter CLi-mode\n");
    write(fd,p2,1);usleep(200000);
	write(fd,p2,1);usleep(200000);
	write(fd,p2,1);sleep(1);
	write(fd,p,1);
	
	sleep(2);
    memset(buf, 0, sizeof(buf));
	read(fd,buf,sizeof(buf));
	printf(" %s\n",buf);

	ret=at_write(fd,"AT+WAP=11BGN,123456,auto\r",25);	
	if(ret!=25)
        perror("AT+WAP");

    memset(buf, 0, sizeof(buf));
	read(fd,buf,sizeof(buf));
	printf("%s\n",buf1);
	sleep(3);
	ret=at_write(fd,"AT+FVEW=disable\r",16);	
		if(ret!=16)	perror("AT+FVEW");
		sleep(4);
	ret=at_write(fd,"AT+FEPTP=on\r",12);
		if(ret!=12)	perror("AT+FEPTP");
		sleep(4);
	ret=at_write(fd,"AT+FEPHY=on\r",12);	
		if(ret!=12)	perror("AT+FEPHY");
		sleep(4);
	ret=at_write(fd,"AT+LANN=192.168.3.1,255.255.255.0\r",34);	
		if(ret != 34)
            perror("AT+LANN");
		sleep(4);
	ret=at_write(fd,"AT+Z\r",5);	
		if(ret!=5)	perror("AT+z");
        printf("Reboot WiFi module, wait 20s\n");
		sleep(20);
        
        close(fd);
}

void wifi_apsta_enable(const char* tty_dev,int bitrate,int datasize,char par,int stop)
{
	int fd=0;
	int ret=0,i;
	char buf[128],buf1[10];

	char *cmd_wsssid ="MYIR_ELECTRONICS";
	char *ver="+ok=6.01T.25-C6078-V1.3.2_20181212";
		
		char switch_mode[3] = {'a','a','a'};
		char *p="a";
		char *p2="+";

		fd = open_tty(tty_dev);
		if(fd<1)
			perror("open tty");
		if(set_tty(fd,bitrate,datasize,par,stop)<0)
			perror("set tty");
		
//		ret=at_write(fd,"AT+Z\r",5);	
//		if(ret!=5)	perror("AT+Z");
//		printf("Reboot WiFi module, wait 20s\n");
//		sleep(20);
		
		write(fd,p2,1);usleep(200000);
		write(fd,p2,1);usleep(200000);
		write(fd,p2,1);sleep(1);
		write(fd,p,1);
		
		sleep(2);
		read(fd,buf,1);
		fprintf(stderr, "%s\n",buf);
		fprintf(stderr, "Restore WiFi module to APSTA mode\n");
	
		ret = at_write(fd, "AT+VER\r", 7);
		memset(buf, 0, sizeof(buf));
		sleep(1);
		ret = read(fd,buf,sizeof(buf));
		printf("\r\n");
		printf("AT+VER=%s,%d\r\n",buf,strlen(buf));
		if(strstr(buf, ver) == NULL){
				err_on_wifi = 21;
		}
		else
		{
				err_on_wifi = 20;
		}
#if 0	
		ret = at_write(fd, "AT+FEPHY=on\r", 12);
		memset(buf, 0, sizeof(buf));
		sleep(1);
		ret = read(fd,buf,sizeof(buf));
		printf("AT+FEPHY=on response[%d]: %s\n",ret, buf);
		
		ret = at_write(fd, "AT+FVEW=DISABLE\r", 16);
		memset(buf, 0, sizeof(buf));
		sleep(1);
		ret = read(fd,buf,sizeof(buf));
		printf("AT+FVEW=DISABLE response[%d]: %s\n",ret, buf);
	
		ret = at_write(fd, "AT+FAPSTA=ON\r", 13);
		memset(buf, 0, sizeof(buf));
		sleep(1);
		ret = read(fd,buf,sizeof(buf));
		printf("AT+FAPSTA=ON response[%d]: %s\n",ret, buf);
		
		
		ret=at_write(fd,"AT+RELD\r",8);
		memset(buf, 0, sizeof(buf));
		ret = read(fd,buf,sizeof(buf));
		sleep(20);
		printf("Restore response[%d]: %s\n",ret, buf);
	
		printf("ReEnter CLi-mode 2\n");
		write(fd,p2,1);usleep(200000);
		write(fd,p2,1);usleep(200000);
		write(fd,p2,1);sleep(1);
		write(fd,p,1);
		
		sleep(2);
		memset(buf, 0, sizeof(buf));
		read(fd,buf,sizeof(buf));
		printf(" %s\n",buf);
	
		ret = at_write(fd, "AT+MSLP=ON\r", 11);
		memset(buf, 0, sizeof(buf));
		ret = read(fd,buf,sizeof(buf));
		sleep(1);
		printf("AT+MSLP=ON response[%d]: %s\n",ret, buf);
#endif	
		ret=at_write(fd,"AT+LANN=192.168.3.2,255.255.255.0\r",34);	
		memset(buf, 0, sizeof(buf));
		ret = read(fd,buf,sizeof(buf));
		sleep(1);
		printf("AT+LANN=192.168.3.2,255.255.255.0 response[%d]: %s\n",ret, buf);
	
		ret=at_write(fd,"AT+DHCPDEN=ON\r",14);	
		memset(buf, 0, sizeof(buf));
		ret = read(fd,buf,sizeof(buf));
		sleep(1);
		printf("AT+DHCPDEN=ON response[%d]: %s\n",ret, buf);
	
		ret=at_write(fd,"AT+DHCPGW=192.168.3.1\r",22);	
		memset(buf, 0, sizeof(buf));
		ret = read(fd,buf,sizeof(buf));
		sleep(1);
		printf("AT+DHCPGW=192.168.3.1 response[%d]: %s\n",ret, buf);
	
		ret=at_write(fd,"AT+WAP=11BGN,IMEON-TEST,AUTO\r",29);	
		memset(buf, 0, sizeof(buf));
		ret = read(fd,buf,sizeof(buf));
		sleep(1);
		printf("AT+WAP=11BGN,IMEON-TEST,AUTO response[%d]: %s\n",ret, buf);
	

		ret=at_write(fd,"AT+WAKEY=WPAPSK,TKIPAES,BonjourImeon\r",37);	
		memset(buf, 0, sizeof(buf));
		ret = read(fd,buf,sizeof(buf));
		sleep(1);
		printf("AT+WAKEY=WPAPSK,TKIPAES,BonjourImeon response[%d]: %s\n",ret, buf);
	
	
		ret=at_write(fd,"AT+DOMAIN=IMEON-ENERGY\r",23); 
		memset(buf, 0, sizeof(buf));
		ret = read(fd,buf,sizeof(buf));
		sleep(1);
		printf("AT+DOMAIN=IMEON-ENERGY response[%d]: %s\n",ret, buf);
	
	
		ret=at_write(fd,"AT+WMODE=AP\r",12);	
		memset(buf, 0, sizeof(buf));
		ret = read(fd,buf,sizeof(buf));
		sleep(1);
		printf("AT+WMODE=AP response[%d]: %s\n",ret, buf);
		
		ret=at_write(fd,"AT+Z\r",5);	
		if(ret!=5)	perror("AT+Z");
		printf("Reboot WiFi module, wait 20s\n");
		sleep(20);
			
			printf("ReEnter CLi-mode 3\n");
			write(fd,p2,1);usleep(200000);
			write(fd,p2,1);usleep(200000);
			write(fd,p2,1);sleep(1);
			write(fd,p,1);
			
			sleep(2);
			memset(buf, 0, sizeof(buf));
			read(fd,buf,sizeof(buf));
			printf(" %s\n",buf);
	
		ret = at_write(fd, "AT+MSLP=ON\r", 11);
		memset(buf, 0, sizeof(buf));
		ret = read(fd,buf,sizeof(buf));
		sleep(1);
		printf("AT+MSLP=ON response[%d]: %s\n",ret, buf);
	
		ret=at_write(fd,"AT+WMODE=STA\r",13);	
		memset(buf, 0, sizeof(buf));
		ret = read(fd,buf,sizeof(buf));
		sleep(1);
		printf("AT+WMODE=STA response[%d]: %s\n",ret, buf);
			
		ret=at_write(fd,"AT+WSSSID=MYIR_ELECTRONICS\r",27);	
		memset(buf, 0, sizeof(buf));
		sleep(2);
		ret = read(fd,buf,sizeof(buf));
		printf("AT+WSSSID=MYIR_ELECTRONICS response[%d]: %s\n",ret, buf);
	
		ret=at_write(fd,"AT+WSKEY=WPA2PSK,AES,myir2016\r",30);	
		memset(buf, 0, sizeof(buf));
		sleep(2);
		ret = read(fd,buf,sizeof(buf));
		printf("AT+WSKEY=WPA2PSK,AES,myir2016 response[%d]: %s\n",ret, buf);
			
		ret=at_write(fd,"AT+Z\r",5);	
		if(ret!=5)	perror("AT+z");
//		sleep(1);
//		ret = read(fd,buf,sizeof(buf));
		printf("Reboot WiFi module, wait 20s\n");
		sleep(20);
				
		printf("ReEnter CLi-mode 4\n");
		write(fd,p2,1);usleep(200000);
		write(fd,p2,1);usleep(200000);
		write(fd,p2,1);sleep(1);
		write(fd,p,1);
				
		sleep(5);
		memset(buf, 0, sizeof(buf));
		read(fd,buf,sizeof(buf));
		printf(" %s\n",buf);
	
#if 1
		ret = at_write(fd, "AT+FVER\r", 8);
		memset(buf, 0, sizeof(buf));
		sleep(1);
		ret = read(fd,buf,sizeof(buf));
		printf("AT+FVER=%s\n",buf);
		if(strstr(buf, "n") == NULL){
			printf("AT+FVER  check error.\r\n");
				err_on_wifi = 22;
		}
		else
		{
			printf("AT+FVER check success\r\n");
				err_on_wifi = 20;
		}
#endif		
			ret=at_write(fd,"AT+WSLK\r",8); 
			memset(buf, 0, sizeof(buf));
			sleep(1);
			ret = read(fd,buf,sizeof(buf));
			printf("AT+WSLK response[%d]: %s\n",ret, buf);
			if(strstr(buf, cmd_wsssid) == NULL){
				printf("AT+WSLK check error\r\n");
//				err_on_wifi += 1 ;
			}
			else
			{
				printf("AT+WSLK check success\r\n");
			}
			
			memset(buf, 0, sizeof(buf));
			ret=at_write(fd,"AT+PING=www.baidu.com\r",22); 
			sleep(1);
			ret = read(fd,buf,sizeof(buf));
			printf("AT+PING response[%d]: %s\n",ret, buf);
			if(strstr(buf, "Success") == NULL){
				printf("AT+PING=www.baidu.com check error\r\n");
			}
			else
			{
				printf("AT+PING=www.baidu.com check success\r\n");
			}
			close(fd);
}



/*
int main()
{
        wifi_ap_enable("/dev/ttyO5",57600,8,'N',1);
        return 0;
}
*/
