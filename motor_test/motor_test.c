#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "../common.h"

#define MOTOR_STOP	0
#define MOTOR_RIGHT	1
#define MOTOR_LEFT	2

#define CTRL_H	"0"
#define CTRL_L	"1"


int motor_test(const char *dev1,const char *dev2,const char *dev3, int op)
{
	int fd_in1 = -1;
	int fd_in2 = -1;
	int fd_en = -1;	

	fd_in1 = open(dev1, O_WRONLY);
	if (fd_in1 < 0) {
		perror(dev1);
		goto err_exit;
	}
	fd_in2 = open(dev2, O_WRONLY);
	if (fd_in2 < 0) {
		perror(dev2);
		goto err_exit;
	}
	fd_en = open(dev3, O_WRONLY);
	if (fd_en < 0) {
		perror(dev3);
		goto err_exit;
	}

	switch(op) {
		case MOTOR_STOP:
			if (write(fd_in1, CTRL_L, 1) <= 0) {
				fprintf(stderr, "write %s failed\n", dev1);
				goto err_exit;
			}
			if (write(fd_in2, CTRL_L, 1) <= 0) {
				fprintf(stderr, "write %s failed\n", dev2);
				goto err_exit;
			}

			printf("stop motor\n");
			break;
		case MOTOR_RIGHT:
			if (write(fd_in1, CTRL_L, 1) <= 0) {
				fprintf(stderr, "write %s failed\n", dev1);
				goto err_exit;
			}
			if (write(fd_in2, CTRL_H, 1) <= 0) {
				fprintf(stderr, "write %s failed\n", dev2);
				goto err_exit;
			}
			if (write(fd_en, CTRL_H, 1) <= 0) {
				fprintf(stderr, "write %s failed\n", dev3);
				goto err_exit;
			}
			printf("turn right\n");
			break;
		case MOTOR_LEFT:
			if (write(fd_in1, CTRL_H, 1) <= 0) {
				fprintf(stderr, "write %s failed\n", dev1);
				goto err_exit;
			}
			if (write(fd_in2, CTRL_L, 1) <= 0) {
				fprintf(stderr, "write %s failed\n", dev2);
				goto err_exit;
			}
			if (write(fd_en, CTRL_H, 1) <= 0) {
				fprintf(stderr, "write %s failed\n", dev3);
				goto err_exit;
			}
			printf("turn left\n");
			break;
		default:
			fprintf(stderr, "unsupported operation!\n");
			goto err_exit;
	}
	
	close(fd_in1);
	close(fd_in2);
	close(fd_en);
	
	return 0;

err_exit:
	if (fd_in1 > 0)
		close(fd_in1);
	if (fd_in2 > 0)
		close(fd_in2);
	if (fd_en > 0)
		close(fd_en);
	
	return -1;
}
