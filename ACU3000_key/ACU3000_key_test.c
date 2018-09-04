#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#define TIMEOUT			2000 /* 1 second */
#define DEFAULT_TRIGGER	"both"
#define KEY_BASE		"/sys/class/gpio/gpio57"
#define KEY_LED			"/sys/class/gpio/gpio107/value"

int set_edge(const char *trigger)
{
	int fd_edge = -1;

	fd_edge = open(KEY_BASE"/edge", O_RDWR);
	if (fd_edge < 0) {
		perror(KEY_BASE"/edge");
		goto err_exit;
	}
	
	if (write(fd_edge, trigger, strlen(trigger)+1) <= 0) {
		fprintf(stderr, "set edge %s failed\n", trigger);
		goto err_exit;
	}
	close(fd_edge);
	
	return 0;
	
err_exit:
	if (fd_edge > 0)
		close(fd_edge);
	return -1;
}

int set_led(int on)
{
	int fd_led = -1;
	
	fd_led = open(KEY_LED, O_RDWR);
	if (fd_led < 0) {
		perror(KEY_LED);
		goto err_exit;
	}
	if (write(fd_led, on?"1":"0", 2) <=0 ){
		fprintf(stderr, "set led failed\n");
		goto err_exit;
	}
	close(fd_led);
	return 0;
	
err_exit:
	if (fd_led > 0)
		close(fd_led);
	return -1;
}

int poll_key(void)
{
	struct pollfd fds[1];
	int fd_key = -1;
	int ret = -1;
	int i = 0;
	char buf[50];
	
	fd_key = open(KEY_BASE"/value", O_RDONLY);
	if (fd_key < 0) {
		perror(KEY_BASE"/value");
		goto err_exit;
	}

	/* We need to do a dummy read before polling. */
	if (read(fd_key, buf, sizeof(buf)) <= 0) {
		fprintf(stderr, "read %s failed\n", KEY_BASE"/value");
		goto err_exit;
	}
	
	do{
		memset(fds, 0, sizeof(fds));
		fds[0].fd = fd_key;
		fds[0].events= POLLPRI;
		
		ret = poll(fds, 1, TIMEOUT);/* wait for 1 second */
		if (ret < 0) {
			perror("poll error");
			continue;
		} else if (ret == 0) {
			printf("polling...\n");
		} else if (fds[0].revents & POLLPRI) {
			lseek(fd_key, 0L, SEEK_SET);
			memset(buf, 0, sizeof(buf));
			if (read(fd_key, buf, sizeof(buf)) <= 0) {
				fprintf(stderr, "read %s failed\n", KEY_BASE"/value");
			} else {
				printf("interrupt triggered! read value: %s\n", buf);
				set_led(buf[0]=='0'?1:0);
				i=1;;
			}
		} else {
			fprintf(stderr, "unknown event, revents: %#X\n", fds[0].revents);
		}
	}while(i==0);
	close(fd_key);
	return 0;
err_exit:
	if(fd_key > 0)
		close(fd_key);
	return -1;
}

void usage(void)
{
	printf( "Usage: key_test <edge>\n"
			"  edge - trigger method: rising|falling|both\n\n"
			);
}

int ACU3000_key_test()
{

	set_edge(DEFAULT_TRIGGER);
	
	poll_key();
	
	return 0;
}
