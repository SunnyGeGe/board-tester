#include <getopt.h>
#include <sys/wait.h>

#include "common.h"

int err_on_testing = 0;

const static int err_flash_interval = 100000;
const static int pass_flash_interval = 1000000;

const static char menu[] = {
	"MYD-C437X 测试程序 >>>>\n"
	" 1. 录音,放音\n"
	" 2. 按键\n"
	" 3. can0 & can0\n"
	" 4. eth0\n"
	" 5. eth1\n"
	" 6. RS485\n"
	" 7. USB Host & Device\n"
	" 8. QSPIFalsh (保留)\n"
	" 9. eeprom\n"
	"10. camera\n"
	"11. rtc\n"
	"12. UART3\n"
};

static char result_list[64][64];
static int  result_idx = 0;
static int expect_usb_cnt = 4;

static pthread_t can0_pthread_id;
static pthread_t can1_pthread_id;
static pthread_t keytest_thread_id;
static pthread_t audiotest_thread_id;

static int codes_list[] = { 102, 158 };

static void local_can_test(void *arg)
{
	if (can_test("can0", "can1", 0x123, 100, 1000000, 15, 50000) != 0)
		err_on_testing = 1;
}

static void do_test(int automatic)
{
	int sel = 1;
	char usb_mp[5][64];
	int usb_cnt = 0;
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
			printf("Create audio test thread...\n");
			
			if ((ret = create_audiotest_thread(&audiotest_thread_id)) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"1. %-15s %s", "Create audio test thread", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 2:
			printf("Create key test thread...\n");
			
			if ((ret = create_keytest_thread("/dev/input/event0",
											 codes_list,
											 sizeof(codes_list) / 4,
											 &keytest_thread_id,
											 0)) < 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"2. %-15s %s", "Create key test thread", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 3:
			printf("testing can0 & can1 ...\n");

			ret = 0;

			if ((ret = can_test("can0", "can1", 0x123, 100, 1000000, 15, 50000)) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"3. %-15s %s", "testing can0 & can1 ", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 4:
			printf("testing eth0...\n");
			if ((ret = eth_test("eth0", "192.168.1.113", "192.168.1.128")) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"5. %-15s %s", "eth0", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 5:
			printf("testing eth1...\n");
			if (ret = eth_test("eth1", "192.168.2.223", "192.168.2.128") != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"6. %-15s %s", "eth1", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
			
		case 6:
			printf("testing RS485...\n");
			if (ret = uart_test("/dev/ttyO5", 115200, 8, 'N', 1, 1, 100, 15, 50000) != 0)
				err_on_testing = 1;
			
			sprintf(result_list[result_idx++],
					"6. %-15s %s", "RS485", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
			
		case 7:
			printf("testing USB HOST & device...\n");

			ret = system("./usb_install.sh");
			if ((ret = WEXITSTATUS(ret)) != 0)
				err_on_testing = 1;

			sleep(3);

			usb_cnt = get_usb_mp("/var/run/media/", usb_mp);
			for (usb_idx = 0; usb_idx < usb_cnt; ++usb_idx) {
				if (ret = storage_test(usb_mp[usb_idx], "MYiR_test.txt") < 0)
					err_on_testing = 1;

				dbg_printf("%s %s\n", usb_mp[usb_idx], ret == 0 ? "ok" : "err");
			}

			if (usb_cnt < expect_usb_cnt) {
				printf("\t*** Expect %d USBs but just found %d ***\n", expect_usb_cnt, usb_cnt);
				err_on_testing = 1;
				sprintf(result_list[result_idx++],
						"7. %-15s %s", "usb test", "err");
			}

			if (!automatic)
				break;
#if 0
		case 9:
			printf("testing dataflash...\n");
			if ((ret = mtd_test("/dev/mtd2", "MYiR dataflash testing")) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"9. %-15s %s", "dataflash", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
#endif
		case 9:
			printf("testing eeprom...\n");
			if ((ret = eeprom_test(0x50)) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"9. %-15s %s", "eeprom", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 10:
			printf("start camera test program...\n");

			ret = system("dual_camera&");

		case 11:
			printf("testing rtc...\n");

			if ((ret = rtc_test("/dev/rtc1")) != 0)
				err_on_testing = 1;
			
			sprintf(result_list[result_idx++],
					"11. %-15s %s", "rtc", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 12:
			printf("testing UART3...\n");

			if (ret = uart_test("/dev/ttyO3", 115200, 8, 'N', 1, 0, 100, 15, 50000) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"12. %-15s %s", "UART3", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

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
	
	printf("\n\n************* failing list *************\n");
		
	for (idx = 0; idx < result_idx; ++idx)
		printf("%s\n", result_list[idx]);
}

int main(int argc, char *argv[])
{
	char leds_path[][64] = {
		"/sys/class/leds/myd_d34/brightness",
		"/sys/class/leds/myd_d35/brightness",
		"/sys/class/leds/myd_d36/brightness",
	};
	
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
	
    lighten_led(leds_path[0], 1); /* turn on led */
	lighten_led(leds_path[1], 1); /* turn on led */
	lighten_led(leds_path[2], 1); /* turn on led */

	if (automatic)
		do_test(1);
	else
		do_test(0);

	printf("Press K2 & K3...\n");
	pthread_join(keytest_thread_id, NULL);

	print_result_list();

	if (err_on_testing) {
		lighten_led(leds_path[0], 0); /* turn off led */
		lighten_led(leds_path[1], 0); /* turn off led */
		lighten_led(leds_path[2], 0); /* turn off led */
    } else /* flash led if pass */
		if (create_ledtest_thread(leds_path,
							  3,
							  flash_interval,
							  0,
							  &ledtest_thread_id) < 0)
			printf("*** create led flash thread failed ***\n");


	
	while (1);
	
	return 0;
}
