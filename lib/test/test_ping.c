#include <stdio.h>
#include <pthread.h>

pthread_t pingthread_t;

#if 0
int
main(int argc,char *argv[])
{
	char buf[BUFSIZ];

	if(argc<2) {
		printf("Usage: %s {host-address} \n",argv[0]);
		return 0;
	}
	printf("pingtest (0.5 sec) \n");
	printf("-> %s \n",fmttm("%y/%m/%d %H:%M:%S.%t",buf));
	printf("%s %s.\n",argv[1],(pingtest(argv[1],0,1,500000)>0)? "alive":"die");
	printf("<- %s \n",fmttm("%y/%m/%d %H:%M:%S.%t",buf));
}

#else

char pingaddr[32];

void *
pingthread(void *arg)
{
	while(1) {
printf("%s:%d \n",__FILE__,__LINE__);
		if (pingtest(pingaddr,0,0,500000) <= 0) {
			system("date >> ./ping.txt");
printf("%s:%d \n",__FILE__,__LINE__);
			if (pingtest(pingaddr,0,0,500000) <= 0) {
				system("date >> ./ping.txt");
printf("%s:%d \n",__FILE__,__LINE__);
				if (pingtest(pingaddr,0,0,500000) <= 0) {
					system("date >> ./ping.txt");
				}
			}
		}
		msleep(3);
	}
}

int
main(int argc,char *argv[])
{
	if(argc<2) {
		printf("Usage: %s {host-address} \n",argv[0]);
		return 0;
	}
	strcpy(pingaddr,argv[1]);
	pthread_create(&pingthread_t,NULL,pingthread,NULL);

	while(1) {
	sleep(10);
	}
/*
	printf("%s %s.\n",argv[1],(pingtest(argv[1],5,1,0))? "alive":"die");
*/
}
#endif
