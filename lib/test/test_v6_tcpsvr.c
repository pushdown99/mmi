#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int
main(int argc, char *argv[])
{
	unsigned short		portnum;
	int					sd;
	struct sockaddr_in6	addr;

	if (argc == 2) {
		portnum		= atoi(argv[1]);
	}
	else {
		portnum		= 9999;
	}
	
	while(1)
	{
		int		len, sent, client;
		char	line[100];

		sd	= v6_tcp_listen(portnum);
		len	= sizeof(addr);
		client	= accept(sd, (struct sockaddr*)&addr, &len);

		printf("Connected : %s port=%d\n",
				inet_ntop(AF_INET6, &addr.sin6_addr, line, sizeof(line)),
				addr.sin6_port);

		do {
			sent	= send(client, line, recv(client, line, sizeof(line), 0), 0);
			printf("server=%s", line);
		} while (sent < 0 && strcmp(line, "bye\n") != 0);
		close(client);
	}
}

