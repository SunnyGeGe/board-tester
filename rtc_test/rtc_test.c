#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "../common.h"

#define DEFAULT_HOUR 12
#define DEFAULT_MIN  12
#define DEFAULT_SEC  12
#define DEFAULT_YEAR 2015
#define DEFAULT_MON  12
#define DEFAULT_DAY  12

int rtc_read_time(int fd, struct rtc_time *rtc_tm)
{
	int retval;

	/*read the RTC time/date*/
	retval = ioctl(fd, RTC_RD_TIME, rtc_tm);
	if (retval == -1) {
		perror("RTC_RD_TIME ioctl");
		return retval;
	}

	fprintf(stderr, "\n\tCurrent RTC date/time is %d-%d-%d, %02d:%02d:%02d.\n\n",
			rtc_tm->tm_mday, rtc_tm->tm_mon + 1, rtc_tm->tm_year + 1900,
			rtc_tm->tm_hour, rtc_tm->tm_min, rtc_tm->tm_sec);

	return 0;
}


int rtc_set_time(int fd, int hour, int minute, int second, int year, int month, int day)
{
	int retval;
	struct rtc_time rtc_tm;

	rtc_tm.tm_mday = day;
	rtc_tm.tm_mon = month - 1;
	rtc_tm.tm_year = year - 1900;
	rtc_tm.tm_hour = hour;
	rtc_tm.tm_min = minute;
	rtc_tm.tm_wday = rtc_tm.tm_yday = rtc_tm.tm_isdst = 0;
	rtc_tm.tm_sec = second;
	/*set the RTC time/date*/
	retval = ioctl(fd, RTC_SET_TIME, &rtc_tm);
	if (retval == -1) {
		perror("RTC_SET_TIME ioctl");
		return retval;
	}

	fprintf(stderr, "\n\t\tdate/time is updated to:  %d-%d-%d, %02d:%02d:%02d.\n\n",
			rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
			rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

	return 0;
}

int rtc_test(const char *rtc_dev)
{
	int fd;
	struct rtc_time rtc;

	fd = open(rtc_dev, O_RDWR);
	if (fd < 1) {
		dbg_perror("Open rtc failed");
		err_on_testing = 1;
		return -1;
	}

	if (rtc_set_time(fd, DEFAULT_HOUR, DEFAULT_MIN, DEFAULT_SEC,
					 DEFAULT_YEAR, DEFAULT_MON, DEFAULT_DAY) < 0) {
		dbg_printf("Failed to set time to %s\n", rtc_dev);
		return -1;
	}

	if (rtc_read_time(fd, &rtc) < 0) {
		dbg_printf("Failed to get time from %s\n", rtc_dev);
		return -1;
	}

	if (rtc.tm_mday != DEFAULT_DAY ||
		(rtc.tm_mon + 1)  != DEFAULT_MON ||
		(rtc.tm_year + 1900) != DEFAULT_YEAR ||
		rtc.tm_hour != DEFAULT_HOUR ||
		rtc.tm_min  != DEFAULT_MIN) {
		dbg_printf("Verified time error\n");
		return -1;
	}

	return 0;
}
	
