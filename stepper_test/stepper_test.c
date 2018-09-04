#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>

#include "../common.h"

#define STEPPER_NUMBER	3
#define DEFAULT_SPEED	10
#define DEFAULT_STEPS	50

#define DIR_P	0 /* positive */
#define DIR_N	1 /* negative */

#define STATE_L	"0"
#define STATE_H	"1"
static const char * dir_str[2] = {"0","1"};

int stepper_test(const char *dev1,const char *dev2,int dir,int speed,int cnt)
{
	int fd_pulse = -1;
	int fd_dir = -1;
	int delay_us;
	int count = 0;

	delay_us = (1000000 / speed) / 2;
	
	delay_us = delay_us <=0 ? 1 : delay_us;
	
	count = cnt;
	
	fd_pulse = open(dev1, O_WRONLY);
	if (fd_pulse < 0) {
		perror(dev1);
		goto err_exit;
	}
	fd_dir = open(dev2, O_WRONLY);
	if (fd_dir < 0) {
		perror(dev2);
		goto err_exit;
	}

	/* set direction */
	if (write(fd_dir, dir_str[dir], strlen(dir_str[dir])) <= 0) {
		fprintf(stderr, "set direction failed(%s write failed)\n", dev2);
		goto err_exit;
	}
	
	/* init pulse state to LOW */
	if (write(fd_pulse, STATE_L, strlen(STATE_L)) <= 0) {
		fprintf(stderr, "set pulse failed(%s write failed)\n", dev1);
		goto err_exit;
	}
	printf("stepping ... ");
	fflush(stdout);
	/* infinite loop */
	while (count --) {
		usleep(delay_us);
		if (write(fd_pulse, STATE_H, strlen(STATE_H)) <= 0) {
			fprintf(stderr, "set pulse failed(%s write failed)\n", dev1);
			goto err_exit;
		}
		usleep(delay_us);
		if (write(fd_pulse, STATE_L, strlen(STATE_L)) <= 0) {
			fprintf(stderr, "set pulse failed(%s write failed)\n", dev1);
			goto err_exit;
		}
	}
	printf("done.\n");

	printf("dir: %s, speed: %d, total steps: %d\n", 
		dir==0?"positive":"negative",
		speed,
		cnt);
	
	return 0;
err_exit:
	if (fd_pulse > 0)
		close(fd_pulse);
	if (fd_dir > 0)
		close(fd_dir);
	
	return -1;
}