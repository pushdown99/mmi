#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include "peer.h"

peer_t *prhash[peer_hash_size] = { NULL, };
void peercheck2(int);

pthread_t t_peer;
/* 2005/05/30, update		*/
pthread_t peercheck_th_t;
int peerfd	=0;
pthread_mutex_t _peer_lock = PTHREAD_MUTEX_INITIALIZER;;

peer_t* peerfind(unsigned int addr);
void 	peercheck(int duration);
void 	(*peerupcbfn)(unsigned int)=NULL;
void 	(*peerdncbfn)(unsigned int)=NULL;

void* peertask(void *args)
{
	fd_set fds,rfds;
    peer_t *p;
	char s[BUFSIZ];
	struct sockaddr_in sin;
    struct timeval tm;
	struct ip* ip;
	struct icmp *icp;
	int len;

	FD_ZERO(&fds);
	FD_SET(peerfd,&fds);

    while(1) {
        tm.tv_sec  = 1;
        tm.tv_usec = 0; 

		rfds = fds;
        if(select(peerfd+1,&rfds,NULL,NULL,&tm)<=0) continue;
		if(FD_ISSET(peerfd,&rfds)) {
			len = sizeof(sin);
			if(recvfrom(peerfd,s,BUFSIZ,0,(struct sockaddr*)&sin,&len)<=0) continue;
			ip = (struct ip*)s;
			icp = (struct icmp*)(ip+1);
			if(icp->icmp_type != ICMP_ECHOREPLY) continue;
			pthread_mutex_lock(&_peer_lock);
			if(p=peerfind(sin.sin_addr.s_addr)) {
				p->t 	= time(0);
				if(!p->stat) {
					p->stat = 1;
					if(peerupcbfn) peerupcbfn(p->addr);
				}
			}
			pthread_mutex_unlock(&_peer_lock);
		}
	}
}

#define PEER_ICMP_EXPIRE	10
#define PEER_ICMP_VALID 	6
#define PEER_ICMP_DURATION	3

void *
peercheck_th(void* arg)
{
	pthread_t myself;
	struct timeval tm;

	myself = pthread_self ();
	/* 
	log ( "thread-create = %d \n", myself ) ;
	*/

	while (1) {
		tm.tv_sec  = PEER_ICMP_DURATION ;
        tm.tv_usec = 0; 
		/* instead, sleep (1)	*/
		select ( 0, 0, 0, 0, &tm );
		peercheck (0);
	}

	pthread_exit (0);

}

void peerhash(peer_t *p)
{
    peer_t **pp;

	if(!peerfd) {
		peerfd = icmpsock();
		pthread_create (&t_peer,NULL,peertask,NULL);
/* DELETE :			*/
#if 0
		set_cb_timeout(peercheck,NULL,0);
#endif
		pthread_create (&peercheck_th_t,	NULL, 	peercheck_th,	NULL);
	}

    for(pp=&prhash[0];*pp;pp=&((*pp)->next)) {
        if((*pp)->addr > p->addr) break;
    }
    if((p->next = *pp))
        p->next->prev = &p->next;
    p->prev = pp;
    *pp = p;
}

void peerunhash(peer_t *p)
{
    if(p->prev) {
        if((*(p->prev) = p->next))
            p->next->prev = p->prev;
        p->next = NULL;
        p->prev = NULL;
    }
}

peer_t* peerfind(unsigned int addr)
{
    peer_t *p;

    for(p=prhash[0];p;p=p->next) {
        if(addr!=p->addr) continue;
        return p;
    }
    return NULL;
}

peer_t* peeradd(unsigned int addr)
{
    peer_t *p;
    if(p=peerfind(addr)) return NULL;
	pthread_mutex_lock(&_peer_lock);
    p = (peer_t*)malloc(sizeof(peer_t));
    if(p) {
		bzero(p,sizeof(peer_t));
		p->addr = addr;
		p->t	= time(0) - PEER_ICMP_DURATION;
		p->stat	= 1;
        p->next = NULL;
        p->prev = NULL;
        peerhash(p);
    }
	pthread_mutex_unlock(&_peer_lock);
    return p;
}

void peerdel(unsigned int addr)
{
    peer_t *p;
    if(!(p=peerfind(addr))) return; /* not found */
	pthread_mutex_lock(&_peer_lock);
    peerunhash(p);
    free(p);
	pthread_mutex_unlock(&_peer_lock);
}

int peerstat(unsigned int addr)
{
	peer_t *p;
    if(!(p=peerfind(addr))) return 0;
	return p->stat;
}

void peercheck(int duration)
{
	struct icmp *icp;
	char s[BUFSIZ],payload[56];
	struct sockaddr_in sin;
    peer_t *p;
	time_t now;
	int num,len,loop;

	for(num=0;num<56;num++) payload[num] = '0'+num;

	pthread_mutex_lock(&_peer_lock);
	for(now=time(0),p=prhash[0];p;p=p->next) {
		/* EXception */
		if(now < p->t) p->t=now;

		if((now-(p->t) >= PEER_ICMP_EXPIRE) && p->stat) {
			p->stat = 0;
			if(peerdncbfn) peerdncbfn(p->addr);
		}
	}

	for(loop=0;loop<1;loop++) {
		icp = (struct icmp*)s;
		icp->icmp_type  = ICMP_ECHO;
		icp->icmp_code  = 0;
		icp->icmp_cksum = 0;
		icp->icmp_id    = htons(loop);
		icp->icmp_seq   = htons(loop);
		memcpy(&s[8],payload,56);
		len = 8+56;
		icp->icmp_cksum = in_chksum((u_short*)icp,len);	

		bzero(&sin,sizeof(sin));
		sin.sin_family		= AF_INET;

		for(now=time(0),p=prhash[0];p;p=p->next) {
			if((now-p->t) < PEER_ICMP_VALID) 	continue;
			if((now-p->t) > PEER_ICMP_EXPIRE) 	{ p->t = now; continue; }
			sin.sin_addr.s_addr	= p->addr;
			sendto(peerfd,s,len,0,(struct sockaddr*)&sin,sizeof(sin));
		}
	}
	pthread_mutex_unlock(&_peer_lock);
	//set_cb_timeout(peercheck,NULL,PEER_ICMP_DURATION*1000);
}

#if defined(EXAMPLECODE)

void sysup(unsigned int addr)
{
	char t[20];
	fmttm("%Y/%m/%d %H:%M:%S",t);
	printf("[Up] %s %s \n",t,inet_ntoa(*(struct in_addr*)&addr));
}

void sysdn(unsigned int addr)
{
	char t[20];
	fmttm("%Y/%m/%d %H:%M:%S",t);
	printf("[Dn] %s %s \n",t,inet_ntoa(*(struct in_addr*)&addr));
}

int main()
{
	unsigned int addr,base;
	int i;

	base = inet_addr("192.168.1.1");

	for(i=0;i<100;i++) {
		addr = htonl(ntohl(base)+i);
		peeradd(addr);
	}

	peerupcbfn = sysup;
	peerdncbfn = sysdn;

	while(1) {
		sleep(1);
	}
}

#endif

/* 2005.05.25 hyhwang -- */

void* peertask2(void *args)
{
    fd_set fds,rfds;
    peer_t *p;
    char s[BUFSIZ];
    struct sockaddr_in sin;
    struct timeval tm;
    struct ip* ip;
    struct icmp *icp;
    int len;

    FD_ZERO(&fds);
    FD_SET(peerfd,&fds);

    while(1) {
        tm.tv_sec  = 1;
        tm.tv_usec = 0;

        rfds = fds;
        if(select(peerfd+1,&rfds,NULL,NULL,&tm)<=0) continue;
        if(FD_ISSET(peerfd,&rfds)) {
            len = sizeof(sin);
            if(recvfrom(peerfd,s,BUFSIZ,0,(struct sockaddr*)&sin,&len)<=0) continue;
            ip = (struct ip*)s;
            if(ip->ip_p == 4) ip += 1;
            icp = (struct icmp*)(ip+1);
            if(icp->icmp_type != ICMP_ECHOREPLY) continue;
            if(p=peerfind(sin.sin_addr.s_addr)) {
                p->t    = time(0);
                if(!p->stat) {
                    p->stat = 1;
                    if(peerupcbfn) peerupcbfn(p->addr);
                }
            }
        }
    }
}

int _peerlst[256];
int _peercnt=0;


void initpeer2(int cnt, int *addr)
{
    if(!peerfd) {
        peerfd = socket(AF_INET, SOCK_RAW, 4 /* IPIP */);
        pthread_create(&t_peer,NULL,peertask2,NULL);
        set_cb_timeout(peercheck2,NULL,0);
    }
    _peercnt    = cnt;
    memcpy((char*)_peerlst,(char*)addr,sizeof(int)*cnt);
}

void peercheck2(int duration)
{
    struct ip* ip;
    struct icmp *icp;
    char s[BUFSIZ],payload[56];
    struct sockaddr_in sin;
    peer_t *p;
    time_t now;
    int num,len,loop, nbyte;

    for(num=0;num<56;num++) payload[num] = '0'+num;

    for(now=time(0),p=prhash[0];p;p=p->next) {
        /* EXception */
        if(now < p->t) p->t=now;

        if((now-(p->t) >= PEER_ICMP_EXPIRE) && p->stat) {
            p->stat = 0;
            if(peerdncbfn) peerdncbfn(p->addr);
        }
    }

    /* send IP-IP packet */

    for(loop=0;loop<_peercnt;loop++) {
        ip  = (struct ip*)s;
        ip->ip_hl           = 5;
        ip->ip_v            = 4;
        ip->ip_tos          = 0;
        ip->ip_len          = htons(20+8+56);
        ip->ip_id           = 0;
        ip->ip_off          = 0;
        ip->ip_ttl          = 64;
        ip->ip_p            = 1;
        ip->ip_sum          = 0;
        ip->ip_src.s_addr   = 0;

        icp = (struct icmp*)(ip+1);
        icp->icmp_type  = ICMP_ECHO;
        icp->icmp_code  = 0;
        icp->icmp_cksum = 0;
        icp->icmp_id    = htons(loop);
        icp->icmp_seq   = htons(loop);
        memcpy(&s[28],payload,56);
        len = 8+56;
        icp->icmp_cksum = in_chksum((u_short*)icp,len);

        bzero(&sin,sizeof(sin));
        sin.sin_family      = AF_INET;

        for(now=time(0),p=prhash[0];p;p=p->next) {
            if((now-p->t) < PEER_ICMP_VALID)    continue;
            if((now-p->t) > PEER_ICMP_EXPIRE)   { p->t = now; continue; }
            ip->ip_dst.s_addr   = p->addr;
            sin.sin_addr.s_addr = _peerlst[loop];   /* outer IP address */
            nbyte = sendto(peerfd,s,20+len,0,(struct sockaddr*)&sin,sizeof(sin));
        }
    }
    set_cb_timeout(peercheck2,NULL,PEER_ICMP_DURATION*1000);
}

