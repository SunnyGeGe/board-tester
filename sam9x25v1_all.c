#include <getopt.h>
#include <sys/wait.h>

#include "common.h"

int err_on_testing = 0;

const static int err_flash_interval = 100000;
const static int pass_flash_interval = 1000000;

const static char menu[] = {
	"SAM9X25_V1 Test Program >>>>\n"
	" 1. 录音,放音\n"
	" 2. Key\n"
	" 3. CAN0\n"
	" 4. ETH0\n"
	" 5. ETH1\n"
	" 6. RS485\n"
	" 7. USB Host\n"
	" 8. USB Device\n"
	" 9. DataFalsh\n"
	"10. EEPROM\n"
};

static char result_list[64][64];
static int  result_idx = 0;
static int expect_usb_cnt = 2;

static pthread_t can0_pthread_id;
static pthread_t can1_pthread_id;
static pthread_t keytest_thread_id;
static pthread_t audiotest_thread_id;

static int codes_list[] = { 139, 158 };

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
			printf("testing can0...\n");

			ret = 0;
			if (pthread_create(&can0_pthread_id, NULL, (void*)local_can_test, "can0") != 0) {
				err_on_testing = 1;
				ret = -1;
			}

			sprintf(result_list[result_idx++],
					"3. %-15s %s", "create can0 thread", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
#if 0
		case 4:
			printf("testing can1...\n");
			if (pthread_create(&can0_pthread_id, NULL, local_can_test, "can1") != 0) {
				err_on_testing = 1;
				ret = -1;
			}

			sprintf(result_list[result_idx++],
					"4. %-15s %s", "create can1 thread", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
#endif
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
			
#if 0			
		case 7:
			printf("testing UART0...\n");

			if (ret = uart_test("/dev/ttyS1", 115200, 8, 'N', 1, 0, 100, 15, 50000 != 0))
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"5. %-15s %s", "UART0", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
#endif
		case 6:
			printf("testing RS485...\n");
			if (ret = uart_test("/dev/ttyS1", 115200, 8, 'N', 1, 1, 100, 15, 50000 != 0))
				err_on_testing = 1;
			
			sprintf(result_list[result_idx++],
					"6. %-15s %s", "RS485", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
			
		case 7:
			printf("testing USB HOST...\n");

			usb_cnt = get_usb_mp("/media", usb_mp);
			for (usb_idx = 0; usb_idx < usb_cnt; ++usb_idx) {
				if (ret = storage_test(usb_mp[usb_idx], "MYiR_test.txt") < 0)
					err_on_testing = 1;
				
				sprintf(result_list[result_idx++],
						"7. %-15s %s", usb_mp[usb_idx], ret == 0 ? "ok" : "err");
				if (!automatic)
					printf("\n%s\n", result_list[result_idx - 1]);
			}

			if (usb_cnt < expect_usb_cnt) {
				printf("\t*** Expect %d USBs but just found %d ***\n", expect_usb_cnt, usb_cnt);
				err_on_testing = 1;
				sprintf(result_list[result_idx++],
						"7. %-15s %s", "usb test", "err");
			}

			if (!automatic)
				break;

		case 8:
			printf("testing USB Device...\n");

			ret = system("./usb_install.sh");
			if ((ret = WEXITSTATUS(ret)) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"8. %-15s %s", "USB device", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

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

		case 10:
			printf("testing eeprom...\n");
			if ((ret = eeprom_test(0x51)) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"10. %-15s %s", "eeprom", ret == 0 ? "ok" : "err");
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
		"/sys/class/leds/d2/brightness",
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
	
    lighten_led(leds_path, 0); /* turn on led */

	if (automatic)
		do_test(1);
	else
		do_test(0);

	printf("Press K2 & K3...\n");
	pthread_join(keytest_thread_id, NULL);
	printf("Wait for can test complate\n");
	pthread_join(can0_pthread_id, NULL);

	print_result_list();

	if (err_on_testing)
		lighten_led(leds_path, 1); /* turn off led */
    else /* flash led if pass */
		if (create_ledtest_thread(leds_path,
							  1,
							  flash_interval,
							  0,
							  &ledtest_thread_id) < 0)
			printf("*** create led flash thread failed ***\n");


	
	while (1);
	
	return 0;
}
