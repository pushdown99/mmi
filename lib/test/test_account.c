#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "radius.h"


int
main()
{
	char buf[BUFSIZ];
	struct sockaddr_in sin;
	int len,n;
	int fd;
	int status = PW_STATUS_START;
	int ip1,ip2;
	char sid[8];
	rattr_t ap[8];

	set_radius_init(ap,8);
	set_radius_attr(ap,0,AT_ACCT_STATUS_TYPE,	4, &status	);
	set_radius_attr(ap,1,AT_CALLING_STATION_ID,10, "0162701004" );
	set_radius_attr(ap,2,AT_CALLED_STATION_ID, 10, "0162650930" );
	set_radius_attr(ap,3,AT_NAS_IP_ADDRESS,     4, &ip1);
	set_radius_attr(ap,4,AT_FRAMED_IP_ADDRESS,  4, &ip2);
	set_radius_attr(ap,5,AT_USER_NAME,		   10, "0162701004" );
	set_radius_attr(ap,6,AT_ACCT_SESSION_ID,    8, sid);

	fd = udpsock(0);
	radius_account_request_build(1,PW_STATUS_START,"test",ap,buf,&len);
	printf("Radius :: account-request packet \n");
	dump(buf,len);

	sin.sin_family 			= AF_INET;
	sin.sin_port 			= 1813;
	sin.sin_addr.s_addr 	= inet_addr("10.120.32.3");

	sendto(fd,buf,len,0,(struct sockaddr*)&sin,sizeof(sin));
	len = sizeof(sin);
	n = recvfrom(fd,buf,BUFSIZ,0,(struct sockaddr*)&sin,&len);
	close(fd);
	dump(buf,n);
}

