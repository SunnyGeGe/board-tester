#include <getopt.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

#include "common.h"
#define WIFI_ONLY 1

int err_on_testing = 0;
int err_on_wifi;
time_t end_time;
const static int err_flash_interval = 100000;
const static int pass_flash_interval = 1000000;

const static char menu[] = {
    "ODM-IMEON Test Program >>>>\n"
    " 1. TTL(J29.7 <==> J29.8)\n"
	" 2. RS232\n"
    " 3. RS485\n"
	" 4. CAN\n"
//	" 4. gpio\n"   run in main
//	" 5. relay\n"
	" 5. LCD\n"
	" 6. ETH\n"
	" 7. ttyUSB0\n"
	" 8. ttyUSB1\n" 
	" 9. miniUSB_adc\n"
	" 10. audio_adc\n"
	" 11. WiFi\n"
};

static char result_list[64][64];
static int  result_idx = 0;
static int expect_usb_cnt = 2;
//int pr=0;
static pthread_t can0_pthread_id;
static pthread_t can1_pthread_id;
static pthread_t keytest_thread_id;
static pthread_t uart_thread_id1;
static pthread_t uart_thread_id2;
static pthread_t rs485_thread_id1;
static pthread_t rs485_thread_id2;
static pthread_t usb_thread_id1;
static pthread_t usb_thread_id2;
static pthread_t wifi_thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int codes_list[] = { 139, 158 };

static void wifi_setup_thread(void *arg)
{
    printf("WiFi setup thread running\n");
	err_on_wifi = 0;
	wifi_apsta_enable("/dev/ttyO5",57600,8,'N',1);
}

static void wifi_restore_thread(void *arg)
{
    printf("WiFi restore thread running\n");
	wifi_restore("/dev/ttyO5",57600,8,'N',1);
}

int relay_test(const char* relay_path,int num)
{
	char cmdline[64];
	sprintf(cmdline,"echo %d > %s", num,relay_path);
	return system(cmdline);
}
void gpio_set_out()
{
        gpio_export(22,"out");
        gpio_export(23,"out");
        gpio_export(26,"out");
        gpio_export(27,"out");
}
void gpio_relay_test()
{
	relay_test("/sys/class/gpio/gpio116/value",1);
	gpio_set_value(22,'1');
	gpio_set_value(26,'1');
	gpio_set_value(27,'1');
	usleep(200000);
	gpio_set_value(22,'0');
	gpio_set_value(26,'0');
	gpio_set_value(27,'0');
	relay_test("/sys/class/gpio/gpio116/value",0);
	usleep(200000);
}

static void do_test(int automatic)
{
        int sel = 1;
	char usb_mp[5][64];
	int usb_cnt = 0;
	int usb_idx = 0;
        int ret;
	int count = 200;
        sleep(1);

        while(1)
        {
                if(!automatic)
                {
                        printf("\n\n%s",menu);
                        printf("choice> ");
                        scanf("%d",&sel);
                }

#ifndef WIFI_ONLY
                switch(sel)
                {
                        case 1:printf("testing TTL(J29.7 and J29.8)...\n");

			if (ret = uart_test("/dev/ttyO1", 115200, 8, 'N', 1, 0, 100, 15, 50000 != 0))
                                err_on_testing = 1;

                        sprintf(result_list[result_idx++],
                                        "1. %-15s %s", "uart-ttl(J29.7 and J29.8)", ret == 0 ? "ok" : "err");
                        if (!automatic) {
                                printf("\n%s\n", result_list[result_idx - 1]);
                                break;
                        }
			
			case 2 : printf("testing rs232...\n");
                        if (ret = uart_test("/dev/ttyO2", 115200, 8, 'N', 1, 0, 100, 15, 50000 != 0))
                                err_on_testing = 1;

                        sprintf(result_list[result_idx++],
                                        "2. %-15s %s", "rs232", ret == 0 ? "ok" : "err");
                        if (!automatic) {
                                printf("\n%s\n", result_list[result_idx - 1]);
                                break;
                        }

                        case 3:printf("testing rs485...\n");

                        if(ret=(pthread_create(&rs485_thread_id1,NULL,(void *)creat_rs485_3_thread,NULL))!=0)
                              {  err_on_testing = 1;ret=-1;}
                        if(ret=(pthread_create(&rs485_thread_id2,NULL,(void *)creat_rs485_4_thread,NULL))!=0)
                              {  err_on_testing = 1;ret=-1;}
				
                       pthread_join(rs485_thread_id1,NULL);
                       pthread_join(rs485_thread_id2,NULL);
			
                        sprintf(result_list[result_idx++],
                                        "3. %-15s %s", "RS485", (ret==0&&pr==0) ? "ok" : "err");
                        	if (!automatic) {
                                	printf("\n%s\n", result_list[result_idx - 1]);
                                	break;
                        	}
			case 4 : printf("testing can...\n");
                        printf("testing can0...\n");
//                       if (ret = can_receive("can0",1000000,5) != 0)
  //                              err_on_testing = 1;
                       if (ret = can_test("can0", NULL, 0x123, 100, 50000, 15, 1000) != 0)
                                err_on_testing = 1;
                        sprintf(result_list[result_idx++],
                                        "4. %-15s %s", "can0", ret==0 ? "ok" : "err");

                        if (!automatic) {
                                printf("\n%s\n", result_list[result_idx - 1]);
                                break;
                        } 
                        case 5 : printf("testing lcd...\n");
			gpio_set_value(23,'1');
                        if (ret = lcd_test()!=0)
                                err_on_testing = 1;

                        sprintf(result_list[result_idx++],
                                        "5. %-15s %s", "lcd", ret == 0 ? "ok" : "err");
                        if (!automatic) {
                                printf("\n%s\n", result_list[result_idx - 1]);
                                break;
                        }
		
                        case 6 : printf("testing eth0...\n");
                            if ((ret = eth_test("eth0", "192.168.1.12", "192.168.1.11")) != 0)
                                    err_on_testing = 1;

                        sprintf(result_list[result_idx++],
                                        "6. %-15s %s", "eth0", ret == 0 ? "ok" : "err");
                                if (!automatic) {
                                        printf("\n%s\n", result_list[result_idx - 1]);
                                        break;
                                }

			sleep(1);
                        case 7 : printf("testing ttyUSB0...\n");
/*
                        if((ret = pthread_create(&usb_thread_id1,NULL,(void *)creat_usb_0_thread,NULL))!=0)
                                err_on_testing = 1;
                        if((ret = pthread_create(&usb_thread_id2,NULL,(void *)creat_usb_1_thread,NULL))!=0)
                                err_on_testing = 1;

                        pthread_join(usb_thread_id1,NULL);
                        pthread_join(usb_thread_id2,NULL);
*/
                        if (ret = uart_test("/dev/ttyUSB0", 115200, 8, 'N', 1, 0, 100, 15, 50000 != 0))
                                err_on_testing = 1;

                        sprintf(result_list[result_idx++],
                                        "7. %-15s %s", "ttyUSB0", (ret == 0&&pr==0) ? "ok" : "err");
                                if (!automatic) {
                                        printf("\n%s\n", result_list[result_idx - 1]);
                                        break;
                                }
			case 8 : printf("testing ttyUSB1...\n");
                        if (ret = uart_test("/dev/ttyUSB1", 115200, 8, 'N', 1, 0, 100, 15, 50000 != 0))
                                err_on_testing = 1;

                        sprintf(result_list[result_idx++],
                                        "8. %-15s %s", "ttyUSB1", (ret == 0&&pr==0) ? "ok" : "err");
                                if (!automatic) {
                                        printf("\n%s\n", result_list[result_idx - 1]);
                                        break;
                                }

                        case 9:printf("testing usb_adc...\n");
                               if((ret=adc_test("/sys/bus/iio/devices/iio:device0/in_voltage2_raw","/sys/bus/iio/devices/iio:device0/in_voltage3_raw"))!=0)
                               err_on_testing = 1;

                               sprintf(result_list[result_idx++],"9. %-15s %s","usb_adc_test",ret == 0?"ok":"err");
                               if(!automatic)
                                                           {
                                               printf("\n%s\n",result_list[result_idx - 1]);
                                                                   break;
                                           }
                        case 10:printf("testing audio_adc...\n");
                               if((ret=adc_test("/sys/bus/iio/devices/iio:device0/in_voltage0_raw","/sys/bus/iio/devices/iio:device0/in_voltage1_raw"))!=0)
                               err_on_testing = 1;

                               sprintf(result_list[result_idx++],"10. %-15s %s","audio_adc_test",ret == 0?"ok":"err");
                               if(!automatic)
                                                           {
                                               printf("\n%s\n",result_list[result_idx - 1]);
                                                                   break;
                                           }

                        case 11 : printf("testing wifi...\n");
                        if(pthread_join(wifi_thread, NULL) == 0)
                            printf("WiFi setup success error_on_wifi=%d\n",err_on_wifi);
							if(err_on_wifi > 20){
								err_on_testing = 11;
							}
							ret = system("udhcpc -i eth1");
							if(ret = WEXITSTATUS(ret) != 0){
								 printf("dhcp on eth1 error!\r\n");
								 err_on_testing = 11;
							}
						
			    system("ifconfig eth0 down");
			    system("ifconfig can0 down");
                            if ((ret = eth_test("eth1", "192.168.30.1", "")) != 0){
                                    err_on_testing = 11;
                            	}
                            if ((ret = eth_test("eth1", "192.168.3.2", "")) != 0){
                                    err_on_testing = 11;
                            	}

							

                        sprintf(result_list[result_idx++],
                                        "11. %-15s %s", "wifi", err_on_testing  < 11 ? "ok" : "err");
                                if (!automatic) {
                                        printf("\n%s\n", result_list[result_idx - 1]);
                                        break;
                                }

			 if (!automatic)
                                break;

		} // end switch(sel)
#else
                        printf("testing wifi...\n");
                        if(pthread_join(wifi_thread, NULL) == 0)
                            printf("WiFi setup success error_on_wifi=%d\n",err_on_wifi);
							if(err_on_wifi > 20){
								err_on_testing = 11;
							}
							ret = system("udhcpc -i eth1");
							if(ret = WEXITSTATUS(ret) != 0){
								 printf("dhcp on eth1 error!\r\n");
								 err_on_testing = 11;
							}
						
			    system("ifconfig eth0 down");
			    system("ifconfig can0 down");
                            if ((ret = eth_test("eth1", "192.168.30.1", "")) != 0){
                                    err_on_testing = 11;
                            	}
                            if ((ret = eth_test("eth1", "192.168.3.2", "")) != 0){
                                    err_on_testing = 11;
                            	}
                        	sprintf(result_list[result_idx++],
                                        "11. %-15s %s", "wifi", err_on_testing  < 11 ? "ok" : "err");


#endif
                if (automatic)
                        return;
	} // end while(1)
}

static void print_result_list(void)
{
        int idx;

        if (result_idx < 1)
                return;

        printf("\n\n*************err_on_testing = %d  *************\n",err_on_testing);
        printf("\n\n*************err_on_wifi = %d  *************\n",err_on_wifi);
        printf("\n\n************* failing list *************\n");

        for (idx = 0; idx < result_idx; ++idx)
                printf("%s\n", result_list[idx]);
}


int main(int argc, char *argv[])
{
    pthread_t ledtest_thread_id;
    printf("start testing...\n");
    int flash_interval = pass_flash_interval;
    int count=10;
    int automatic = 0;
    int next_option, ret;
    const char *short_options = "a";
    const struct option options[] = {
        { "auto", 0, NULL, 'a' },
        { NULL, 0, NULL, 0}
    };
    
    if((ret = pthread_create(&wifi_thread, NULL, (void *)wifi_setup_thread, NULL))!=0)
        printf("WiFi setup thread failed\n");

	//wifi_ap_enable("/dev/ttyO5",57600,8,'N',1);
	sleep(2);	
	gpio_set_out();
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
	{
#ifndef WIFI_ONLY
		do{
		gpio_relay_test();
		}while(count--);
#else
		sleep(5);
#endif
                do_test(1);
	}
        else{
                do_test(0);
		}
        print_result_list();
//		
//		if((ret = pthread_create(&wifi_thread, NULL, (void *)wifi_restore_thread, NULL))!=0){
//			printf("WiFi restore thread failed\n");
//			}
//		if(pthread_join(wifi_thread, NULL) == 0){
//			printf("WiFi restore success\n");
//			}

        return 0;
}

