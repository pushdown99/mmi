#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "dhcp.h"

int
main()
{
	char buf[BUFSIZ],hostname[20];
	dhcp_t dhcp,*dp;
	struct sockaddr_in sin;
	int fd, len, nbyte;

	strcpy(hostname,"WAG");
	dhcp.xid = random();
	dhcp_discover_build(
		&dhcp,
		0,
		hostname,
		buf,
		&len
	);
	dump(buf,len);

	fd = udpbsock(DHCP_CPORT);
	bzero(&sin,sizeof(sin));
	sin.sin_family 		= AF_INET;
#if 0
	sin.sin_addr.s_addr = INADDR_BROADCAST;
#else
	sin.sin_addr.s_addr = inet_addr("10.120.32.3");
#endif
	sin.sin_port 		= htons(DHCP_SPORT);

	sendto(fd,buf,len,0,(struct sockaddr*)&sin,sizeof(sin));

	len = sizeof(sin);
	nbyte = recvfrom(fd,buf,BUFSIZ,0,(struct sockaddr*)&sin,&len);
	dp = (dhcp_t*)buf;
	
	dump(buf,nbyte);
	dump(&dp->yiaddr,4);

	dhcp_req_build(
		&dhcp,
		dp->yiaddr,
		hostname,
		buf,
		&len
	);
	dump(buf,len);

	bzero(&sin,sizeof(sin));
	sin.sin_family 		= AF_INET;
	sin.sin_addr.s_addr = INADDR_BROADCAST;
	sin.sin_port 		= htons(67);

	sendto(fd,buf,len,0,(struct sockaddr*)&sin,sizeof(sin));
}

