#include <stdio.h>
#include <sys/socket.h>

#include "__time.h"
#include "ntp.h"

int main(int argc,char *argv[])
{
	if(argc<2) {
		printf("Usage: %s {host address} \n",argv[0]);
		exit(0);
	}
    syncntp(argv[1]);
}

