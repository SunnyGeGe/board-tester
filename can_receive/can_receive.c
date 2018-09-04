#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <net/if.h> 
#include <sys/ioctl.h> 
#include <sys/socket.h> 
#include <linux/can.h> 
#include <linux/can/raw.h> 
#include <getopt.h>
#include "../common.h"

/**
 * @brief: main function  
 * @Param: argc: number of parameters
 * @Param: argv: parameters list
 */
int can_receive(char* device,const int bitrate,const int timeout)  
{  
	int s, nbytes, i,j,f=0;
	int id, next_option, device_flag=0, id_flag=0;  
	int can_fd;
	int ret;
	fd_set fds;
	struct timeval tv;
	struct sockaddr_can addr;  
	struct ifreq ifr;  
	struct can_frame frame;  
	struct can_filter rfilter[1];
	const char *const short_options = "hd:i:";
	const struct option long_options[] = {
		{ "help",   0, NULL, 'h'},
		{ "device", 1, NULL, 'd'},
		{ "id", 1, NULL, 'i'},
		{ NULL,     0, NULL, 0  }
	};

	can_setting(device,bitrate);
	
	/* create a socket */  
	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	strcpy(ifr.ifr_name, device);
	/* determine the interface index */  
	ioctl(s, SIOCGIFINDEX, &ifr);                    
	addr.can_family = AF_CAN;  
	addr.can_ifindex = ifr.ifr_ifindex;
	/* bind the socket to a CAN interface */    
	bind(s, (struct sockaddr *)&addr, sizeof(addr));
	
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	can_fd = s;
	FD_ZERO(&fds);
	FD_SET(can_fd,&fds);
	
	if(timeout > 0)
	{
		ret = select(can_fd+1,&fds,NULL,NULL,&tv);
		if(ret == -1)
		{dbg_perror("delect can fd");
		goto __ERR;
		}else if(ret == 0)
		{dbg_printf("select can fd timeout\n");
		goto __ERR;}
	}
	if (id_flag) {     
		/* define the filter rules */   
		rfilter[0].can_id   = id;  
		rfilter[0].can_mask = CAN_SFF_MASK;  
		/* Set the filter rules */	  
		setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter)); 
	}  
	for(j=0;j<100;j++) {
		/* receive frame */  
		nbytes = read(s, &frame, sizeof(frame));            
		/* printf the received frame */  
		if (nbytes > 0) { 
			printf("%s  %#x  [%d]  ", ifr.ifr_name, frame.can_id, frame.can_dlc);
			for (i = 0; i < frame.can_dlc; i++)
				printf("%#x ", frame.data[i]); 
			printf("\n"); 
			f++;
		}  
	if(f==100)
	{
		printf("\n============can test past============\n\n");
	}
	}  
	close(s);  
	return 0;  

__ERR:
	close(s);
	return -1;
}  
