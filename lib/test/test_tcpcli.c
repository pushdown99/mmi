#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int
main(int argc,char *argv[])
{
	int fd;
	unsigned int addr;
	unsigned short port;

	if(argc<3) {
		printf("Usage: %s {addr} {port} \n",argv[0]);
		exit(0);
	}
	addr = inet_addr(argv[1]);
	port = atoi(argv[2]);

	fd = tcp_connect(addr,port);
	printf("tcp %s \n",(fd>0)? "connected":"no response");
	close(fd);

}

