#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "../common.h"

int adc_test(const char* dev1,const char* dev2)
{
    int adc1,fd_adc1 = -1;
    int adc2,fd_adc2 = -2;
    int adc_2 = -1;
    int ret = -1;
    int i=20;
    char buf[20];

    if((fd_adc1 = open(dev1, O_RDONLY))<0)
    {
	perror(dev1);
	goto err_exit;
    }
    
    if((fd_adc2 = open(dev2, O_RDONLY))<0)
    {
	perror(dev2);
	goto err_exit;
    }

    printf("please chance the vol\n");
       do
    	{
//	adc1 = adc2 = -1;
	lseek(fd_adc1, 0L, SEEK_SET);
	memset(buf, 0 ,sizeof(buf));
	if(read(fd_adc1,buf,sizeof(buf))<=0)
	{
	    perror("read adc1");
	}else{
	    adc1 = atoi(buf);
	}
	
//	adc_2 = adc2;
	lseek(fd_adc2, 0L, SEEK_SET);
	memset(buf, 0 ,sizeof(buf));
	if(read(fd_adc2,buf,sizeof(buf))<=0)
	{
	    perror("read adc2");
	}else{
	    adc2 = atoi(buf);
	}
//	if(adc_2!=adc2)
//	{
	    printf("ADC_1 : %5d, ADC_2 : %5d\r\n",adc1,adc2);
//	}
	fflush(stdout);
	sleep(1);

    }while(i--);
    return 0;

err_exit:
    if(fd_adc1 > 0)
	close(fd_adc1);
   
    if(fd_adc2 > 0)
	close(fd_adc2);

    return -1;
}
