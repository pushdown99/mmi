#include <stdio.h>

#include "__time.h"

#define MAXHOST	20

char hosts[MAXHOST][20] = {
	"192.168.1.45",
	"192.168.1.50",
	"192.168.1.60",
	"192.168.1.62",
	""
};

int
main()
{
	char buf[BUFSIZ];
	int i;

	while(1) {
	printf("-> %s\n",fmttm("%y/%m/%d %H:%S.%t",buf));
	for(i=0;i<MAXHOST;i++) {
		if(!hosts[i][0]) break;
		if (pingtest(hosts[i],0,0,500000) <= 0) {
			if (pingtest(hosts[i],0,0,500000) <= 0) {
				if (pingtest(hosts[i],0,0,500000) <= 0) {
				}
				else {
					printf("     - %s alive\n",hosts[i]);
				}
			}
			else {
				printf("     - %s alive\n",hosts[i]);
			}
		}
		else {
			printf("     - %s alive\n",hosts[i]);
		}
	}
	#if 0
	msleep(5,0);
	#endif
	printf("-- %s\n",fmttm("%y/%m/%d %H:%S.%t",buf));
	sleep(5);
	printf("<- %s\n",fmttm("%y/%m/%d %H:%S.%t",buf));
	}
}

