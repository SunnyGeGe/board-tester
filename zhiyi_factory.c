#include <getopt.h>
#include <sys/wait.h>

#include "common.h"

static int expect_usb_cnt = 1;
int err_on_testing = 0;

const static int err_flash_interval = 100000;
const static int pass_flash_interval = 500000;

const static char menu[] = {
	"ZHIYI BOARD TEST PROGRAM >>>>\n"
	" 1. Camera(i2c0)\n"
	" 2. UART0\n"
	" 3. UART1\n"
	" 4. Eeprom(i2c1)\n"
	" 5. GPIO0~7\n"
	" 6. USB\n"
	" 7. exit\n"
};

static char result_list[64][64];
static int  result_idx = 0;

static void do_test(int automatic)
{
	int sel = 1;
	char usb_mp[5][64];
	char usb_cnt = 0;
	int  usb_idx = 0;
	int ret;

	sleep(1);

	while (1) {
		if (!automatic) {
			printf("\n\n%s", menu);
			printf("choice>  ");
			scanf("%d", &sel);
		}

		switch(sel) {
		case 1:
			printf("testing camera(i2c0)...\n");

			if (ret = camera_test(1, NULL) != 0)
				err_on_testing = 1;
			else {
				ret = system("./usb_install.sh");
				if ((ret = WEXITSTATUS(ret)) != 0)
					err_on_testing = 1;
			}
			
			if (!automatic) {
				printf("\n1. %-15s %s\n", "camera", ret == 0 ? "ok" : "err");
				break;
			} else {
				sprintf(result_list[result_idx++],
						"1. %-15s %s", "camera", ret == 0 ? "ok" : "err");
			}

		case 2:
			printf("testing uart0 ...\n");

			if (ret = uart_test("/dev/ttyS2", 115200, 8, 'N', 1, 0, 100, 15, 50000) != 0)
				err_on_testing = 1;
			
			if (!automatic) {
				printf("\n2. %-15s %s\n", "uart0", ret == 0 ? "ok" : "err");
				break;
			} else {
				sprintf(result_list[result_idx++],
						"2. %-15s %s", "uart0", ret == 0 ? "ok" : "err");
			}

		case 3:
			printf("testing uart1 ...\n");

			if (ret = uart_test("/dev/ttyS1", 115200, 8, 'N', 1, 0, 100, 15, 50000) != 0)
				err_on_testing = 1;
			
			if (!automatic) {
				printf("\n3. %-15s %s\n", "uart1", ret == 0 ? "ok" : "err");
				break;
			} else {
				sprintf(result_list[result_idx++],
						"3. %-15s %s", "uart1", ret == 0 ? "ok" : "err");
			}

		case 4:
			printf("testing Eeprom(i2c1) ...\n");

			if (ret = eeprom_test() != 0)
				err_on_testing = 1;
			
			if (!automatic) {
				printf("\n4. %-15s %s\n", "Eeprom(i2c1)", ret == 0 ? "ok" : "err");
				break;
			} else {
				sprintf(result_list[result_idx++],
						"4. %-15s %s", "Eeprom(i2c1)", ret == 0 ? "ok" : "err");
			}

		case 5:
			printf("testing GPIO ...\n");

			ret = system("./gpiotest.sh");
			if ((ret = WEXITSTATUS(ret)) != 0)
				err_on_testing = 1;
			
			if (!automatic) {
				printf("\n5. %-15s %s\n", "GPIO", ret == 0 ? "ok" : "err");
				break;
			} else {
				sprintf(result_list[result_idx++],
						"5. %-15s %s", "GPIO", ret == 0 ? "ok" : "err");
			}
			
		case 6:
			printf("testing USB ...\n");
			usb_cnt = get_usb_mp(usb_mp);
			for (usb_idx = 0; usb_idx < usb_cnt; ++usb_idx) {
				if (ret = storage_test(usb_mp[usb_idx], "MYiR_test.txt") < 0)
					err_on_testing = 1;
				if (!automatic)
					printf("6. %-15s %s\n", usb_mp[usb_idx], ret == 0 ? "ok" : "err");
				else
					sprintf(result_list[result_idx++],
							"6. %-15s %s", usb_mp[usb_idx], ret == 0 ? "ok" : "err");
			}

			if (usb_cnt < expect_usb_cnt) {
				printf("\t*** Expect %d USBs but just found %d ***\n", expect_usb_cnt, usb_cnt);
				err_on_testing = 1;
				sprintf(result_list[result_idx++],
						"6. %-15s %s", "usb test", "err");
			}

			if (!automatic)
				break;
			
		case 7:
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

int main(int argc, char *argv[])
{
	int codes_list[] = { 1, 102 };
	char leds_path[][64] = {
		"/sys/class/leds/d6/brightness",
		"/sys/class/leds/d16/brightness"
	};
	
	pthread_t keytest_thread_id;
	pthread_t audiotest_thread_id;
	pthread_t ledtest_thread_id;

	int flash_interval = pass_flash_interval;

	int automatic = 0;
	int next_option;
	const char *short_options = "a";
	const struct option options[] = {
		{ "auto", 0, NULL, 'a' },
		{ NULL, 0, NULL, 0}
	};

	system("./led.sh init");
	system("./led.sh on");
	
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

	if (automatic)
		do_test(1);
	else
		do_test(0);

	print_result_list();

	if (err_on_testing)
		system("./led.sh off");
	else
		system("./led.sh flash");
	
	while (1);
	
	return 0;
}
