#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>


#include "__ipc.h"

#define MSGQ	0x5000L

int
main()
{
	char buf[BUFSIZ];

	initq(MSGQ);

	sendq(MSGQ,"Hello",5);
	recvq(MSGQ,buf);
	printf("buf: %s \n",buf);
}

