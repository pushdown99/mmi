#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "radius.h"


#define MAXATTR	64

int
main()
{
	char buf[BUFSIZ];
	char passwd[BUFSIZ];
	struct sockaddr_in sin;
	int len,n;
	int fd;
	int ip1,ip2;
	int d1;
	char sid[8];
	rattr_t ap[MAXATTR];
	radius_t *rp;

	ip1 = inet_addr("10.120.32.5");

	bzero(passwd,18);
	bcopy("admin",passwd,5);

	set_radius_init(ap,MAXATTR);
	set_radius_attr(ap,0,AT_USER_NAME,		   10, "0162701004" );
	set_radius_attr(ap,1,AT_USER_PASSWD,       18, passwd);
/*
	set_radius_attr(ap,1,AT_NAS_IP_ADDRESS,     4, &ip1);
	set_radius_attr(ap,2,AT_CALLING_STATION_ID,10, "0162701004" );
	set_radius_attr(ap,3,AT_CHAP_PASSWD,4, "test" );
	set_radius_attr(ap,4,AT_CHAP_CHALLENGE,4, "test" );
*/

	radius_access_request_build(1,"test",ap,buf,&len);
/*
	len+=add_radius_vattr(&buf[len],AT_3GPP2_SO,4, &d1);
	len+=add_radius_vattr(&buf[len],AT_3GPP2_IP_TECH,4, &d1);
	len+=add_radius_vattr(&buf[len],AT_3GPP2_HA_IP_ADDR,4, &ip1);
	rp = (radius_t*)buf;
	rp->len = len;
*/

	printf("Radius :: account-request packet \n");
	dump(buf,len);

	fd = udpsock(0);
	sin.sin_family 			= AF_INET;
	sin.sin_port 			= 1812;
	sin.sin_addr.s_addr 	= inet_addr("10.120.32.3");

	sendto(fd,buf,len,0,(struct sockaddr*)&sin,sizeof(sin));
	len = sizeof(sin);
	n = recvfrom(fd,buf,BUFSIZ,0,(struct sockaddr*)&sin,&len);
	close(fd);
	dump(buf,n);
}

