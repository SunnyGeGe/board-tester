#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int mtd_test(const char *mtd,
			 const char *test_str)
{
	int mtd_fd;
	char read_back[1024], buf[10],cmd[48];
	int len = strlen(test_str);
    int ret, i;
    len = 10;
    for(i = 0; i < len; i++)
        buf[i] = i;
    sprintf(cmd,"flash_erase %s 0 1",mtd);
    ret = system(cmd);
    if(ret){
        printf("%s erase failed\n", *mtd);
        return -1;
    }

	if ((mtd_fd = open(mtd, O_RDWR)) < 1) {
		printf("Failed to open %s\n", mtd);
		return -1;
	}

	if (write(mtd_fd, buf, len) < len) {
		printf("Write to %s failed\n", mtd);
		return -1;
	}

	lseek(mtd_fd, SEEK_SET, 0);
    
	memset(read_back, 0, 1024);
    ret = read(mtd_fd, read_back, len);
	if (ret < len) {
		printf("Read from %s failed\n", mtd);
		return -1;
	}
    for (i = 0; i < len; i++){
        //printf("0x%x\n", read_back[i]);
        
        if(read_back[i] != buf[i]){
		printf("Varify on %s failed, %d\n", mtd, ret);
		return -1;
        }
    }
#if 0
	if (strncmp(test_str, read_back, len) != 0) {
		printf("Varify on %s failed, %d\n", mtd, ret);
		return -1;
	}
#endif

	return 0;
}
