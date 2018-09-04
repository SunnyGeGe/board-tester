#include <getopt.h>

#include "common.h"

int err_on_testing = 0;

const static int err_flash_interval = 100000;
const static int pass_flash_interval = 1000000;

const static char menu[] = {
	"AM335X TEST PROGRAM >>>>\n"
	" 1. eth0\n"
	" 2. eth1\n"
	" 3. can0\n"
	" 4. RS485\n"
	" 5. UART2\n"
	" 6. storage\n"
	" 7. automatic\n"
	" 8. exit\n"
};

static char failed_list[64][64];
static int  failed_idx = 0;

static void do_test(int automatic)
{
	int sel = 1;
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
			printf("testing eth0...\n");
			if ((ret = eth_test("eth0", "192.168.1.113", "192.168.1.100")) != 0)
				err_on_testing = 1;
			
			if (!automatic) {
				printf("\n1. %-15s %s\n", "eth0", ret == 0 ? "ok" : "err");
				break;
			} else {
				sprintf(failed_list[failed_idx++],
						"1. %-15s %s", "eth0", ret == 0 ? "ok" : "err");
			}

		case 2:
			printf("testing eth1...\n");
			if (ret = eth_test("eth1", "192.168.2.223", "192.168.2.100") != 0)
				err_on_testing = 1;

			if (!automatic) {
				printf("\n2. %-15s %s\n", "eth1", ret == 0 ? "ok" : "err");
				break;
			} else {
				sprintf(failed_list[failed_idx++],
						"2. %-15s %s", "eth1", ret == 0 ? "ok" : "err");
			}
			
		case 3:
			printf("testing can0...\n");
			if (ret = can_test("can0", NULL, 0x123, 100, 1000000, 15, 50000) != 0)
				err_on_testing = 1;

			if (!automatic) {
				printf("\n3. %-15s %s\n", "can0", ret == 0 ? "ok" : "err");
				break;
			} else {
				sprintf(failed_list[failed_idx++],
						"3. %-15s %s", "can0", ret == 0 ? "ok" : "err");
			}

		case 4:
			printf("testing ttyO1...\n");
			if (ret = uart_test("/dev/ttyO1", 115200, 8, 'N', 1, 0, 100, 15, 50000) != 0)
				err_on_testing = 1;
			
			if (!automatic) {
				printf("\n4. %-15s %s\n", "ttyO1", ret == 0 ? "ok" : "err");
				break;
			} else {
				sprintf(failed_list[failed_idx++],
						"4. %-15s %s", "ttyO1", ret == 0 ? "ok" : "err");
			}

		case 5:
			printf("testing ttyO2...\n");
			if (ret = uart_test("/dev/ttyO2", 115200, 8, 'N', 1, 0, 100, 15, 50000) != 0)
				err_on_testing = 1;
			
			if (!automatic) {
				printf("\n5. %-15s %s\n", "ttyO2", ret == 0 ? "ok" : "err");
				break;
			} else {
				sprintf(failed_list[failed_idx++],
						"5. %-15s %s", "ttyO2", ret == 0 ? "ok" : "err");
			}

		case 6:
			printf("testing storage...\n");
			if (ret = storage_test("/media/sda1", "MYiR_test.txt") < 0)
				err_on_testing = 1;
			if (!automatic)
				printf("\n6. %-15s %s\n", "sda1", ret == 0 ? "ok" : "err");
			else
				sprintf(failed_list[failed_idx++],
						"6. %-15s %s", "sda1", ret == 0 ? "ok" : "err");

			if (ret = storage_test("/media/sdb1", "MYiR_test.txt") < 0)
				err_on_testing = 1;
			if (!automatic)
				printf(" . %-15s %s\n", "sdb1", ret == 0 ? "ok" : "err");
			else 
				sprintf(failed_list[failed_idx++],
						" . %-15s %s", "sdb1", ret == 0 ? "ok" : "err");

			if (ret = storage_test("/media/sdc1", "MYiR_test.txt") < 0)
				err_on_testing = 1;
			if (!automatic)
				printf(" . %-15s %s\n", "sdc1", ret == 0 ? "ok" : "err");
			else
				sprintf(failed_list[failed_idx++],
						" . %-15s %s", "sdc1", ret == 0 ? "ok" : "err");

			if (ret = storage_test("/media/sdd1", "MYiR_test.txt") < 0)
				err_on_testing = 1;
			if (!automatic)
				printf(" . %-15s %s\n", "sdd1", ret == 0 ? "ok" : "err");
			else
				sprintf(failed_list[failed_idx++],
						" . %-15s %s", "sdd1", ret == 0 ? "ok" : "err");

			if (ret = storage_test("/media/sde1", "MYiR_test.txt") < 0)
				err_on_testing = 1;
			if (!automatic)
				printf(" . %-15s %s\n", "sde1", ret == 0 ? "ok" : "err");
			else
				sprintf(failed_list[failed_idx++],
						" . %-15s %s", "sde1", ret == 0 ? "ok" : "err");

			if (ret = storage_test("/media/mmcblk0p1", "MYiR_test.txt") < 0)
				err_on_testing = 1;
			if (!automatic) {
				printf(" . %-15s %s\n", "mmcblk0p1", ret == 0 ? "ok" : "err");
				break;
			} else {
				sprintf(failed_list[failed_idx++],
						" . %-15s %s", "mmcblk0p1", ret == 0 ? "ok" : "err");
			}

		case 7:
			if (automatic)
				return;
			automatic = 1;
			sel = 1;
			continue;
		
		case 8:
			return;
			
		default:
			break;
		}

		if (automatic)
			return;
	}
}

static void print_failed_list(void)
{
	int idx;

	if (failed_idx < 1)
		return;
	
	printf("\n\n************* failing list *************\n");
		
	for (idx = 0; idx < failed_idx; ++idx)
		printf("%s\n", failed_list[idx]);
}

int main(int argc, char *argv[])
{
	int codes_list[] = { 1, 102 };
	char leds_path[][64] = {
		"/sys/class/leds/user_led0/brightness",
		"/sys/class/leds/user_led1/brightness"
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
	
	if (create_keytest_thread("/dev/input/event0",
							  codes_list,
							  sizeof(codes_list) / 4,
							  &keytest_thread_id, 1) < 0) {
		printf("*** keys test (/dev/input/event0) failed ***\n");
		return -1;
	}

	if (create_audiotest_thread(&audiotest_thread_id) < 0) {
		printf("*** audio test (recoard & play) failed ***\n");
		return -1;
	}

	
	if (automatic)
		do_test(1);
	else
		do_test(0);
	

	printf("Press K2 & K3...\n");
	pthread_join(keytest_thread_id, NULL);

	if (err_on_testing)
		flash_interval = err_flash_interval;

	if (create_ledtest_thread(leds_path,
							  2,
							  flash_interval,
							  0,
							  &ledtest_thread_id) < 0) {
		printf("*** led test failed ***\n");
		return -1;
	}

	print_failed_list();
	
	while (1);
	
	return 0;
}
