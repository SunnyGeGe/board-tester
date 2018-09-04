#include <getopt.h>
#include <sys/wait.h>

#include "common.h"

int err_on_testing = 0;

const static int err_flash_interval = 100000;
const static int pass_flash_interval = 1000000;

const static char menu[] = {
	"a5d27 测试程序 >>>>\n"
	" 1. 按键\n"
	" 2. eth0\n"
	" 3. USB Host\n"
	" 4. USB Device\n"
	" 5. eeprom\n"
	" 6. rtc\n"
	" 7. spi flash\n"
	" 8. RS485\n"
	" 9. RS232\n"
	" 10. can\n"
	" 11. lcd\n"
	" 12. touchscreen\n"
	" 13. audio\n" 
};

static char result_list[64][64];
static int  result_idx = 0;
static int  expect_usb_cnt = 1;

static pthread_t can0_pthread_id;
static pthread_t can1_pthread_id;
static pthread_t keytest_thread_id;
static pthread_t audiotest_thread_id;

static int codes_list[] = { 260, 260 };


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
			printf("Create key test thread...\n");
			
			if ((ret = create_keytest_thread("/dev/input/event1",
											 codes_list,
											 sizeof(codes_list) / 4,
											 &keytest_thread_id,
											 0)) < 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"1. %-15s %s", "Create key test thread", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 2:
			printf("testing eth0...\n");
			if ((ret = eth_test("eth0", "192.168.1.112", "192.168.1.113")) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"2. %-15s %s", "eth0", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
                
			case 3:
                        printf("testing USB HOST...\n");

                        usb_cnt = get_usb_mp("/media", usb_mp);
                        for (usb_idx = 0; usb_idx < usb_cnt; ++usb_idx) {
                                if (ret = storage_test(usb_mp[usb_idx], "MYiR_test.txt") < 0)
                                        err_on_testing = 1;

                                sprintf(result_list[result_idx++],
                                                "3. %-15s %s", usb_mp[usb_idx], ret == 0 ? "ok" : "err");
                                if (!automatic)
                                        printf("\n%s\n", result_list[result_idx - 1]);
                        }

                        if (usb_cnt < expect_usb_cnt) {
                                printf("\t*** Expect %d USBs but just found %d ***\n", expect_usb_cnt, usb_cnt);
                                err_on_testing = 1;
                                sprintf(result_list[result_idx++],
                                                "3. %-15s %s", "usb host", "err");
                        }

                        if (!automatic)
                                break;

                case 4:
                        printf("testing USB Device...\n");

                        ret = system("/root/usb_install.sh");
                        if ((ret = WEXITSTATUS(ret)) != 0)
                                err_on_testing = 1;

                        sprintf(result_list[result_idx++],
                                        "4. %-15s %s", "USB device", ret == 0 ? "ok" : "err");
                        if (!automatic) {
                                printf("\n%s\n", result_list[result_idx - 1]);
                                break;
                        }


		case 5:
			printf("testing eeprom...\n");

			/* Disable write protect */
			//system("echo 103 > /sys/class/gpio/export");
			//system("echo out > /sys/class/gpio/gpio103/direction");
			
			if ((ret = eeprom_test("/dev/i2c-1",0x50)) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"5. %-15s %s", "eeprom", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 6:
			printf("testing rtc...\n");

			if ((ret = rtc_test("/dev/rtc0")) != 0)
				err_on_testing = 1;
			
			sprintf(result_list[result_idx++],
					"6. %-15s %s", "rtc", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

		case 7:
			printf("testing spi-flash...\n");

			if ((ret = mtd_test("/dev/mtd0", "MYiR SPI FLASH Test")) != 0)
				err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"7. %-15s %s", "spi-flash", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
		sleep(1);
		case 8:
			printf("testing RS485...\n");
			if (ret = uart_test("/dev/ttyS2", 115200, 8, 'N', 1, 1, 100, 15, 10000) != 0)
				err_on_testing = 1;
			
			sprintf(result_list[result_idx++],
					"8. %-15s %s", "RS485", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
		sleep(1);
                case 9:
                        printf("testing RS232...\n");
                        if (ret = uart_test("/dev/ttyS1", 115200, 8, 'N', 1, 0, 100, 15, 10000) != 0)
                                err_on_testing = 1;

                        sprintf(result_list[result_idx++],
                                        "9. %-15s %s", "RS232", ret == 0 ? "ok" : "err");
                        if (!automatic) {
                                printf("\n%s\n", result_list[result_idx - 1]);
                                break;
                        }
		
                case 10 : printf("testing can...\n");
                        printf("testing can0...\n");
                       if (ret = can_test("can0", NULL, 0x123, 100, 50000, 15, 1000) != 0)
                                err_on_testing = 1;
                        sprintf(result_list[result_idx++],
                                        "10. %-15s %s", "can0", ret==0 ? "ok" : "err");

                        if (!automatic) {
                                printf("\n%s\n", result_list[result_idx - 1]);
                                break;
                        }
                case 11 : printf("testing lcd...\n");
                        if (ret = lcd_drm_test()!=0)
                                err_on_testing = 1;
				sleep(1);
			if (ret = lcd_test()!=0)
				err_on_testing = 1;

                        sprintf(result_list[result_idx++],
                                        "11. %-15s %s", "lcd", ret == 0 ? "ok" : "err");
                        if (!automatic) {
                                printf("\n%s\n", result_list[result_idx - 1]);
                                break;
                        }
		case 12: printf("test touchscreen...\n");
			system("ts_calibrate");

		case 13 : printf("testing audio\n");
			

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
		"/sys/class/leds/D8/brightness",
		"/sys/class/leds/D9/brightness",
		"/sys/class/leds/blue/brightness",
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

	printf("Press K3...\n");
	pthread_join(keytest_thread_id, NULL);
	
	print_result_list();

	system("aplay -f dat /root/test.wav");

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


	
//	while (1);
	
	return 0;
}
