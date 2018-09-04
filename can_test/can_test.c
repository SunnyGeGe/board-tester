#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/error.h>
#include <string.h>
#include <stdlib.h>

#include "../common.h"

const static char *CAN_TEST_STR = "myirCANT";

static int can_init(const char *can)
{
	int canfd;
	struct ifreq ifr;
	struct sockaddr_can can_addr;
	int loopback = 0;

	memset(&ifr, 0, sizeof(ifr));
	
	canfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (canfd < 0) {
		perror("socket");
		return canfd;
	}
	
	strcpy(ifr.ifr_name, can);

	/* Determine the interface index */
	ioctl(canfd, SIOCGIFINDEX, &ifr);
	can_addr.can_family = AF_CAN;
	can_addr.can_ifindex = ifr.ifr_ifindex;

	/* bin the socket to a CAN interface */
	bind(canfd, (struct sockaddr *)&can_addr, sizeof(can_addr));

    /* Set the lookback rules */
	setsockopt(canfd, SOL_CAN_RAW, CAN_RAW_LOOPBACK,
			   &loopback, sizeof(loopback));
	
	return canfd;
}

void can_setting(const char *can,
						const int bitrate)
{
	char cmdline[128] = { '\0' };

	sprintf(cmdline,
			"ifconfig %s down; ip link set %s type can bitrate %d; ifconfig %s up",
			can, can, bitrate, can);

	system(cmdline);
    sleep(3);
}

int can_test(const char *can_0,
			 const char *can_1,
			 const int can_id,
			 const int times,
			 const int bitrate,
			 const int timeout,
			 const int interval)
{
	int canfd_0;
	int canfd_1;
	int cur_canfd;
	int loopback = 0;
	struct can_frame frame;
	struct can_filter filter;
	fd_set fds;
	struct timeval tv;
	int cnt = 0;
	int ret;

/*    if(can_0 != NULL)
	    can_setting(can_0, bitrate);
    if(can_1 != NULL)
	    can_setting(can_1, bitrate);
*/	
	if ((canfd_0 = can_init(can_0)) < 0)
		return -1;

	if (can_1 != NULL && (canfd_1 = can_init(can_1)) < 0){
		return -1;
    }

	can_err_mask_t err_mask = CAN_ERR_TX_TIMEOUT | CAN_ERR_BUSOFF | CAN_ERR_ACK | CAN_ERR_LOSTARB;


	frame.can_id = can_id;
        filter.can_id=can_id;
        filter.can_mask=CAN_SFF_MASK;

	/* timeout setting */
	tv.tv_sec  = timeout;
	tv.tv_usec = 0;

	cur_canfd = canfd_0;
	while (times < 1 || cnt++ < times) {
		memset(frame.data, 0, sizeof(frame.data));
		frame.can_dlc = strlen(CAN_TEST_STR);
		strncpy(frame.data, CAN_TEST_STR, frame.can_dlc);
		ret = write(cur_canfd, &frame, sizeof(frame));
		if (ret < 0) {
			dbg_perror("can raw write");
			goto __ERR;
		}

		/* Read data from another end */
		if (can_1 != NULL)
			cur_canfd = (cur_canfd == canfd_0) ? canfd_1 : canfd_0;
		
		memset(frame.data, 0, sizeof(frame.data));
		FD_ZERO(&fds);
		FD_SET(cur_canfd, &fds);
		if (timeout > 0) {
			ret = select(cur_canfd + 1, &fds, NULL, NULL, &tv);
			if (ret == -1) {
				dbg_perror("select can fd");
				goto __ERR;
			} else if (ret == 0) {
				dbg_printf("select can fd timeout\n");
				goto __ERR;
			}
		}
                setsockopt(cur_canfd,SOL_CAN_RAW,CAN_RAW_ERR_FILTER,err_mask,sizeof(err_mask));//过滤错误帧
                setsockopt(cur_canfd,SOL_CAN_RAW,CAN_RAW_LOOPBACK,&loopback,sizeof(loopback));//关闭本地回环      
                setsockopt(cur_canfd,SOL_CAN_RAW,CAN_RAW_FILTER,&filter,sizeof(filter));//设置规则，接收规定报文

		ret = read(cur_canfd, &frame, sizeof(frame));
		if (ret < 0) {
			dbg_perror("can raw read");
			goto __ERR;
		}

		/* Verify data */
                if (strncmp(frame.data,CAN_TEST_STR,8) != 0) 
		{
                        printf("Verified can frame.data failed\n");
                        goto __ERR;
                }

		usleep(interval);
	}

	dbg_printf("\n========== %s %s test pass ===========\n\n",
			   can_0,
			   can_1 == NULL ? " " : can_1);

	close(canfd_0);
	if (can_1 != NULL)
		close(canfd_1);
	
	return 0;

 __ERR:
	close(canfd_0);
	if (can_1 != NULL)
		close(canfd_1);
	return -1;
}

