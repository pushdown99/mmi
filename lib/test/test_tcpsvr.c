#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int
main(int argc,char *argv[])
{
	unsigned short port;
	struct sockaddr sin;
	int lfd,fd;
	int len;
	int pid;

	if(argc<2) {
		printf("Usage: %s {port} \n",argv[0]);
		exit(0);
	}
	port = atoi(argv[1]);
	printf("TCP socket:%d listen\n",port);

while(1) {
	lfd = tcp_listen(port);
	len = sizeof(sin);
	fd 	= accept(lfd,&sin,&len);

	pid = fork();
	if(pid) {
		close(fd);
	}
	else {
		close(lfd);
		sleep(100);
	}
	}


sleep(100);
	close(fd);
	close(lfd);
}

