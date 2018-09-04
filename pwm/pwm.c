#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define PWM_1	"/sys/class/leds/pwm_1/brightness"
#define PWM_2	"/sys/class/leds/pwm_2/brightness"
#define MAX_DUTY	255

int i = 200;

int pwm_test()
{
	int fd_pwm1 = -1;
	int fd_pwm2 = -1;
	int duty = 0;
	char buf[20];
	
	fd_pwm1 = open(PWM_1, O_RDWR);
	if (fd_pwm1 < 0) {
		perror(PWM_1);
		goto err_exit;
	}

	fd_pwm2 = open(PWM_2, O_RDWR);
	if (fd_pwm2 < 0) {
		perror(PWM_2);
		goto err_exit;
	}
	
	printf("please look the light's change\n");
	
	do {
		sprintf(buf, "%d", duty);
		if (write(fd_pwm1, buf, strlen(buf)) <= 0) {
			perror("Set PWM_1");
		}
		sprintf(buf, "%d", MAX_DUTY - duty);
		if (write(fd_pwm2, buf, strlen(buf)) <= 0) {
			perror("Set PWM_2");
		}
		duty -= 20;
		if (duty <= 0) {
			duty = 255;
		}
		
		/* Delay 50 ms */
		usleep(50*1000);
	}while(i--);
	
	return 0;
	
err_exit:
	if (fd_pwm1 > 0) 
		close(fd_pwm1);
		
	if (fd_pwm2 > 0)
		close(fd_pwm2);
		
	return -1;
}
