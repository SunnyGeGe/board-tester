#include <getopt.h>
#include <sys/wait.h>
#include <linux/input.h>
#include <fcntl.h>

#include "common.h"
#include "gpio_test/gpio_test.h"

static int expect_usb_cnt = 3;
int err_on_testing = 0;

const static int err_flash_interval = 100000;
const static int pass_flash_interval = 500000;

const static char menu[] = {
	"CANADA SAM9X5 TEST PROGRAM >>>>\n"
	" 1. KEYs\n"
	" 2. eth0\n"
	" 3. eth1\n"
	" 4. AUX uart\n"
	" 5. GPIO\n"
	" 6. USB\n"
	" 7. Update boot images\n"
	" 8. exit\n"
};

static char leds_path[][64] = {
	"/sys/class/leds/internet/brightness",
	"/sys/class/leds/payment/brightness",
	"/sys/class/leds/app/brightness"
};

static char result_list[64][64];
static int  result_idx = 0;
static pthread_t keytest_thread_id = -1;
static pthread_t ledtest_thread_id;

static void do_test(int automatic)
{
	int  sel = 1;
	char usb_mp[5][64];
	char usb_cnt = 0;
	int  usb_idx = 0;
	int  key_codes_list[] = { 50, 25, 173, 0x186, 30, 28, 0x161, 31, 23 };
	int  ret;

	sleep(1);

	while (1) {
		result_idx > 63 ? result_idx = 0 : NULL;

		if (!automatic) {
			printf("\n\n%s", menu);
			printf("choice>  ");
			scanf("%d", &sel);
		}

		switch(sel) {
		case 1:
			printf("testing keys...\n");

			if (ret = (create_keytest_thread("/dev/input/event0",
											 key_codes_list,
											 sizeof(key_codes_list) / 4,
											 &keytest_thread_id,
											 2) < 0))
				err_on_testing = 1;

			if (!automatic)
				break;

		case 2:
			printf("testing eth0...\n");
			
			if (ret = eth_test("eth0", "192.168.1.113", "192.168.1.128") != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"2. %-15s %s", "eth0", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 3:
			printf("testing eth1...\n");
			
			if (ret = eth_test("eth1", "192.168.2.223", "192.168.2.128") != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"3. %-15s %s", "eth1", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 4:
			printf("testing AUX uart ...\n");

			if (ret = uart_test("/dev/ttyS5", 115200, 8, 'N', 1, 0, 100, 15, 50000) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"4. %-15s %s", "AUX uart", ret == 0 ? "ok" : "err");
			
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 5:
			printf("testing GPIO ...\n");

			ret = system("./gpiotest.sh");
			if ((ret = WEXITSTATUS(ret)) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"5. %-15s %s", "GPIO", ret == 0 ? "ok" : "err");
			
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
			
		case 6:
			printf("testing USB ...\n");

			usb_cnt = get_usb_mp("/media", usb_mp);
			for (usb_idx = 0; usb_idx < usb_cnt; ++usb_idx) {
				if (ret = storage_test(usb_mp[usb_idx], "MYiR_test.txt") < 0)
					err_on_testing = 1;

				sprintf(result_list[result_idx++],
						"6. %-15s %s", usb_mp[usb_idx], ret == 0 ? "ok" : "err");

				if (!automatic)
					printf("%s\n", result_list[result_idx - 1]);
			}
			
			if (usb_cnt < expect_usb_cnt) {
				printf("\t*** Expect %d USBs but just found %d ***\n", expect_usb_cnt, usb_cnt);
				err_on_testing = 1;
				sprintf(result_list[result_idx++], "5. %-15s %s", "usb test", "err");
			}

			if (!automatic)
				break;

		case 7:
			while (!key_test_thread_exit);
			printf("Update boot images ...\n");

			if (err_on_testing != 0 && automatic)
				return;

			if (create_ledtest_thread(leds_path,
									  3,
									  300000,
									  0,
									  &ledtest_thread_id) < 0)
				printf("*** Create led_test thread failed ***\n");

			ret = system("./update_boot.sh");
			if ((ret = WEXITSTATUS(ret)) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"7. %-15s %s", "Update boot images", ret == 0 ? "ok" : "err");
			
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
		case 8:
			return;
			
		default:
			break;
		}

		if (automatic)
			return;
	}
}

static void print_result_list(void)
{
	int idx;

	if (result_idx < 1)
		return;
	
	printf("\n\n************* result list *************\n");
		
	for (idx = 0; idx < result_idx; ++idx)
		printf("%s\n", result_list[idx]);
}

static int roast(void)
{
	int key_fd;
	struct input_event ev;
	int ret;

	fd_set fds;
	struct timeval tv;

	key_fd = open("/dev/input/event0", O_RDONLY);
	if (key_fd < 0) {
		dbg_perror("Open keys event node failed");
		return -1;
	}

	tv.tv_sec = 3;
	tv.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(key_fd, &fds);

	ret = select(key_fd + 1, &fds, NULL, NULL, &tv);
	if (ret == -1) {
		dbg_perror("Select key fd");
		goto __ERR;
	} else if (ret == 0) {
		dbg_printf("Select key fd timeout\n");
		goto __ERR;
	}

	ret = read(key_fd, &ev, sizeof(ev));
	if (ret < 0) {
		dbg_perror("Read from key fd");
		goto __ERR;
	}

	return 0;
	
 __ERR:
	close(key_fd);
	return -1;
}
		
int main(int argc, char *argv[])
{
	int codes_list[] = { 1, 102 };
	
	pthread_t audiotest_thread_id;

	int flash_interval = pass_flash_interval;

	int automatic = 0;
	int next_option;
	const char *short_options = "a";
	const struct option options[] = {
		{ "auto", 0, NULL, 'a' },
		{ NULL, 0, NULL, 0}
	};
	
	while (1) {
		next_option = getopt_long(argc, argv, short_options, options, NULL);
		if (next_option < 0)
			break;

		switch (next_option) {
		case 'a':
			automatic = 1;
			break;
		default:
			break;
		}
	}

	if (automatic) {
		if (create_ledtest_thread(leds_path,
								  3,
								  1000000,
								  1,
								  &ledtest_thread_id) < 0)
			printf("*** Create led_test thread failed ***\n");

		/* Roast or test? */
		if (roast() == 0)
			system("./myir_stress -l /sys/class/leds/payment/brightness");

		/* Initialize buzzer and ring 100ms */
		gpio_export(113, "out");
		gpio_set_value(113, '1');
		usleep(100000);
		gpio_set_value(113, '0');

		do_test(1);
	} else
		do_test(0);

	/*
	 * Wait key test thread terminate,
	 * why can not use phtread_join here?
	 */
	while (!key_test_thread_exit);
	
	print_result_list();

	if (err_on_testing) {
		lighten_all_led(1);
		/* ring buzzer */
		gpio_set_value(113, '1');
	} else {
		while (1) {
			gpio_set_value(113, '1');
			usleep(80000);
			gpio_set_value(113, '0');
			usleep(1000000);
		}
	}
	while (1);
	
	return 0;
}
