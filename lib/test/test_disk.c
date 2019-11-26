#include <stdio.h>

#include "disk.h"

int main()
{
    disk_t dd[10];
    int i,cnt,ret=1;

	for(cnt=0;ret;cnt++) {
   		ret = getdiskuse(&dd);

    	for(i=0;dd[i].name[0];i++) {
        	printf("%s total:%u avail:%u used:%u %u\n"
            	,dd[i].name
            	,dd[i].total
            	,dd[i].avail
            	,dd[i].used
            	,dd[i].pcnt
        	);
    	}
		printf("cnt: %d (return %d)\n",cnt,ret);

		sleep(1);
	}
}
