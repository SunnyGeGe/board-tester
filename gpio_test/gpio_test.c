#include <stdlib.h>
#include <linux/fcntl.h>
#include <string.h>

#include "gpio_test.h"
#include "../common.h"

#define PR_GPIO_DIR "/sys/class/gpio/"

int gpio_export(const int gpio,
					 const char *dir)
{
	int fd;
	char dir_path[32] = { '\0' };
	char gpio_str[8] =  { '\0' };
	int  len;
	char dir_verify[4] = { '\0' };

	fd = open(PR_GPIO_DIR"export", O_WRONLY);
	if (fd < 0) {
		dbg_perror("open gpio export");
		return -1;
	}

	sprintf(gpio_str, "%d", gpio);
	len = strlen(gpio_str);
	if (write(fd, gpio_str, len) < len) {
//		dbg_perror("write to export");
		goto _ERR;
	}

	close(fd);

	sprintf(dir_path, PR_GPIO_DIR"gpio%d/direction", gpio);
	fd = open(dir_path, O_RDWR);
	if (fd < 0) {
		dbg_perror("open gpio direction");
		return -1;
	}

	write(fd, dir, strlen(dir));
	if (read(fd, dir_verify, 3) < 0) {
		dbg_perror("read direction back");
		goto _ERR;
	}

	if (strncmp(dir, dir_verify, strlen(dir)) != 0) {
		dbg_printf("set direction failed\n");
		goto _ERR;
	}

	close(fd);
	
	return 0;

 _ERR:
	close(fd);
	return -1;
}

int gpio_set_value(const int gpio, const char value)
{
	int  fd;
	char value_path[32] = { '\0' };
	char value_verify;

	sprintf(value_path, PR_GPIO_DIR"gpio%d/value", gpio);
	fd = open(value_path, O_RDWR);
	if (fd < 0) {
		dbg_perror("open gpio value");
		return -1;
	}

	write(fd, &value, 1);
	if (read(fd, &value_verify, 2) < 1) {
		dbg_perror("read value back");
		close(fd);
		return -1;
	}
/* bug, while gpio set out mode, 'value_verify' maybe not equal to 'value' set
	if (value != &value_verify) {
		dbg_printf("set value failed\n");
		printf("value=%d,value_verify=%d\n",value,value_verify);
		close(fd);
		return -1;
	}
*/
	close(fd);

	return 0;
}
