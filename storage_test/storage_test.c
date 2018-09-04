#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../common.h"

const static char *STORAGE_TEST_STR = "MYiR storage write/read test";

static int dostat(char *path, struct stat *st, int do_lstat)
{
	int n;

	if (do_lstat)
		n = lstat(path, st);
	else
		n = stat(path, st);

	if (n != 0) {
		dbg_printf("mountpoint: %s: %s\n", path,
					strerror(errno));
		return -1;
	}
	
	return 0;
}

int is_mountpoint(const char *path)
{
	struct stat st;
	struct stat st2;
	char buf[256];

	if (dostat(path, &st, 1) < 0)
		return 0;

	if (!S_ISDIR(st.st_mode))
		return 0;

	memset(buf, 0, sizeof(buf));
	strncpy(buf, path, sizeof(buf) - 4);
	strcat(buf, "/..");
	if (dostat(buf, &st2, 0) < 0)
		return 0;

	return (st.st_dev != st2.st_dev) ||
		(st.st_dev == st2.st_dev && st.st_ino == st2.st_ino);
}

int get_usb_mp(const char *media_path, char (*mp_list)[64])
{
	struct dirent *ent = NULL;
	DIR *p_dir = NULL;
	int cnt = 0;
	
	p_dir = opendir(media_path);
	if (!p_dir)
		return cnt;
	
	while (NULL != (ent = readdir(p_dir))) {
		if (strncmp(ent->d_name, "card", 4) == 0) {
			sprintf(mp_list[cnt], "%s/%s", media_path, ent->d_name);
			if (is_mountpoint(mp_list[cnt]))
				++cnt;
		}
	}

	return cnt;
}

int storage_test(const char *mount_point,
				 const char *file_name)
{
	int fd;
	char read_back[32] = { '\0' };
	char full_path[128] = { '\0' };
	char cur_path[128] = { '\0' };

	getcwd(cur_path, 128);
	
	if (access(mount_point, F_OK) != 0) {
		dbg_printf("%s not exist\n", mount_point);
		return -1;
	}

	strncat(full_path, mount_point, strlen(mount_point));
	strncat(full_path, "/", 1);
	strncat(full_path, file_name, strlen(file_name));

	chdir(mount_point);
	
	fd = open(file_name, O_RDWR | O_CREAT | O_TRUNC);
	if (fd < 1) {
		dbg_printf("Failed to open test file: %s\n", full_path);
		return -1;
	}

	if (write(fd, STORAGE_TEST_STR, strlen(STORAGE_TEST_STR)) < 0) {
		dbg_printf("Write to %s failed\n", mount_point);
		goto __ERR;
	}

	lseek(fd, SEEK_SET, 0);
	
	if (read(fd, read_back, strlen(STORAGE_TEST_STR)) < 0) {
		dbg_printf("Read from %s failed\n", mount_point);
		goto __ERR;
	}

	if (strcmp(STORAGE_TEST_STR, read_back) != 0) {
		dbg_printf("Verified failed\n");
		dbg_printf("  expect: %s\n", STORAGE_TEST_STR);
		dbg_printf("  got:    %s\n", read_back);
		goto __ERR;
	}

	chdir(cur_path);
	close(fd);
	
	return 0;

 __ERR:
	close(fd);
	return -1;
}
