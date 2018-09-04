#include <getopt.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common.h"

int err_on_testing = 0;
time_t end_time;
const static int err_flash_interval = 100000;
const static int pass_flash_interval = 1000000;
const static char menu[] = {
	"TS-ACU3000 Test Program >>>>\n"
	" 1. UART\n"
	" 2. pwm_test\n"
	" 3. spi_led\n"
	" 4. adc_test\n"
	" 5. motor_test\n"
	" 6. stepper_test\n"
	" 7. eth0\n"
	" 8. eth1\n"
	" 9. usb\n"
	" 10. gpio\n"
	" 11. key\n"
	" 12. rs485\n"
};

static char result_list[64][64];
static int  result_idx = 0;
static int expect_usb_cnt = 2;
static pthread_t can0_pthread_id;
static pthread_t can1_pthread_id;
static pthread_t keytest_thread_id;
static pthread_t audiotest_thread_id;
static pthread_t uart_thread_id1;
static pthread_t uart_thread_id2;
static pthread_t uart_thread_id3;
static pthread_t rs485_thread_id1;
static pthread_t rs485_thread_id2;
static pthread_t rs485_thread_id3;
static pthread_t rs485_thread_id4;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int gp1=0,gp2=0,gp3=0,gp4=0,gp5=0,gp6=0;
static int codes_list[] = { 139, 158 };

static void local_can_test(void *arg)
{
	if (can_test("can0", "can1", 0x123, 100, 1000000, 15, 50000) != 0)
		err_on_testing = 1;
}

int gpio()
{
	int fd1=0,fd2=0,fd3=0,fd4=0,fd5=0,fd6=0;
	char buf1[1],buf2[1],buf3[1],buf4[1],buf5[1],buf6[1];
	int x=0;	
	
	while(1)
	{
        fd1 = open("/sys/class/gpio/gpio61/value",O_RDWR);
        fd2 = open("/sys/class/gpio/gpio60/value",O_RDWR);
        fd3 = open("/sys/class/gpio/gpio59/value",O_RDWR);
        fd4 = open("/sys/class/gpio/gpio58/value",O_RDWR);
        fd5 = open("/sys/class/gpio/gpio146/value",O_RDWR);
        fd6 = open("/sys/class/gpio/gpio145/value",O_RDWR);
        read(fd1,buf1,1);
        read(fd2,buf2,1);
        read(fd3,buf3,1);
        read(fd4,buf4,1);
        read(fd5,buf5,1);
        read(fd6,buf6,1);

			sleep(1);
			x++;
			if(x==15)
			{
	                printf("IO1 READ : %c\n",buf1[0]);
        	        printf("IO2 READ : %c\n",buf2[0]);
              	 	printf("IO3 READ : %c\n",buf3[0]);
           	    	printf("IO4 READ : %c\n",buf4[0]);
         	        printf("IO5 READ : %c\n",buf5[0]);
       		        printf("IO6 READ : %c\n",buf6[0]);
				gp1=1;
				break;
			}
                if((buf1[0]=='1')&&(buf2[0]=='1')&&(buf3[0]=='1')&&(buf4[0]=='1')&&(buf5[0]=='1')&&(buf6[0]=='1'))
                {
                printf("IO1 READ : %c\n",buf1[0]);
                printf("IO2 READ : %c\n",buf2[0]);
                printf("IO3 READ : %c\n",buf3[0]);
                printf("IO4 READ : %c\n",buf4[0]);
                printf("IO5 READ : %c\n",buf5[0]);
                printf("IO6 READ : %c\n",buf6[0]);
                gp1=0;
                break;
               
                }
		
		}

	close(fd1);
	close(fd2);
	close(fd3);
	close(fd4);
	close(fd5);
	close(fd6);
	return gp1;
	
}


static void do_test(int automatic)
{
	int sel = 1;

	int ret;
	int ret1,ret2,ret3,ret4,ret5;
	sleep(1);

	while(1)
	{
		if(!automatic)
		{
			printf("\n\n%s",menu);
			printf("choice> ");
			scanf("%d",&sel);
		}

		switch(sel)
		{
                        case 1:printf("1. testing uart...\n\n");
                        if((ret = pthread_create(&uart_thread_id1,NULL,(void *)creat_uart2_thread,NULL))!=0)
                                err_on_testing = 1;
                        sleep(2);
                        if((ret = pthread_create(&uart_thread_id2,NULL,(void *)creat_uart1_thread,NULL))!=0)
                                err_on_testing = 1;

                        pthread_join(uart_thread_id1,NULL);
                        pthread_join(uart_thread_id2,NULL);
                        sleep(1);
                        if (ret1 = uart_test("/dev/ttyS3", 115200, 8, 'N', 1, 0, 100, 15, 50000 != 0))
                                err_on_testing = 1;
                        sleep(1);
                        if (ret2 = uart_test("/dev/spi_usart2", 115200, 8, 'N', 1, 0, 100, 15, 50000 != 0))
                                err_on_testing = 1;
                        sleep(1);
                        if (ret3 = uart_test("/dev/spi_usart3", 115200, 8, 'N', 1, 0, 100, 15, 50000 != 0))
                                err_on_testing = 1;
                        sleep(1);
                        if (ret4 = uart_test("/dev/spi_usart4", 115200, 8, 'N', 1, 0, 100, 15, 50000 != 0))
                                err_on_testing = 1;
                        sleep(1);
                        if (ret5 = uart_test("/dev/ttyS5", 115200, 8, 'N', 1, 0, 100, 15, 50000 != 0))
                                err_on_testing = 1;
                        sprintf(result_list[result_idx++],
                                        "1. %-15s %s", "UART", (ret==0&&pr==0&&ret1==0&&ret2==0&&ret3==0&&ret4==0&&ret5==0) ? "ok" : "err");
                                if (!automatic) {
                                        printf("\n%s\n", result_list[result_idx - 1]);
                                        break;
                                }
/*			sleep(5);
                        case 2:printf("testing rs485...\n");

                        if((ret = pthread_create(&rs485_thread_id1,NULL,(void *)creat_rs485_1_thread,NULL))!=0)
                                err_on_testing = 1;
                        sleep(2);
                        if((ret = pthread_create(&rs485_thread_id2,NULL,(void *)creat_rs485_2_thread,NULL))!=0)
                                err_on_testing = 1;
			sleep(2);
                        pthread_join(rs485_thread_id1,NULL);
                        pthread_join(rs485_thread_id2,NULL);
                        sprintf(result_list[result_idx++],
                                        "2. %-15s %s", "RS485", (ret==0&&pr==0) ? "ok" : "err");
                        if (!automatic) {
                                printf("\n%s\n", result_list[result_idx - 1]);
                                break;
                        }
                        sleep(3);
                        case 3:printf("testing rs4852...\n");
                        if((ret = pthread_create(&rs485_thread_id1,NULL,(void *)creat_rs485_5_thread,NULL))!=0)
                                err_on_testing = 1;
                        sleep(2);
                        if((ret = pthread_create(&rs485_thread_id2,NULL,(void *)creat_rs485_2_thread,NULL))!=0)
                                err_on_testing = 1;
                        sleep(2);
                        pthread_join(rs485_thread_id1,NULL);
                        pthread_join(rs485_thread_id2,NULL);
                        sprintf(result_list[result_idx++],
                                        "2. %-15s %s", "RS485", (ret==0&&pr==0) ? "ok" : "err");
                        if (!automatic) {
                                printf("\n%s\n", result_list[result_idx - 1]);
                                break;
                        }

*/
                        case 2:printf("\n\n2. testing pwm...\n");sleep(2);
                                   if((ret=pwm_test("/sys/class/leds/pwm_1/brightness","/sys/class/leds/pwm_2/brightness"))!=0)
                                                err_on_testing = 1;
                                                 sprintf(result_list[result_idx++],"2. %-15s %s","pwm_test",ret == 0?"ok":"err");
                                if (!automatic) {
                                        printf("\n%s\n", result_list[result_idx - 1]);
                                        break;
                                }

                        case 3:printf("\n\n3. testing spi oled...\n");sleep(2);
                                if (ret = spi_led_test("/dev/spi_led") != 0)
                                        err_on_testing = 1;

                                sprintf(result_list[result_idx++],
                                                "3. %-15s %s", "spi_led", ret == 0 ? "ok" : "err");
                                if (!automatic) {
                                        printf("\n%s\n", result_list[result_idx - 1]);
                                        break;
                                }
                        case 4:printf("\n\n4. testing adc...\n");sleep(2);
                               if((ret=adc_test("/sys/devices/platform/at91_adc/chan4","/sys/devices/platform/at91_adc/chan6"))!=0)
                               err_on_testing = 1;

                               sprintf(result_list[result_idx++],"4. %-15s %s","adc_test",ret == 0?"ok":"err");
                               if(!automatic)
                                                           {
                                               printf("\n%s\n",result_list[result_idx - 1]);
                                                                   break;
                                           }
			case 5:printf("\n\n5. testing polarized motor...\n");sleep(2);
				   if((ret=motor_test("/sys/class/gpio/gpio143/value","/sys/class/gpio/gpio144/value","/sys/class/gpio/gpio77/value",1))!=0)
						err_on_testing = 1;
						sleep(2);
				   if((ret=motor_test("/sys/class/gpio/gpio143/value","/sys/class/gpio/gpio144/value","/sys/class/gpio/gpio77/value",0))!=0)
						err_on_testing = 1;
						sleep(2);
				   if((ret=motor_test("/sys/class/gpio/gpio143/value","/sys/class/gpio/gpio144/value","/sys/class/gpio/gpio77/value",2))!=0)
						err_on_testing = 1;
						sleep(2);
					if((ret=motor_test("/sys/class/gpio/gpio143/value","/sys/class/gpio/gpio144/value","/sys/class/gpio/gpio77/value",0))!=0)
                                                err_on_testing = 1;


						sprintf(result_list[result_idx++],"5. %-15s %s","motor_test",ret == 0?"ok":"err");
				if (!automatic) {
					printf("\n%s\n", result_list[result_idx - 1]);
					break;
				}
			case 6:printf("\n\n6. testing stepper motor...\n");sleep(2);
				   if((ret=stepper_test("/sys/class/gpio/gpio51/value","/sys/class/gpio/gpio52/value",1,1500,5000))!=0)
					   err_on_testing = 1;
                                          
				   if((ret=stepper_test("/sys/class/gpio/gpio50/value","/sys/class/gpio/gpio49/value",1,1500,5000))!=0)
					   err_on_testing = 1;
					   sleep(1);
				   if((ret=stepper_test("/sys/class/gpio/gpio50/value","/sys/class/gpio/gpio49/value",0,1500,5000))!=0)
                                           err_on_testing = 1;
				   if((ret=stepper_test("/sys/class/gpio/gpio47/value","/sys/class/gpio/gpio48/value",1,1500,5500))!=0)
					   err_on_testing = 1;
                                           
				   sprintf(result_list[result_idx++],"6. %-15s %s","stepper_test",ret == 0?"ok":"err");
				if (!automatic) {
					printf("\n%s\n", result_list[result_idx - 1]);
					break;
				}

			case 7:			
				printf("\n\n7. testing eth0...\n");sleep(2);
			    if ((ret = eth_test("eth0", "192.168.1.11", "192.168.1.12")) != 0)
				    err_on_testing = 1;

			sprintf(result_list[result_idx++],
					"7. %-15s %s", "eth0", ret == 0 ? "ok" : "err");
				if (!automatic) {
					printf("\n%s\n", result_list[result_idx - 1]);
					break;
				}
			case 8:printf("\n\n8. testing eth1...\n");sleep(2);
				if (ret = eth_test("eth1", "192.168.2.11", "192.168.2.12") != 0)
					err_on_testing = 1;

				sprintf(result_list[result_idx++],
						"8. %-15s %s", "eth1", ret == 0 ? "ok" : "err");
				if (!automatic) {
					printf("\n%s\n", result_list[result_idx - 1]);
					break;
				}
			case 9:printf("\n\n9. testing usb device...\n");sleep(2);
			        ret = system("/./usb_install.sh");
				if((ret = WEXITSTATUS(ret))!=0)
					err_on_testing = 1;
					
				sprintf(result_list[result_idx++],
						"9.%-15s %s","USB",ret == 0 ? "ok" : "err");
				if (!automatic) {
					printf("\n%s\n", result_list[result_idx - 1]);
					break;
				}
                        case 10:printf("\n\n10. testing gpio in...\n");sleep(2);
                                gpio();
                                sprintf(result_list[result_idx++],
                                                "10. %-15s %s", "gpio",  gp1==0 ? "ok" : "err");
                                if (!automatic) {
                                        printf("\n%s\n", result_list[result_idx - 1]);
                                        break;
                                }

			case 11:printf("\n\n11. testing key...\n");sleep(2);
				if (ret = ACU3000_key_test() != 0)
					err_on_testing = 1;

				sprintf(result_list[result_idx++],
						"11. %-15s %s", "key", ret == 0 ? "ok" : "err");
				if (!automatic) {
					printf("\n%s\n", result_list[result_idx - 1]);
					break;
				}
			case 12: printf("\n\n12. testing rs485...\n");
				 rs485_write("/dev/spi_usart1");
				
		
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
	pthread_t ledtest_thread_id;

	int flash_interval = pass_flash_interval;

	int automatic = 0;
	int next_option;
	int value=0;
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
	
/*	while(1)
	{
		value = gpio();
		if(value)
			break;
	}
*/	
	if (automatic)
		do_test(1);
	else
		do_test(0);

	print_result_list();
	printf("12 : rs485 plese look the 485 test result\n");
	
//	while (1);
	
	return 0;
}
