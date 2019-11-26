#include <stdio.h>
#include <sys/time.h>

int
main()
{
	struct timeval timeout;
	int cnt;

	for(cnt=0;;cnt++) {
#if 1
		timeout.tv_sec  = 1;
		timeout.tv_usec = 0;
		select(0,0,0,0,&timeout);
#else
		sleep(1);
#endif
		printf("[%d] Wakeup \n",cnt);
	}
}

