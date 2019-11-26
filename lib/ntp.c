#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "bufq.h"
#include "__time.h"
#include "ntp.h"

ntph_t *ntptab[1] = { NULL, };

void ntphash(ntph_t *p)
{
    ntph_t **pp = (ntph_t**)&ntptab[0];
    if((p->next = *pp))
        p->next->prev = &p->next;
    p->prev = pp;
    *pp = p;
}

void ntpunhash(ntph_t *p)
{
    if(p->prev) {
        if((*(p->prev) = p->next))
            p->next->prev = p->prev;
        p->next = NULL;
        p->prev = NULL;
    }
}

ntph_t* ntpfind(unsigned int addr)
{
    ntph_t *p;
    for(p=(ntph_t*)ntptab[0];p;p=p->next) {
		if(addr!=p->addr) continue;
        return p;
    }
    return NULL;
}

ntph_t* ntpadd(unsigned int addr)
{
    ntph_t *p;
    if(p=ntpfind(addr)) return NULL;
    p = (ntph_t*)malloc(sizeof(ntph_t));
    if(p) {
		bzero(p,sizeof(ntph_t));
		p->addr = addr;
        p->next = NULL;
        p->prev = NULL;
        ntphash(p);
    }
    return p;
}

void ntpdel(unsigned int addr)
{
    ntph_t *p;
    if(!(p=ntpfind(addr))) return;
    ntpunhash(p);
    free(p);
}

void
prnntp(ntp_t *np,struct sockaddr_in sin,int dir)
{
    ensure(np,return);

    printf("%s %c : NTP(Network Time Protocol) packet\n"
        ,inet_ntoa(sin.sin_addr)
        ,(dir)? '>':'<'
    );
    printf("\t----------------------------------------------\n");
    printf("\tLeap Indicator            : %d \n",np->li);
    printf("\tVersion Number            : %d \n",np->vn);
    printf("\tMode                      : %d \n",np->mode);
    printf("\tStartum Indicator         : %d \n",np->stra);
    printf("\tPoll Indicator            : %d \n",np->poll);
    printf("\tPrecision Indicator       : %d \n",np->prec);
    printf("\tSynchronizing Distance    : %d \n",htonl(np->dist));
    printf("\tEstimated Drift Rate      : %d \n",htonl(np->drift));
    printf("\tRefernced Clock Identifier: %d \n",htonl(np->clkid));
    printf("\tReferenced Timestamp      : %08x%08x\n"
		,htonl(np->reftm[0]),htonl(np->reftm[1]));
    printf("\tOriginate Timestamp       : %08x%08x\n"
		,htonl(np->orgtm[0]),htonl(np->orgtm[1]));
    printf("\tReceived Timestamp        : %08x%08x\n"
		,htonl(np->rcvtm[0]),htonl(np->rcvtm[1]));
    printf("\tTransmit Timestamp        : %08x%08x\n"
		,htonl(np->xmttm[0]),htonl(np->xmttm[1]));
    printf("\n\n");
}

#define NTP_POLL	4

int
syncntp(char* host)
{
	fd_set fds;
	int fd,i,len;
	char s[BUFSIZ];
    struct sockaddr_in sin;
    struct timeval tm,tmv;
    ntp_t *np;

	if((fd=udpsock(0))<=0) return 0;

	FD_ZERO(&fds);
	FD_SET(fd,&fds);
	tmv.tv_sec	= 1;
	tmv.tv_usec	= 0;

	for(i=0;i<NTP_POLL;i++) sndntp(fd,host);
	if(select(fd+1,&fds,NULL,NULL,&tmv)<=0) goto clsfd;
	if(FD_ISSET(fd,&fds)) {
		len = sizeof(sin);
		if(recvfrom(fd,s,BUFSIZ,0,(struct sockaddr*)&sin,&len)<=0) goto clsfd;
		np = (ntp_t*)s;
    	tm.tv_sec	= htonl(np->xmttm[0])-JAN_1970;
    	tm.tv_usec	= tsftotvu(htonl(np->xmttm[1]));
    	settimeofday(&tm,NULL);
	}

clsfd:
	close(fd);
	return 1;
}

int
sndntp(int fd,char *host)
{
    char s[BUFSIZ];
    ntp_t *np = (ntp_t*)s;
    struct sockaddr_in sin;
    struct timeval tm;
    int nbyte;

    bzero((char*)&sin,sizeof(sin));
    sin.sin_family      = AF_INET;
    sin.sin_addr.s_addr = inet_addr(host);
    sin.sin_port        = htons(NTP_PORT);

    daylight = 0;
    np->li          = LEAP_NOTINSYNC;
    np->vn          = NTP_VERSION;
    np->mode        = MODE_CLIENT;
    np->stra        = 0;
    np->poll        = NTP_MINPOLL;
    np->prec        = 250;
    np->dist        = htonl(65536);
    np->drift       = htonl(65536);
    np->clkid       = 0;
    np->reftm[0]    = 0;
    np->reftm[1]    = 0;
    np->orgtm[0]    = 0;
    np->orgtm[1]    = 0;
    np->rcvtm[0]    = 0;
    np->rcvtm[1]    = 0;
    gettimeofday(&tm,NULL);
    np->xmttm[0]    = htonl(tm.tv_sec+JAN_1970);
    np->xmttm[1]    = htonl(tvutotsf(tm.tv_usec));

    nbyte = sendto(fd,np,sizeof(ntph_t),0,(struct sockaddr*)&sin,sizeof(sin));
#if 0
    if(nbyte) prnntp(np,sin,1);
#endif
    return nbyte;
}

/* Server-mode NTP Thread */
void *ntpstask(void *arg)
{
	fd_set fds,rfds;
	struct timeval tm,tmv;
	char s[BUFSIZ];
	ntp_t 	*np;
	ntph_t	*nh;
	int fd,nbyte,len,clk;
	struct sockaddr_in sin;

	fd = udpsock(NTP_PORT);

	srand(getpid()|time(0));
	clk = rand();

	FD_ZERO(&fds);
	FD_SET(fd,&fds);

	while(1) {
		rfds = fds;
		tmv.tv_sec	= 1;
		tmv.tv_usec	= 0;

		if(select(fd+1,&rfds,NULL,NULL,&tmv)<=0) continue;
		if(FD_ISSET(fd,&rfds)) {
			len = sizeof(sin);
			nbyte = recvfrom(fd,s,BUFSIZ,0,(struct sockaddr*)&sin,&len);
			if(nbyte>=sizeof(ntp_t)) {
				np = (ntp_t*)s;

				if(nh=ntpfind(sin.sin_addr.s_addr)) {
					nh->poll	-= 1;
					if(!nh->poll) ntpunhash(nh);
				}
				else if(nh=ntpadd(sin.sin_addr.s_addr)) {
					nh->poll	= np->poll;
    				gettimeofday(&tm,NULL);
					nh->sec		= htonl(tm.tv_sec+JAN_1970);
					nh->fix		= htonl(tvutotsf(tm.tv_usec));
				}

#if 0
				prnntp(np,sin,0);
#endif
				np->li			= LEAP_NOWARNING;
				np->vn			= NTP_VERSION;
				np->mode		= MODE_SERVER;
				np->stra		= 1;
				np->prec		= 238;
				np->dist		= 0;
				np->drift		= 35;
				np->clkid		= clk;
    			np->reftm[0]    = nh->sec;
    			np->reftm[1]    = nh->fix;
    			np->orgtm[0]    = np->xmttm[0];
    			np->orgtm[1]    = np->xmttm[1];
    			gettimeofday(&tm,NULL);
    			np->rcvtm[0]    = htonl(tm.tv_sec+JAN_1970);
    			np->rcvtm[1]    = htonl(tvutotsf(tm.tv_usec));
    			gettimeofday(&tm,NULL);
    			np->xmttm[0]    = htonl(tm.tv_sec+JAN_1970);
    			np->xmttm[1]    = htonl(tvutotsf(tm.tv_usec));
				sendto(fd,s,sizeof(ntph_t),0,(struct sockaddr*)&sin,sizeof(sin));
#if 0
				prnntp(np,sin,1);
#endif
			}
		}
	}
}

/* Peer-mode NTP Thread */
void *ntppm(void* arg)
{
}


#if defined(EXAMPLECODE)

#define EMS	"192.168.1.60"

int main()
{
	syncntp(EMS);
}

#endif
