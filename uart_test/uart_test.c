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

#include "../common.h"

#ifndef TIOCSRS485
    #define TIOCSRS485     0x542F
#endif

#ifndef TIOCGRS485
    #define TIOCGRS485     0x542e
#endif

int re = 0;
int pr = 0;
const static char *UART_TEST_STR = "MYiR UART TEST";
typedef enum {DISABLE = 0, ENABLE} RS485_ENABLE_t;

int open_tty(const char *tty_dev)
{
	int fd;
	fd = open(tty_dev, O_RDWR);
	if(fd < 0)
		dbg_perror("open tty");

	return fd;
}

int set_tty(int fd,
				   int bitrate,
				   int datasize,
				   char par,
				   int stop)
{
	struct termios newtio;

	/* ignore modem control lines and enable receiver */
	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	/* set character size */
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
	
	/* set the parity */
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
	
	/* set the stop bits */
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
	
	/* set output and input baud rate */
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
	
	/* set timeout in deciseconds for non-canonical read */
	newtio.c_cc[VTIME] = 0;
	/* set minimum number of characters for non-canonical read */
	newtio.c_cc[VMIN] = strlen(UART_TEST_STR);

	/* flushes data received but not read */
	tcflush(fd, TCIFLUSH);
	/* set the parameters associated with the terminal from
	   the termios structure and the change occurs immediately */
	if((tcsetattr(fd, TCSANOW, &newtio))!=0) {
		dbg_perror("set_tty/tcsetattr");
		return -1;
	}

	return 0;
}

int rs485_enable(const int fd, const RS485_ENABLE_t enable)
{
	struct serial_rs485 rs485conf;
	int res;

	/* Get configure from device */
	res = ioctl(fd, TIOCGRS485, &rs485conf);
	if (res < 0) {
		perror("Ioctl error on getting 485 configure:");
		close(fd);
		return res;
	}

	/* Set enable/disable to configure */
	if (enable) {   // Enable rs485 mode
		rs485conf.flags |= SER_RS485_ENABLED;
	} else {        // Disable rs485 mode
		rs485conf.flags &= ~(SER_RS485_ENABLED);
	}

	rs485conf.delay_rts_before_send = 0x00000004;

	/* Set configure to device */
	res = ioctl(fd, TIOCSRS485, &rs485conf);
	if (res < 0) {
		perror("Ioctl error on setting 485 configure:");
		close(fd);
	}

	return res;
}

int chk_str(const char *a_str, const char *b_str, int count)
{
	static int idx = 0;
	int i;

	for (i = 0; i < count; ++i) {
		if (a_str[idx] != b_str[i]) {
			dbg_printf("%d : %d, %c : %c\n", idx, i, a_str[idx], b_str[i]);
			idx = 0;
			return -1;
		}
		++idx >= strlen(a_str) ? idx = 0 : NULL;
	}

	return 0;
	
#if 0
	int a_idx = 0;
	int b_idx;
	int a_len = strlen(a_str);

	for (b_idx = 0; b_idx < count; ++b_idx) {
		if (b_str[b_idx] == a_str[a_idx])
			++a_idx;
		else if (a_idx > 1)
			return -1;
	}

	if (a_idx < 1)
		return -1;
	else
		return 0;
#endif
}

int uart_test(const char *tty_dev,
			  int bitrate,
			  int datasize,
			  char par,
			  int stop,
			  int rs485_mode,
			  int times,
			  int timeout,
			  int interval)
{
	int tty_fd;
	int cnt = 0;
	char read_back[16];
	int str_len = strlen(UART_TEST_STR);
	struct timeval tv;
	fd_set fds;
	int ret;

	tty_fd = open_tty(tty_dev);
	if (tty_fd < 1)
		return -1;

	if (set_tty(tty_fd, bitrate, datasize, par, stop) < 0)
		goto __ERR;

	if (rs485_mode) {
		/* TODO: setting 485 mode */
		rs485_enable(tty_fd, ENABLE);
	}

	tv.tv_sec  = timeout;
	tv.tv_usec = 0;
	
	while (times < 1 || cnt++ < times) {
		if (write(tty_fd, UART_TEST_STR, str_len) < 0) {
			dbg_perror("uart write");
			goto __ERR;
		}
		// dbg_printf("%s write: %s\n", tty_dev, UART_TEST_STR);

		FD_ZERO(&fds);
		FD_SET(tty_fd, &fds);

		if (timeout > 0) {
			ret = select(tty_fd + 1, &fds, NULL, NULL, &tv);
			if (ret == -1) {
				dbg_perror("select uart");
				goto __ERR;
			} else if (ret == 0) {
				dbg_printf("select %s timeout\n", tty_dev);
				goto __ERR;
			}
		}
		
		memset(read_back, '\0', sizeof(read_back));
		ret = read(tty_fd, read_back, str_len);
		if (ret < 0) {
			dbg_perror("uart write");
			goto __ERR;
		} else if (ret == 0) {
			dbg_printf("%s timeout at read\n", tty_dev);
			goto __ERR;
		}
//		printf("uart/rs485 read %s\n",read_back);
		
//		dbg_printf("%s read : %s\n", tty_dev, read_back);

		/* Verify data */
		if (chk_str(UART_TEST_STR, read_back, ret) < 0) {
			dbg_printf("Verify %s data failed (%s)\n", tty_dev, read_back);
			goto __ERR;
		}
	
		usleep(interval);
	}

	dbg_printf("\n========== %s test pass ===========\n\n", tty_dev);

	close(tty_fd);
	
	return 0;
 __ERR:
	close(tty_fd);
	return -1;

}
/*
int uart_test2(const char *tty_dev,
			  int bitrate,
			  int datasize,
			  char par,
			  int stop,
			  int rs485_mode,
			  int times,
			  int timeout,
			  int interval)
{
	int tty_fd;
	int cnt = 0;
	char read_back[16];
	int str_len = strlen(UART_TEST_STR);
	struct timeval tv;
	fd_set fds;
	int ret;

	tty_fd = open_tty(tty_dev);
	if (tty_fd < 1)
		return -1;

	if (set_tty(tty_fd, bitrate, datasize, par, stop) < 0)
		goto __ERR;

	if (rs485_mode) {
		rs485_enable(tty_fd, ENABLE);
	}

	tv.tv_sec  = timeout;
	tv.tv_usec = 0;
	
	while (times < 1 || cnt++ < times) {

		ret = read(tty_fd, read_back, str_len);
                if (ret < 0) {
                        dbg_perror("uart write");
                        goto __ERR;
                } else if (ret == 0) {
                        dbg_printf("%s timeout at read\n", tty_dev);
                        goto __ERR;
                }
		// dbg_printf("%s write: %s\n", tty_dev, UART_TEST_STR);

		FD_ZERO(&fds);
		FD_SET(tty_fd, &fds);

		if (timeout > 0) {
			ret = select(tty_fd + 1, &fds, NULL, NULL, &tv);
			if (ret == -1) {
				dbg_perror("select uart");
				goto __ERR;
			} else if (ret == 0) {
				dbg_printf("select %s timeout\n", tty_dev);
				goto __ERR;
			}
		}
		
		memset(read_back, '\0', sizeof(read_back));
		ret = read(tty_fd, read_back, str_len);
		if (ret < 0) {
			dbg_perror("uart write");
			goto __ERR;
		} else if (ret == 0) {
			dbg_printf("%s timeout at read\n", tty_dev);
			goto __ERR;
		}
//		printf("uart/rs485 read %s\n",read_back);
		
		dbg_printf("%s read : %s\n", tty_dev, read_back);

		if (chk_str(UART_TEST_STR, read_back, ret) < 0) {
			dbg_printf("Verify %s data failed (%s)\n", tty_dev, read_back);
			goto __ERR;
		}
		if(write(tty_fd,UART_TEST_STR,str_len)<0)
		{
            dbg_perror("uart write");
            goto __ERR;
        }

		usleep(interval);
	}

	dbg_printf("\n========== %s test pass ===========\n\n", tty_dev);

	close(tty_fd);
	
	return 0;
 __ERR:
	close(tty_fd);
	return -1;
}
*/
int uart_test2(const char *tty_dev,
			  int bitrate,
			  int datasize,
			  char par,
			  int stop,
			  int rs485_mode,
			  int times,
			  int timeout,
			  int interval)
{
	int tty_fd;
	int cnt = 0;
	char read_back[16];
	int str_len = strlen(UART_TEST_STR);
	struct timeval tv;
	fd_set fds;
	int ret;

	tty_fd = open_tty(tty_dev);
	if (tty_fd < 1)
		return -1;

	if (set_tty(tty_fd, bitrate, datasize, par, stop) < 0)
		goto __ERR;

	if (rs485_mode) {
		/* TODO: setting 485 mode */
		rs485_enable(tty_fd, ENABLE);
	}

	tv.tv_sec  = timeout;
	tv.tv_usec = 0;
	
	while (times < 1 || cnt++ < times) {
		if (write(tty_fd, UART_TEST_STR, str_len) < 0) {
			dbg_perror("uart write");
			goto __ERR;
		}
                if (write(tty_fd, UART_TEST_STR, str_len) < 0) {
                        dbg_perror("uart write");
                        goto __ERR;
                }
		// dbg_printf("%s write: %s\n", tty_dev, UART_TEST_STR);

		FD_ZERO(&fds);
		FD_SET(tty_fd, &fds);

		if (timeout > 0) {
			ret = select(tty_fd + 1, &fds, NULL, NULL, &tv);
			if (ret == -1) {
				dbg_perror("select uart");
				goto __ERR;
			} else if (ret == 0) {
				dbg_printf("select %s timeout\n", tty_dev);
				goto __ERR;
			}
		}
		
		memset(read_back, '\0', sizeof(read_back));
		ret = read(tty_fd, read_back, str_len);
		if (ret < 0) {
			dbg_perror("uart write");
			goto __ERR;
		} else if (ret == 0) {
			dbg_printf("%s timeout at read\n", tty_dev);
			goto __ERR;
		}
		usleep(interval);
        //printf("uart/rs485 read %s\n",read_back);
		
		dbg_printf("%s read : %s\n", tty_dev, read_back);

		/* Verify data */
		if (chk_str(UART_TEST_STR, read_back, ret) < 0) {
			dbg_printf("Verify %s data failed (%s)\n", tty_dev, read_back);
			goto __ERR;
		}
	
	}

	dbg_printf("\n========== %s test pass ===========\n\n", tty_dev);

	close(tty_fd);
	
	return 0;
 __ERR:
        close(tty_fd);
        return -1;
}

void creat_uart1_thread(void)
{
	int ret = uart_test("/dev/ttyS2", 115200, 8, 'N', 1, 0, 100, 15, 50000);
	if(ret != 0)
	{
		//err_on_testing=1;pr=-1;
	}
	pthread_exit(0);
}

void creat_uart2_thread(void)
{
	int ret = uart_test2("/dev/spi_usart0", 115200, 8, 'N', 1, 0, 100, 15, 50000);
	if(ret != 0)
	{
		//err_on_testing=1;pr=-1;
}

	pthread_exit(0);
}
/*IMEON TTL NODE*/
void creat_uart3_thread(void)
{
        int ret = uart_test2("/dev/ttyO2", 115200, 8, 'N', 1, 0, 100, 15, 50000);
        if(ret != 0)
                err_on_testing;
        pthread_exit(0);
}
/*IMEON 232 NODE*/
void creat_uart4_thread(void)
{
        int ret = uart_test("/dev/ttyO1", 115200, 8, 'N', 1, 0, 100, 15, 50000);
        if(ret != 0)
                err_on_testing;
        pthread_exit(0);
}

void creat_rs485_1_thread(void)
{
	int ret = uart_test("/dev/spi_usart1", 115200, 8, 'N', 1, 1, 100, 15, 50000);
        if(ret != 0)
	{
                err_on_testing=1;pr=-1;}
	pthread_exit(0);
}
void creat_rs485_2_thread(void)
{
	int ret = uart_test2("/dev/ttyS4", 115200, 8, 'N', 1, 1, 100, 15, 50000);
	if(ret != 0)
	{
		err_on_testing=1;pr=-1;}
	pthread_exit(0);
}

void creat_rs485_5_thread(void)
{
        int ret = uart_test2("/dev/ttyS1", 115200, 8, 'N', 1, 1, 100, 15, 50000);
        if(ret != 0)
        {
                err_on_testing=1;pr=-1;}
        pthread_exit(0);
}


void creat_rs485_3_thread(void)
{
        int ret = uart_test2("/dev/ttyO3", 115200, 8, 'N', 1, 0, 100, 15, 50000);
        if(ret != 0)
	{
                err_on_testing=1;pr=-1;}

        pthread_exit(0);
}
void creat_rs485_4_thread(void)
{
        int ret = uart_test2("/dev/ttyO4", 115200, 8, 'N', 1, 0, 100, 15, 50000);
        if(ret != 0)
	{
                err_on_testing=1;pr=-1;}

        pthread_exit(0);
}


void creat_usb_0_thread(void)
{
        int ret = uart_test("/dev/ttyUSB0", 115200, 8, 'N', 1, 0, 100, 15, 50000);
        if(ret != 0)
	{
                err_on_testing=1;pr=-1;}

        pthread_exit(0);
}

void creat_usb_1_thread(void)
{
        int ret = uart_test("/dev/ttyUSB1", 115200, 8, 'N', 1, 0, 100, 15, 50000);
        if(ret != 0)
	{
                err_on_testing=1;pr=-1;}

        pthread_exit(0);
}


