#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <sys/poll.h>

#ifdef SunOS

#include <fcntl.h>
#include <stropts.h>
#include <sys/types.h>
#include <kvm.h>
#include <sys/fcntl.h>
#include <kstat.h>
#include <errno.h>
#include <time.h>

#include <sys/sockio.h>
#include <sys/socket.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/tiuser.h>
#include <inet/common.h>
#include <inet/common.h>
#include <inet/mib2.h>
#include <inet/ip.h>
#include <net/if.h>
#include <netinet/in.h>

#include "mib2.h"

#define Line printf("%s:%d \n",__FILE__,__LINE__)

void prn_mib2_ip(char *s)
{
	char *p = s;
	
	printf("ipForwarding            : %d \n",*((unsigned int*)p)++);
	printf("ipDefaultTTL            : %d \n",*((unsigned int*)p)++);
	printf("ipInReceives            : %d \n",*((unsigned int*)p)++);
	printf("ipInHdrErrors           : %d \n",*((unsigned int*)p)++);
	printf("ipInAddrErrors          : %d \n",*((unsigned int*)p)++);
	printf("ipForwDatagrams         : %d \n",*((unsigned int*)p)++);
	printf("ipInUnknownProtos       : %d \n",*((unsigned int*)p)++);
	printf("ipInDiscards            : %d \n",*((unsigned int*)p)++);
	printf("ipInDelivers            : %d \n",*((unsigned int*)p)++);
	printf("ipOutRequests           : %d \n",*((unsigned int*)p)++);
	printf("ipOutDiscards           : %d \n",*((unsigned int*)p)++);
	printf("ipOutNoRoutes           : %d \n",*((unsigned int*)p)++);
	printf("ipReasmTimeout          : %d \n",*((unsigned int*)p)++);
	printf("ipReasmReqds            : %d \n",*((unsigned int*)p)++);
	printf("ipReasmOKs              : %d \n",*((unsigned int*)p)++);
	printf("ipReasmFails            : %d \n",*((unsigned int*)p)++);
	printf("ipFragOKs               : %d \n",*((unsigned int*)p)++);
	printf("ipFragFails             : %d \n",*((unsigned int*)p)++);
	printf("ipFragCreates           : %d \n",*((unsigned int*)p)++);
	printf("ipAddrEntrySize         : %d \n",*((unsigned int*)p)++);
	printf("ipRouteEntrySize        : %d \n",*((unsigned int*)p)++);
	printf("ipNetToMediaEntrySize   : %d \n",*((unsigned int*)p)++);
	printf("ipRoutingDiscards       : %d \n",*((unsigned int*)p)++);
}

int get_mib2_ip(char *dat,int maxlen)
{
	struct strbuf ctrl,data;
	char ctl[BUFSIZ];
	struct T_optmgmt_req *tr = (struct T_optmgmt_req *)ctl;
	struct T_optmgmt_ack *ta = (struct T_optmgmt_ack *)ctl;
	struct opthdr *req;
	int fd,size=0,flags=0;

	if((fd=open("/dev/ip",O_RDWR))<=0) return 0;

	ctrl.buf	= ctl;
	ctrl.maxlen	= BUFSIZ;

	tr->PRIM_type	= T_OPTMGMT_REQ;
	tr->OPT_offset 	= sizeof(struct T_optmgmt_req);
	tr->OPT_length 	= sizeof(struct opthdr);
	tr->MGMT_flags 	= T_CURRENT;

	req = (struct opthdr*)(tr+1);
	req->level 		= MIB2_IP;
	req->name 		= 0;
	req->len 		= 0;
	ctrl.len 		= tr->OPT_length+tr->OPT_offset;

	if(putmsg(fd,&ctrl,NULL,0)) goto mib2_ip_fail;

    while (1) {
        struct pollfd pfd[1];

        pfd[0].fd       = fd;
        pfd[0].events   = POLLIN | POLLPRI;
        pfd[0].revents  = 0;

        switch (poll(pfd,1,0)) {
        case -1 :
            break;
        case 0  :
			break;
        case 1  :
            if (pfd[0].revents & (POLLIN | POLLPRI | POLLERR | POLLHUP)) {
				ctrl.maxlen = BUFSIZ;
				ctrl.len    = 0;
				ctrl.buf    = (char*)ctl;
				data.maxlen = maxlen;
				data.len    = 0;
				data.buf    = (char*)dat;

				flags = 0;
				req = (struct opthdr*)(ta+1);
				if(getmsg(fd, &ctrl, &data, &flags)<0) goto mib2_ip_fail;
				if(ctrl.len < sizeof(struct T_optmgmt_ack)
					|| (ta->PRIM_type != T_OPTMGMT_ACK)
					|| (ta->MGMT_flags != T_SUCCESS)) break;
				if(req->level != MIB2_IP || req->name != 0) break;
				size = data.len;
				goto mib2_ip_succ;
			}
		}
	}

mib2_ip_fail:
	ioctl(fd,I_FLUSH,FLUSHRW);
	close(fd);
	return 0;

mib2_ip_succ:
	ioctl(fd,I_FLUSH,FLUSHRW);
	close(fd);
	return size;
}

#endif


#ifdef Linux

#define IP_STATS_PREFIX_LEN 4
#define IP_STATS_LINE  "Ip: %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu"

int get_mib2_ip(char *dat, int maxlen)
{
	char *p=dat;
    unsigned long d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13,d14,d15,d16,d17,d18,d19;
    FILE *fp;
    char line[1024];

//logprn(0, 0, 0, "(STI-LIB:ip.c) func=get_mib2_ip() start\n");

    if(!(fp = fopen("/proc/net/snmp", "r"))) {
//logprn(0, 0, 0, "(STI-LIB:ip.c) func=get_mib2_ip() fopen fail\n");
        return 0;
    }

    while(line==fgets(line, sizeof(line), fp)) {
	    if(!strncmp(line, IP_STATS_LINE, IP_STATS_PREFIX_LEN)) {
   		    sscanf(line, IP_STATS_LINE
				,&d1,&d2,&d3,&d4,&d5,&d6,&d7,&d8,&d9,&d10,&d11,&d12,&d13,&d14,&d15,&d16,&d17,&d18,&d19
   		     );
    	 }
    }

	*((unsigned long *)p)		= d1;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d2;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d3;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d4;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d5;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d6;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d7;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d8;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d9;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d10;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d11;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d12;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d13;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d14;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d15;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d16;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d17;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d18;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= d19;	p+=sizeof(unsigned long);
	*((unsigned long *)p)		= 0;	p+=sizeof(unsigned long); 	/* routing discard	*/

    fclose(fp);

//logprn(0, 0, 0, "(STI-LIB:ip.c) func=get_mib2_ip() end \n");
    return 1;
}

#endif

void prn_mib2_ip(char *s)
{
	char *p = s;
	
	printf("ipForwarding            : %lu \n",(*((unsigned long*)p))++);
	printf("ipDefaultTTL            : %lu \n",(*((unsigned long*)p))++);
	printf("ipInReceives            : %lu \n",(*((unsigned long*)p))++);
	printf("ipInHdrErrors           : %lu \n",(*((unsigned long*)p))++);
	printf("ipInAddrErrors          : %lu \n",(*((unsigned long*)p))++);
	printf("ipForwDatagrams         : %lu \n",(*((unsigned long*)p))++);
	printf("ipInUnknownProtos       : %lu \n",(*((unsigned long*)p))++);
	printf("ipInDiscards            : %lu \n",(*((unsigned long*)p))++);
	printf("ipInDelivers            : %lu \n",(*((unsigned long*)p))++);
	printf("ipOutRequests           : %lu \n",(*((unsigned long*)p))++);
	printf("ipOutDiscards           : %lu \n",(*((unsigned long*)p))++);
	printf("ipOutNoRoutes           : %lu \n",(*((unsigned long*)p))++);
	printf("ipReasmTimeout          : %lu \n",(*((unsigned long*)p))++);
	printf("ipReasmReqds            : %lu \n",(*((unsigned long*)p))++);
	printf("ipReasmOKs              : %lu \n",(*((unsigned long*)p))++);
	printf("ipReasmFails            : %lu \n",(*((unsigned long*)p))++);
	printf("ipFragOKs               : %lu \n",(*((unsigned long*)p))++);
	printf("ipFragFails             : %lu \n",(*((unsigned long*)p))++);
	printf("ipFragCreates           : %lu \n",(*((unsigned long*)p))++);
	printf("ipAddrEntrySize         : %lu \n",(*((unsigned long*)p))++);
	printf("ipRouteEntrySize        : %lu \n",(*((unsigned long*)p))++);
	printf("ipNetToMediaEntrySize   : %lu \n",(*((unsigned long*)p))++);
	printf("ipRoutingDiscards       : %lu \n",(*((unsigned long*)p))++);
}


#if defined(STANDALONE)

int
main()
{
	char dat[BUFSIZ];
	int nbyte;

	nbyte = get_mib2_ip(dat,BUFSIZ);
	prn_mib2_ip(dat);

	nbyte = get_mib2_if(dat,BUFSIZ);
	prn_mib2_if(dat,nbyte);
}

#endif
