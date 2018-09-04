#include <unistd.h>
#include <fcntl.h>

#include <stdlib.h>
#include <string.h>

#include "../common.h"

typedef struct leds {
	char leds_path[32][64];
	int  leds_cnt;
	int  interval;
	int  flash_times;
} leds_t;

static leds_t leds;

int lighten_led(char *led_path, int lighten)
{
	char cmdline[64];

	sprintf(cmdline, "echo %d > %s", lighten, led_path);
	return system(cmdline);
}

void lighten_all_led(const int lighten)
{
	int idx;

	for (idx = 0; idx < leds.leds_cnt; ++idx)
		lighten_led(leds.leds_path[idx], lighten);
}

static void ledtest_thread(void)
{
	char cmdline[64];
	int times = 0;
	int idx;

	while (leds.flash_times < 1 || times++ < leds.flash_times) {
		lighten_all_led(1);

		usleep(leds.interval);

		lighten_all_led(0);
		
		usleep(leds.interval);
	}
}

int create_ledtest_thread(char (*leds_path)[64],
						  int  leds_cnt,
						  int  interval,
						  int  flash_times,
						  pthread_t  *thread_id)
{
	int idx;

	leds.leds_cnt  = leds_cnt;
	leds.interval  = interval;
	leds.flash_times = flash_times;

	for (idx = 0; idx < leds_cnt; ++idx)
		strncpy(leds.leds_path[idx], leds_path[idx], strlen(leds_path[idx]));
	
	if (pthread_create(thread_id, NULL, (void *)ledtest_thread, NULL) < 0) {
		dbg_perror("Failed to create ledtest thread");
		return -1;
	}

	return 0;
}
