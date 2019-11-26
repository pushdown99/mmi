#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int
main(int argc, char *argv[])
{
	int		sd, portnum;
	struct sockaddr_in6 addr;
	char 	line[100];

	if (argc == 3) {
		portnum = atoi(argv[2]);
	}
	else {
		portnum = 9999;
	}

	sd = v6_tcp_connect(argv[1], portnum);
	do {
		fgets(line, sizeof(line), stdin);
		send(sd, line, strlen(line)+1, 0);
		recv(sd, line, sizeof(line), 0);
		printf("client=%s", line);
	} while (strcmp(line, "bye\n") != 0);
	close(sd);
}

