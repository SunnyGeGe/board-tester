#include <fcntl.h>
#include <linux/input.h>

#include "../common.h"

typedef struct keys {
	char *key_ev_path;
	int  *codes_list;
	int  keys_cnt;
} keys_t;

static keys_t keys;
static int keys_pressed  = 0;
static int keys_released = 0;

static int requir_press_times = 1; /* default 1 */
static int record_press_times[32] = { 0 };

int key_test_thread_exit = 0;

static int  keycode_index(int code)
{
	int idx;

	for (idx = 0; idx < keys.keys_cnt; ++idx)
		if (keys.codes_list[idx] == code)
			return idx;

	return -1;
}

static void handle_event(const struct input_event *ev)
{
	int idx;

	idx = keycode_index(ev->code);
	if (idx < 0)
		return;

	if (record_press_times[idx] < requir_press_times) {
		++record_press_times[idx];
		return;
	}
	
	if (ev->value == 0)
		keys_released |= (1 << idx);
	else
		keys_pressed  |= (1 << idx);

}

static void keytest_thread(void)
{
	int key_ev_fd;
	struct input_event ev;

	key_ev_fd = open(keys.key_ev_path, O_RDONLY);
	if (key_ev_fd < 0) {
		dbg_perror("Open keys event node failed");
		err_on_testing = 1;
		return;
	}

	while (1) {
		if (read(key_ev_fd, &ev, sizeof(ev)) < 0) {
			perror("Read key event failed");
			err_on_testing = 1;
			return;
		}
		handle_event(&ev);

/*		if ((keys_pressed  == ((1 << keys.keys_cnt) - 1)) &&
			(keys_released == ((1 << keys.keys_cnt) - 1)))*/
			break;
	}

	dbg_printf("=== Key test pass ===\n");
	key_test_thread_exit = 1;
	
	close(key_ev_fd);
}

int create_keytest_thread(char *key_ev_path,
						  int *codes_list,
						  int keys_cnt,
						  pthread_t *thread_id,
						  const int times)
{
	keys.key_ev_path = key_ev_path;
	keys.codes_list  = codes_list;
	keys.keys_cnt    = keys_cnt;

	requir_press_times = times;

	if (pthread_create(thread_id, NULL, (void *)keytest_thread, NULL) != 0) {
		dbg_perror("create keytest pthread failed");
		return -1;
	}

	return 0;
}
