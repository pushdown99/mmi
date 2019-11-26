#include <stdio.h>

int main()
{
	int fd;

	while((fd=icmpsock())>0) printf("icmp socket: %d \n",fd);
}

