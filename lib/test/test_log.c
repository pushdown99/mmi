#include <stdio.h>

#include "log.h"

#define TEST_LOG0   0
#define TEST_LOG1   1

int main()
{
	int loop;
	char date[BUFSIZ];
	char t[BUFSIZ];
	if (initlog(TEST_LOG0, "/root/log/logtest1", "ems", "logtest1", LOG_0, LOG_FILE, MODE_PATH)<=0) {
		printf("\nLog Init Fail\n");
		exit(0);
	}
#if 1
	if (initlog(TEST_LOG1, "/root/log/logtest2", "ems", "logtest2", LOG_0, LOG_FILE, MODE_NO_PATH)<=0) {
		printf("\nLog Init Fail\n");
		exit(0);
	}
#endif
	logset(TEST_LOG0, 5, 4*1024*1024);
	logset(TEST_LOG1, 10, 8*1024*1024);
	for(loop=0;loop<10000000;loop++) {
		logprn(TEST_LOG0, LOG_0,0,"Log0- Testing (%s:%d)\r\n"
				,__FILE__,__LINE__);
	lprintf(TEST_LOG1, LOG_0, "Log1- Testing (%s:%d)\r\n"
				,__FILE__,__LINE__);
#if 0
      sleep(1);
#endif
          }
}
