#include <getopt.h>
#include <sys/wait.h>

#include "common.h"

#define ENABLE_ETH0_TEST
#define ENABLE_CAN0_TEST
#define ENABLE_USB_HOST_TEST
#define ENABLE_USB_DEVICE_TEST
#define ENABLE_DATAFLASH_TEST

int err_on_testing = 0;

const static int err_flash_interval = 100000;
const static int pass_flash_interval = 1000000;

const static char menu[] = {
	"SAM9X5 Test Program >>>>\n"
	" 1. Record,Playback\n"
	" 2. Key\n"
	" 3. CAN0\n"
	" 4. CAN1\n"
	" 5. ETH0\n"
	" 6. ETH1\n"
	" 6. RS485\n"
	" 7. USB Host\n"
	" 8. USB Device\n"
	" 9. DataFalsh\n"
	"10. EEPROM\n"
};

const char* its_list[] = {
    {"Audio Input/Output"},
    {"Key"},
    {"CAN0"},
    {"CAN1"},
    {"ETH0"},
    {"ETH1"},
    {"UART1"},
    {"RS485"},
    {"USB Host"},
    {"USB Device"},
    {"DataFalsh"},
    {"EEPROM"},
    {"RTC"},
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
	if (can_test("can0", NULL, 0x123, 100, 1000000, 15, 50000) != 0)
		err_on_testing |= (1 << 2);
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
			printf("Testing audio Input/Outpu...\n");
			
			if ((ret = create_audiotest_thread(&audiotest_thread_id)) != 0)
				err_on_testing |= (1 << 0);

			sprintf(result_list[result_idx++],
					"%-15s %s", "Create audio test thread", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
#ifdef ENABLE_KEY_TEST
		case 2:
			printf("Testing key...\n");
			
			if ((ret = create_keytest_thread("/dev/input/event0",
											 codes_list,
											 sizeof(codes_list) / 4,
											 &keytest_thread_id,
											 0)) < 0)
				err_on_testing |= (1 << 1);

			sprintf(result_list[result_idx++],
					"%-15s %s", "Create key test thread", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
#endif
#ifdef ENABLE_CAN0_TEST
		case 3:
			printf("Testing can0...\n");

			ret = 0;
			if (pthread_create(&can0_pthread_id, NULL, (void*)local_can_test, "can0") != 0) {
				err_on_testing |= (1 << 2);
				ret = -1;
			}

			sprintf(result_list[result_idx++],
					"%-15s %s", "create can0 thread", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
#endif
#ifdef ENABLE_CAN1_TEST
		case 4:
			printf("Testing can1...\n");
			if (pthread_create(&can0_pthread_id, NULL, local_can_test, "can1") != 0) {
				err_on_testing |= (1 << 3);
				ret = -1;
			}

			sprintf(result_list[result_idx++],
					"%-15s %s", "create can1 thread", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
#endif
#ifdef ENABLE_ETH0_TEST
		case 5:
			printf("Testing eth0...\n");
			if ((ret = eth_test("eth0", "192.168.1.113", "192.168.1.128")) != 0)
				err_on_testing |= (1 << 4);

			sprintf(result_list[result_idx++],
					"%-15s %s", "eth0", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
#endif
#ifdef ENABLE_ETH1_TEST
		case 6:
			printf("Testing eth1...\n");
			if (ret = eth_test("eth1", "192.168.2.223", "192.168.2.128") != 0)
				err_on_testing |= (1 << 5);

			sprintf(result_list[result_idx++],
					"%-15s %s", "eth1", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
#endif	
#ifdef ENABLE_UART0_TEST	
		case 7:
			printf("Testing UART1...\n");

			if (ret = uart_test("/dev/ttyS1", 115200, 8, 'N', 1, 0, 100, 15, 50000 != 0))
				err_on_testing |= (1 << 6);

			sprintf(result_list[result_idx++],
					"%-15s %s", "UART0", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
#endif
#ifdef ENABLE_RS485_TEST
		case 8:
			printf("Testing RS485...\n");
			if (ret = uart_test("/dev/ttyS1", 115200, 8, 'N', 1, 1, 100, 15, 50000 != 0))
				err_on_testing |= (1 << 7);
			
			sprintf(result_list[result_idx++],
					"%-15s %s", "RS485", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
#endif
#ifdef ENABLE_USB_HOST_TEST
		case 9:
			printf("Testing USB HOST...\n");

			usb_cnt = get_usb_mp(usb_mp);
			for (usb_idx = 0; usb_idx < usb_cnt; ++usb_idx) {
				if (ret = storage_test(usb_mp[usb_idx], "MYiR_test.txt") < 0)
					err_on_testing |= (1 << 8);
				
				sprintf(result_list[result_idx++],
						"%-15s %s", usb_mp[usb_idx], ret == 0 ? "ok" : "err");
				if (!automatic)
					printf("\n%s\n", result_list[result_idx - 1]);
			}

			if (usb_cnt < expect_usb_cnt && usb_cnt <= 0) {
				printf("\t*** Expect %d USBs but just found %d ***\n", expect_usb_cnt, usb_cnt);
				err_on_testing |= (1 << 9);
			    sprintf(result_list[result_idx++],
                    "%-15s %s", "USB HOST Test", "err");
			}

			if (!automatic)
				break;
#endif
#ifdef ENABLE_USB_DEVICE_TEST
		case 10:
			printf("Testing USB Device...\n");

			ret = system("./usb_install.sh");
			if ((ret = WEXITSTATUS(ret)) != 0)
				err_on_testing |= (1 << 10);

			sprintf(result_list[result_idx++],
					"%-15s %s", "USB Device Test", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
#endif
#ifdef ENABLE_DATAFLASH_TEST
		case 11:
			printf("Testing dataflash...\n");
			if ((ret = mtd_test("/dev/mtd2", "MYiR dataflash testing")) != 0)
				err_on_testing |= (1 << 11);

			sprintf(result_list[result_idx++],
					"%-15s %s", "dataflash", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}
#endif
		case 12:
			printf("Testing EEPROM...\n");
			if ((ret = eeprom_test(0x51)) != 0)
				err_on_testing |= (1 << 12);

			sprintf(result_list[result_idx++],
					"%-15s %s", "EEPROM", ret == 0 ? "ok" : "err");
			if (!automatic) {
				printf("\n%s\n", result_list[result_idx - 1]);
				break;
			}

        case 13:
            printf("Testing RTC...\n");
            if(ret = system("./rtc_test.sh") != 0)
                err_on_testing |= (1 << 13);
            sprintf(result_list[result_idx++],
                    "%-15s %s", "RTC", ret == 0?"ok":"err");
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
	
	printf("\n\n************* Testing result list *************\n");
		
	for (idx = 0; idx < result_idx; ++idx)
		printf("%s\n", result_list[idx]);
}

int main(int argc, char *argv[])
{
	char leds_path[][64] = {
		"/sys/class/leds/d1/brightness",
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
    int idx = 0;

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
	
    lighten_led(leds_path[1], 1); /* turn off red led */

	if (automatic)
		do_test(1);
	else
		do_test(0);
#if 0
	printf("Press K2 & K3...\n");
	pthread_join(keytest_thread_id, NULL);
#endif
#ifdef ENABLE_CAN0_TEST
	printf("Wait for CAN0 test complate\n");
	pthread_join(can0_pthread_id, NULL);
#endif
#ifdef ENABLE_CAN1_TEST
	printf("Wait for CAN1 test complate\n");
	pthread_join(can1_pthread_id, NULL);
#endif
	print_result_list();

	if (err_on_testing){
        for(idx = 0; idx < 13; idx++){
            if((err_on_testing >> idx) & 0x1)
	            printf("%-15s %s\n", its_list[idx], "err");
        }
        system("echo \"none\" > /sys/class/leds/d1/trigger");
		lighten_led(leds_path[0], 1); /* turn off blue led */

		lighten_led(leds_path[1], 0); /* turn on red led */
    }else{ 
        /* flash led if pass */
        system("echo \"none\" > /sys/class/leds/d1/trigger");
		lighten_led(leds_path[0], 0); /* turn on blue led */
		lighten_led(leds_path[1], 1); /* turn off red led */
        
#if 0
		if (create_ledtest_thread(leds_path,
							  1,
							  flash_interval,
							  0,
							  &ledtest_thread_id) < 0)
			printf("*** create led flash thread failed ***\n");

#endif
    }
	//while (1);
	
	return 0;
}
