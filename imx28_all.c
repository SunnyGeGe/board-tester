#include <getopt.h>
#include <sys/wait.h>

#include "common.h"

int is_imx287 = 0;
int err_on_testing = 0;

const static int err_flash_interval = 100000;
const static int pass_flash_interval = 1000000;

const static char menu[] = {
	"IMX28 test program >>>>\n"
	" 1. audio record & play\n"
	" 2. keys\n"
	" 3. can0 & can1 (IMX287)\n"
	" 4. eth0\n"
	" 5. eth1 (IMX287)\n"
	" 6. UART0 (J13)\n"
	" 7. RS485\n"
	" 8. USB HOST & USB MINI\n"
#if 0
	" 9. spi falsh\n"
	"10. update system\n"
#endif
};

char leds_path[][64] = {
	"/sys/class/leds/d19/brightness",
};
	
static char result_list[64][64];
static int  result_idx = 0;
static int expect_usb_cnt = 1;

static pthread_t can_pthread_id = -1;
static pthread_t keytest_thread_id = -1;
static pthread_t audiotest_thread_id;
static pthread_t ledtest_thread_id;

static int codes_list[] = { 139, 158 };

static void local_can_test(void *arg)
{
	if (can_test("can0", "can1", 0x123, 100, 1000000, 15, 50000) != 0)
		err_on_testing = 1;
}

static void print_result_list(void)
{
	int idx;

	if (result_idx < 1)
		return;
	
	printf("\n\n************* failing list *************\n");
		
	for (idx = 0; idx < result_idx; ++idx)
		printf("%s\n", result_list[idx]);

	printf("\n\n");
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
			printf("1. Create audio test thread...\n");
			
			if ((ret = create_audiotest_thread(&audiotest_thread_id)) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"1. %-15s %s", "Create audio test thread", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 2:
			printf("2. Create key test thread...\n");
			
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
			if (!is_imx287)
				dbg_printf("3. imx283 没有 CAN 接口\n");
			else {
				dbg_printf("testing can0 & can1 ...\n");

				system("ifconfig can0 down; ifconfig can1 down");
				system("echo 1000000 > /sys/devices/platform/FlexCAN.0/bitrate");
				system("echo 1000000 > /sys/devices/platform/FlexCAN.1/bitrate");
				system("ifconfig can0 up; ifconfig can1 up");
				
				ret = 0;
				if (pthread_create(&can_pthread_id, NULL, (void*)local_can_test, NULL) != 0) {
					err_on_testing = 1;
					ret = -1;
				}

				sprintf(result_list[result_idx++],
						"3. %-15s %s", "create can0/can1 thread", ret == 0 ? "ok" : "err");
				if (!automatic) {
					printf("\n%s\n", result_list[result_idx - 1]);
					break;
				}
			}
			if (!automatic)
				break;

		case 4:
			printf("4. testing eth0...\n");
			system("ifconfig eth1 down");
			system("ifconfig eth0 192.168.1.128 up");
			sleep(1);
			system("ifconfig eth0 down");
			sleep(1);
			system("ifconfig eth0 up");
			sleep(1);
			system("ifconfig eth0 down");
			sleep(1);
			if ((ret = eth_test("eth0", "192.168.1.113", "192.168.1.128")) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"4. %-15s %s", "eth0", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 5:
			if (!is_imx287)
				dbg_printf("5. imx283 没有 eth1\n");
			else {
				printf("testing eth1...\n");
				system("ifconfig eth0 down");
				system("ifconfig eth1 192.168.2.128 up");
				sleep(1);
				system("ifconfig eth1 down");
				sleep(1);
				system("ifconfig eth1 up");
				sleep(1);
				system("ifconfig eth1 down");
				sleep(1);
				if (ret = eth_test("eth1", "192.168.2.223", "192.168.2.128") != 0)
					err_on_testing = 1;

				sprintf(result_list[result_idx++],
						"5. %-15s %s", "eth1", ret == 0 ? "ok" : "err");
				if (!automatic) {
					printf("\n%s\n", result_list[result_idx - 1]);
					break;
				}
			}
			if (!automatic)
				break;
			
		case 6:
			printf("testing UART0 (J13)...\n");

			if (ret = uart_test("/dev/ttySP0", 115200, 8, 'N', 1, 0, 100, 15, 50000 != 0))
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"6. %-15s %s", "UART0", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 7:
			printf("testing RS485...\n");
			if (ret = uart_test("/dev/ttySP1", 9600, 8, 'N', 1, 0, 100, 15, 50000 != 0))
				err_on_testing = 1;
			
			sprintf(result_list[result_idx++],
					"7. %-15s %s", "RS485", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
			
		case 8:
			printf("testing USB HOST & USB MINI...\n");

			system("./usb_install.sh");
			sleep(3); /* Sleep 3s for wait the usb ready */

			usb_cnt = get_usb_mp("/media/", usb_mp);
			for (usb_idx = 0; usb_idx < usb_cnt; ++usb_idx) {
				if (ret = storage_test(usb_mp[usb_idx], "MYiR_test.txt") < 0)
					err_on_testing = 1;
				
				sprintf(result_list[result_idx++],
						"8. %-15s %s", usb_mp[usb_idx], ret == 0 ? "ok" : "err");
				if (!automatic)
					printf("\n%s\n", result_list[result_idx - 1]);
			}

			if (usb_cnt < expect_usb_cnt) {
				printf("\t*** Expect %d USBs but just found %d ***\n", expect_usb_cnt, usb_cnt);
				err_on_testing = 1;
				sprintf(result_list[result_idx++],
						"8. %-15s %s", "usb test", "err");
			}

			system("./usb_install.sh uninstall");
			if (!automatic)
				break;
#if 0
		case 9:
			printf("testing spi flash...\n");
			if ((ret = mtd_test("/dev/mtd2", "MYiR dataflash testing")) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"9. %-15s %s", "spi flash", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 10:
			if (automatic)
				print_result_list();

			if (err_on_testing) {
				dbg_printf("测试失败，不烧写系统\n");
				break;
			}
			
			pthread_join(keytest_thread_id, NULL);
			pthread_join(can_pthread_id, NULL);
			
			if (err_on_testing) {
				dbg_printf("测试失败，不烧写系统\n");
				break;
			}

			printf("正在烧写系统...\n");

			if (create_ledtest_thread(leds_path,
									  1,
									  300000,
									  0,
									  &ledtest_thread_id) < 0)
				printf("*** Create led_test thread failed ***\n");

			ret = system("./update_sys.sh");
			if ((ret = WEXITSTATUS(ret)) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"10. %-15s %s", "update system", ret == 0 ? "ok" : "err");

			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
#endif		
		default:
			break;
		}

		if (automatic)
			return;
	}
}

int main(int argc, char *argv[])
{
	int flash_interval = pass_flash_interval;

	int automatic = 0;
	int next_option;
	const char *short_options = "ap:";
	const struct option options[] = {
		{ "auto", 0, NULL, 'a' },
		{ "platform", 1, NULL, 'p' },
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

		case 'p':
			if (strcmp(optarg, "imx287") == 0)
				is_imx287 = 1;
			break;
			
		default:
			break;
		}
	}

	/* start signal */
	system("echo 88 > /sys/class/mxs_buzzer/volume");
	usleep(500000);
	system("echo 0 > /sys/class/mxs_buzzer/volume");
    lighten_led(leds_path, 1); /* turn on led */

	if (automatic)
		do_test(1);
	else
		do_test(0);

	if (keytest_thread_id != -1)
		pthread_join(keytest_thread_id, NULL);
	if (can_pthread_id != -1)
		pthread_join(can_pthread_id, NULL);

	if (automatic)
		print_result_list();
	
	if (err_on_testing) {
		lighten_led(leds_path, 0); /* turn off led */
		system("echo 88 > /sys/class/mxs_buzzer/volume"); /* buzzer */
    } else {/* flash led if pass */
		while (1) {
			system("echo 88 > /sys/class/mxs_buzzer/volume"); /* buzzer */
			usleep(80000);
			system("echo 0 > /sys/class/mxs_buzzer/volume"); /* buzzer */
			sleep(1);
		}
	}
	
	while (1);
	
	return 0;
}
