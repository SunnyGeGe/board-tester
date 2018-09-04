#ifndef COMMON_H #define ETH_TEST_H

#include <stdio.h>
#include <pthread.h>

#define DEBUG 1

#ifdef DEBUG
#define dbg_printf(fmt, args...) printf(fmt, ##args)
#define dbg_perror(msg) (perror(msg))
#else
#define dbg_printf(fmt, args...)
#define dbg_perror(msg)
#endif

extern int pr;
extern time_t end_time;
extern pthread_mutex_t mutex;
extern int err_on_testing;
extern int key_test_thread_exit;
extern int lcd_drm_test();
extern int rs485_write(const char *dev);
extern void can_setting(const char* can,const int bitrate);
//extern int uart_receive(char* dev);
//extern int uart_send(char* dev);

extern  int open_tty(const char *tty_dev);
extern  int set_tty(int fd,
                                   int bitrate,
                                   int datasize,
                                   char par,
                                   int stop);

extern void wifi_ap_enable(const char* tty_dev,int bitrate,int datasize,char par,int stop);
extern void wifi_zver_enable(const char* tty_dev,int bitrate,int datasize,char par,int stop);
extern void wifi_restore(const char* tty_dev,int bitrate,int datasize,char par,int stop);

extern int can_receive(char* device,const int bitrate,const int timeout);
extern int lcd_test();
extern void creat_usb_0_thread(void);
extern void creat_usb_1_thread(void);
extern void creat_uart1_thread(void);
extern void creat_uart2_thread(void);
extern void creat_uart3_thread(void);
extern void creat_uart4_thread(void);
extern void creat_rs485_1_thread(void);
extern void creat_rs485_2_thread(void);
extern void creat_rs485_3_thread(void);
extern void creat_rs485_4_thread(void);
extern void creat_rs485_5_thread(void);
extern void x335_rs485_0_thread(void);
extern void x335_rs485_1_thread(void);
extern int spi_led_test(const char *dev);

extern int ACU3000_key_test();

extern int pwm_test();

extern int stepper_test(const char *dev1,const char *dev2,int dir,int speed,int cnt);

extern int motor_test(const char *dev1,const char *dev2,const char *dev3,int op);

extern int adc_test(const char* dev1,const char* dev2);

extern int eth_test(const char *eth,
					const char *dest_addr,
					const char *eth_ip);

extern int can_test(const char *can_0,
					const char *can_1,
					const int can_id,
					const int times,
					const int bitrate,
					const int timeout,
					const int interval);
extern int can_test1(const char *can_0,
					const int can_id,
					const int times,
					const int bitrate,
					const int timeout,
					const int interval);

extern int can_test2(const char *can_0,
					const char *can_1,
					const int can_id,
					const int times,
					const int bitrate,
					const int timeout,
					const int interval);


extern int uart_test(const char *tty_dev,
					 int bitrate,
					 int datasize,
					 char par,
					 int stop,
					 int rs485_mode,
					 int times,
					 int timeout,
					 int interval);

extern int uart_test2(const char *tty_dev,
					 int bitrate,
					 int datasize,
					 char par,
					 int stop,
					 int rs485_mode,
					 int times,
					 int timeout,
					 int interval);

extern int create_keytest_thread(char *key_ev_path,
								 int *codes_list,
								 int keys_cnt,
								 pthread_t *thread_id,
								 const int times);

extern int create_audiotest_thread(pthread_t *thread_id);

extern int get_usb_mp(const char *media_path, char (*mp_list)[64]);
extern int storage_test(const char *mount_point,
						const char *file_name);

extern int lighten_led(char *led_path, int lighten);
extern void lighten_all_led(const int lighten);
extern int create_ledtest_thread(char (*leds_path)[64],
								 int  leds_cnt,
								 int  interval,
								 int  flash_times,
								 pthread_t  *thread_id);

extern int mtd_test(const char *mtd,
					const char *test_str);

extern int camera_test(int argc, char *argv[]);

extern int eeprom_test(const char *dev,const int i2c_addr);

#endif /* COMMON_H end */
