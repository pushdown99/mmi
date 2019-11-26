#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#if 0
int
icmpsock()
{
    struct protoent* proto;
    int fd,opt=1;

    if((proto=getprotobyname("icmp"))==NULL) {
        return -1;
    }
    if((fd=socket(AF_INET,SOCK_RAW,proto->p_proto))<0) {
        return -1;
    }
    setsockopt(fd,SOL_SOCKET,SO_DEBUG,(char*)&opt,sizeof(opt));
    opt=0;
    setsockopt(fd,SOL_SOCKET,SO_DONTROUTE,(char*)&opt,sizeof(opt));
    return fd;
}
#endif

#define BACKLOG	15

int
v6_tcp_listen(unsigned short port)
{
    int fd,opt=1;
    struct protoent* proto;
    struct sockaddr_in6 snd;
#if 0
    if((proto=getprotobyname("tcp"))==NULL) {
        return -1;
    }
    if((fd=socket(PF_INET6,SOCK_STREAM,proto->p_proto))<0) {
        return -1;
    }
#endif
    if((fd=socket(PF_INET6,SOCK_STREAM,0))<0) {
        return -1;
    }
    bzero((char*)&snd,sizeof(snd));
    snd.sin6_family      = AF_INET6;
    snd.sin6_port        = htons(port);
	if (inet_pton(AF_INET6, "0::0", &snd.sin6_addr) == 0) {
		return -1;
	}
    if(bind(fd,(struct sockaddr*)&snd,sizeof(snd)) != 0) {
        return -1;
    }
	if(listen(fd,BACKLOG)<0) {
		return -1;
	}
    return fd;
}

int
v6_tcp_connect(char *addr,unsigned short port)
{
    int fd,opt=1;
    struct protoent* proto;
    struct sockaddr_in6 snd;

#if 0
    if((proto=getprotobyname("tcp"))==NULL) {
        return -1;
    }
    if((fd=socket(PF_INET6,SOCK_STREAM,proto->p_proto))<0) {
        return -1;
    }
#endif
    if((fd=socket(PF_INET6,SOCK_STREAM,0))<0) {
        return -1;
    }
    bzero((char*)&snd,sizeof(snd));
    snd.sin6_family		= AF_INET6;
    snd.sin6_port        = htons(port);
	if(inet_pton(AF_INET6, addr, &snd.sin6_addr) == 0) {
		return -1;
	}
    if(connect(fd,(struct sockaddr*)&snd,sizeof(snd))<0) {
        return -1;
    }
    return fd;
}

#if 0
int
udpsock(unsigned short port)
{
    int fd,opt=1;
    struct protoent* proto;
    struct sockaddr_in snd;

    if((proto=getprotobyname("udp"))==NULL) {
        return -1;
    }
    if((fd=socket(AF_INET,SOCK_DGRAM,proto->p_proto))<0) {
        return -1;
    }
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
    bzero((char*)&snd,sizeof(snd));
    snd.sin_family      = AF_INET;
    snd.sin_addr.s_addr = htonl(INADDR_ANY);
    snd.sin_port        = htons(port);
    if(bind(fd,(struct sockaddr*)&snd,sizeof(snd))<0) {
        return -1;
    }
    return fd;
}

int
udpbsock(unsigned short port)
{
    int fd,opt=1;
    struct protoent* proto;
    struct sockaddr_in snd;

    if((proto=getprotobyname("udp"))==NULL) {
        return -1;
    }
    if((fd=socket(AF_INET,SOCK_DGRAM,proto->p_proto))<0) {
        return -1;
    }
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
    bzero((char*)&snd,sizeof(snd));
    snd.sin_family      = AF_INET;
    snd.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    snd.sin_port        = htons(port);
    if(bind(fd,(struct sockaddr*)&snd,sizeof(snd))<0) {
        return -1;
    }
    return fd;
}

int
udpaddrsock(unsigned char* ipaddr,unsigned short port)
{
    int fd,opt=1;
    struct protoent* proto;
    struct sockaddr_in snd;

    if((proto=getprotobyname("udp"))==NULL) {
        return -1;
    }
    if((fd=socket(AF_INET,SOCK_DGRAM,proto->p_proto))<0) {
        return -1;
    }
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
    bzero((char*)&snd,sizeof(snd));
    snd.sin_family      = AF_INET;
    snd.sin_addr.s_addr = inet_addr((char*)ipaddr);
    snd.sin_port        = htons(port);
    if(bind(fd,(struct sockaddr*)&snd,sizeof(snd))<0) {
        return -1;
    }
    return fd;
}

#define INADDR_NONE	0

int
getsaddrbyhost(char* h,int port,struct sockaddr_in* snd)
{
    struct hostent* host;
    struct in_addr  inaddr;

    snd->sin_family = AF_INET;
    snd->sin_port   = htons(port);
    if((snd->sin_addr.s_addr=inet_addr(h))!=INADDR_NONE) {
    }
    else {
        if((host = gethostbyname(h))==NULL) {
            return -1;
        }
        snd->sin_family = host->h_addrtype;
        bcopy(host->h_addr,(caddr_t)&snd->sin_addr,host->h_length);
    }
    return 1;
}

int
in_chksum(unsigned short* s,int n)
{
    unsigned int sum = 0;
    unsigned short answer;

    while(n>1) {
        sum	+= htons(*s);
		s	+= 1;
        n	-= 2;
    }
    if(n==1) {
		sum	+= *(unsigned char*)s;
    }
    sum 	= (sum>>16)+(sum&0xffff);
    sum 	+= (sum>>16);
    answer 	= ~sum;

    return htons(answer);
}

int
sendnwait(char *s,int len,char *addr,unsigned short port,unsigned int wait)
{
	struct sockaddr_in snd;
	struct timeval tm;
	fd_set fds;
	int sec,msec,ret,nbyte;
	int fd = udpsock(0),nint;

	if(fd<=0) return -1;

    bzero((char*)&snd,sizeof(snd));
    snd.sin_family      = AF_INET;
    snd.sin_addr.s_addr = inet_addr(addr);
    snd.sin_port        = htons(port);

	FD_ZERO(&fds);
	FD_SET(fd,&fds);
	if((nbyte=sendto(fd,s,len,0,(struct sockaddr*)&snd,sizeof(snd)))<=0) {
		return -1;
	}
	sec = wait / 1000;
	msec= (wait % 1000)*1000;

	settimeout(&tm,sec,msec);
	if((ret=select(fd+1,&fds,NULL,NULL,&tm))<=0) return ret;
	if(FD_ISSET(fd,&fds)) {
		nint = sizeof(snd);
		nbyte = recvfrom(fd,s,len,0,(struct sockaddr*)&snd,&nint);
	}
	return nbyte;
}
#endif


#if defined(EXAMPLECODE)

#undef OPEN_MAX
#define OPEN_MAX	1024

int main()
{
	int fd;

	while((fd=udpsock(0))>0) printf("sockfd %d\n",fd);
	perror("sock");
}

#endif
