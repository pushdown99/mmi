#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>

#include "dns.h"

int
main(int argc,char *argv[])
{
	char buf[BUFSIZ];
	int len,n;
	int fd;
	unsigned int addr;
	struct sockaddr_in sin;
#if 0
	if(argc<2) {
		printf("Usage: %s {name}\n",argv[0]);
		exit(0);
	}
	fd =udpsock(0);
	printf("\n\ndns: query (%s)\n","%s",argv[1]);
	dns_query(
		argv[1],
		&addr
	);
	printf("resolved addr %s \n",inet_ntoa(*(struct in_addr*)&addr));
#endif
#if 0
	if(argc<3) {
		printf("Usage: %s {user} {addr} \n",argv[0]);
		exit(0);
	}
	fd =udpsock(0);
	printf("\n\ndns: dynamic update \n");
	dns_update_build(
		argv[1],
		inet_addr(argv[2]),
		buf,
		&len
	);
	if(len>0) {
		dump(buf,len);
		dns_hdr_prn((dns_t*)buf);

		sin.sin_family 	= AF_INET;
		sin.sin_port 	= htons(53);
		sin.sin_addr.s_addr = inet_addr("10.120.3.103");

		sendto(fd,buf,len,0,(struct sockaddr*)&sin,sizeof(sin));
		len = sizeof(sin);
		n = recvfrom(fd,buf,BUFSIZ,0,(struct sockaddr*)&sin,&len);
		dump(buf,n);
		dns_hdr_prn((dns_t*)buf);
	}
#endif
#if 1
	if(argc<3) {
		printf("Usage: %s {user} {addr} \n",argv[0]);
		exit(0);
	}
	fd =udpsock(0);
	printf("release sending\n");
	dns_release_build(
		argv[1],
		inet_addr(argv[2]),
		buf,
		&len
	);
	dump(buf,len);
	dns_hdr_prn((dns_t*)buf);
	sin.sin_family 	= AF_INET;
	sin.sin_port 	= htons(53);
	sin.sin_addr.s_addr = inet_addr("10.120.3.103");
	sendto(fd,buf,len,0,(struct sockaddr*)&sin,sizeof(sin));
	len = sizeof(sin);
	n = recvfrom(fd,buf,BUFSIZ,0,(struct sockaddr*)&sin,&len);
	dump(buf,n);
	dns_hdr_prn((dns_t*)buf);
#endif
}

