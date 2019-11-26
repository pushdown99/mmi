#include <stdio.h>

int
main()
{
	int fd;
	while((fd=udpsock(0))>0) printf("udp socket : %d \n",fd);
}

