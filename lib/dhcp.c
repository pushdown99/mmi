#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "dhcp.h"

dhcp_fsm_t dhcpfsm[]=
{
    { DHCP_INIT,        DHCP_UP,        SND_DHCPDISCOVER,   DHCP_SELECTING },
    { DHCP_SELECTING,   RCV_DHCPOFFER,  SND_DHCPREQUEST,    DHCP_REQUESTING },
    { DHCP_REQUESTING,  RCV_DHCPOFFER,  DHCP_NOP,           DHCP_REQUESTING },
    { DHCP_REQUESTING,  RCV_DHCPACK,    DHCP_LEASE,         DHCP_BOUND },
    { DHCP_BOUND,       RCV_DHCPOFFER,  DHCP_NOP,           DHCP_BOUND },
    { DHCP_BOUND,       RCV_DHCPACK,    DHCP_NOP,           DHCP_BOUND },
    { DHCP_BOUND,       RCV_DHCPNAK,    DHCP_NOP,           DHCP_BOUND },
    { DHCP_BOUND,       DHCP_T1_TMO,    SND_DHCPREQUEST,    DHCP_RENEWING },
    { DHCP_RENEWING,    RCV_DHCPACK,    DHCP_LEASE,         DHCP_BOUND },
    { DHCP_RENEWING,    RCV_DHCPNAK,    DHCP_DOWN,          DHCP_INIT },
    { DHCP_RENEWING,    DHCP_T2_TMO,    SND_DHCPREQUEST,    DHCP_REBINDING },
    { DHCP_REBINDING,   RCV_DHCPACK,    DHCP_LEASE,         DHCP_BOUND },
    { DHCP_REBINDING,   RCV_DHCPNAK,    DHCP_DOWN,          DHCP_INIT },
    { DHCP_REBINDING,   DHCP_TMO,       DHCP_DOWN,         DHCP_INIT },
    { 0,0,0,0 }
};

int get_dhcp_cli_fsm(int prev, int event, int *act, int *next)
{
	int i=0;

	for(i=0;dhcpfsm[i].prev;i++) {

		if(dhcpfsm[i].prev != prev) 	continue;
		if(dhcpfsm[i].event != event) 	continue;

		*act 	= dhcpfsm[i].act;
		*next 	= dhcpfsm[i].next;
		return i;
	}
	return -1;	/* Not Found */
}

unsigned char* get_dhcp_opt(unsigned char* op,int code)
{
    unsigned char *p;
	int type, len;

    p = (unsigned char*)op;
    while(*p!=0xFF) {
		type 	= *p++;
        if(type==0xFF) 	return NULL;
        if(!type) 		continue;
		len		= *p++;

        if(type==code) return p;
        p+=len;
    }
    return NULL;
}

int get_dhcp_msgtype(unsigned char* op)
{
    unsigned char *p;
    if(p=(unsigned char*)get_dhcp_opt(op,dhcpMessageType)) {
        return *p;
    }
    return -1;
}

int end_opt(unsigned char **op)
{
	unsigned char *tp = *op;

	while(tp[0] != 0xFF) {
		if(!tp[0]) continue;
		tp++;
		tp += tp[0];
		tp++;
	}
	*op = tp;
	return 0;
}

int add_dhcp_opt(unsigned char *op,unsigned char code,char len,char *data)
{
	end_opt(&op);
	op[0] = code;
	
	if(code == 0xFF) return 0;
	op++;
	op[0] = len;
	op++;
	memcpy(op,data,len);
	op += len;
	op[0] = 0xFF;

	return 0;
}

unsigned char *set_dhcp_opt(unsigned char* op,int code,char len,char* data)
{
    unsigned char *p;

    p = (unsigned char*)op;
    while(*p!=0xFF) {
        if(!*p) { p++; continue; }
        if(*p==0xFF) return NULL;
        if(*p==code) {
            bcopy(data,p+2,len);
            return (p+2);
        }
        p++;
        p+=*p;
        p++;
    }
    return NULL;
}

int dhcp_relay_build(
	dhcp_t *dp,
	unsigned int relay,
	unsigned int server)
{
	dp->giaddr	= relay;
	dp->hops	+=1;

	set_dhcp_opt(dp->options, dhcpServerIdentifier,	4, (char*)&server);
	return 1;
}

int dhcp_discover_build(
	dhcp_t *odp,
	unsigned int client,
	char *hostname,
	char *buf,
	int *len)
{
	dhcp_t *dp = (dhcp_t*)buf;
	int optlen=0;

	bzero(dp,sizeof(*dp));
	dp->op		= BOOTREQUEST;
	dp->htype	= ETH_10MB;
	dp->hlen	= ETH_10MB_LEN;
	dp->flags	= htons(0x8000);
	dp->xid		= odp->xid;
	
	memcpy(&dp->chaddr, odp->chaddr, 16);
	memcpy(&dp->cookie, "\x63\x82\x53\x63", 4);
	dp->options[0] = 0xFF;							/* 0xFF: end-of-options */

	add_dhcp_opt(dp->options, dhcpMessageType, 		1, (char*)"\x01");		optlen+=3;		/* DHCPDISCOVER */
	add_dhcp_opt(dp->options, dhcpClientIdentifier,	strlen(hostname), (char*)hostname);	optlen+=strlen(hostname)+2;
	add_dhcp_opt(dp->options, dhcpRequestedIPaddr,	4, (char*)&client);		optlen+=6;
	add_dhcp_opt(dp->options, hostName, strlen(hostname), (char*)hostname);	optlen+=strlen(hostname)+2;

	*len = DHCPHDRSZ + optlen+1;						/* Options are all not zero. */
	return 1;
}

int dhcp_offer_build(
	dhcp_t *odp,
	unsigned int req,
	unsigned int server,
	unsigned int subnet,
	unsigned int gateway,
	unsigned int dns,
	int lease,
	char *domain,
	char *buf, int *len)
{
	dhcp_t *dp = (dhcp_t*)buf;
	int optlen=0;
	int t1,t2;

	bzero(dp,sizeof(dhcp_t));
	dp->op		= BOOTREPLY;
	dp->htype	= ETH_10MB;
	dp->hlen	= ETH_10MB_LEN;
	dp->flags	= htons(0x8000);
	dp->xid		= odp->xid;
	dp->yiaddr	= req;
	dp->siaddr	= server;
	
	t1	= lease * 0.5;
	t2	= lease * 0.875;
	memcpy(&dp->chaddr, odp->chaddr, 16);
	memcpy(&dp->cookie, "\x63\x82\x53\x63", 4);
	dp->options[0] = 0xFF;							/* 0xFF: end-of-options */

	add_dhcp_opt(dp->options, dhcpMessageType, 		1, (char*)"\x02"); 		optlen+=3;		/* DHCPOFFER */
	add_dhcp_opt(dp->options, dhcpServerIdentifier,	4, (char*)&server); 	optlen+=6;
	add_dhcp_opt(dp->options, subnetMask, 			4, (char*)&subnet);		optlen+=6;
	add_dhcp_opt(dp->options, routersOnSubnet, 		4, (char*)&gateway);	optlen+=6;
	add_dhcp_opt(dp->options, dnsServer, 			4, (char*)&dns);		optlen+=6;
	add_dhcp_opt(dp->options, dhcpIPaddrLeaseTime, 	4, (char*)&lease); 		optlen+=6;
	add_dhcp_opt(dp->options, dhcpT1value, 	4, (char*)&t1); 		optlen+=6;
	add_dhcp_opt(dp->options, dhcpT2value, 	4, (char*)&t2); 		optlen+=6;
	add_dhcp_opt(dp->options, domainName, 	strlen(domain), (char*)domain); 		optlen+=strlen(domain);

	*len = DHCPHDRSZ + optlen+1;				/* Options are all not zero. */
	return 1;
}

int dhcp_req_build(
	dhcp_t *odp,
	unsigned int client,
	char* hostname,
	int leasetime,
	char *buf, int *len)
{
	dhcp_t *dp = (dhcp_t*)buf;
	int optlen=0;

	bzero(dp,sizeof(*dp));
	dp->op		= BOOTREQUEST;
	dp->htype	= ETH_10MB;
	dp->hlen	= ETH_10MB_LEN;
	dp->flags	= htons(0x8000);
	dp->xid		= odp->xid;
	
	memcpy(&dp->chaddr, odp->chaddr, 16);
	memcpy(&dp->cookie, "\x63\x82\x53\x63", 4);
	dp->options[0] = 0xFF;							/* 0xFF: end-of-options */

	add_dhcp_opt(dp->options, dhcpMessageType, 		1, (char*)"\x03");		optlen+=3;		/* DHCPREQUEST */
	add_dhcp_opt(dp->options, dhcpClientIdentifier,	strlen(hostname), (char*)hostname);	optlen+=strlen(hostname)+2;
	add_dhcp_opt(dp->options, dhcpRequestedIPaddr,	4, (char*)&client);		optlen+=6;
	if(leasetime>0) { add_dhcp_opt(dp->options, dhcpIPaddrLeaseTime,	4, (char*)&leasetime);	optlen+=6; }
	add_dhcp_opt(dp->options, hostName, strlen(hostname), (char*)hostname);	optlen+=strlen(hostname)+2;

	*len = DHCPHDRSZ + optlen+1;					/* Options are all not zero. */
	return 1;
}

int dhcp_ack_build(
	dhcp_t *odp,
	unsigned int req,
	unsigned int server,
	unsigned int subnet,
	unsigned int gateway,
	unsigned int dns,
	int lease,
	char *domain,
	char *buf,int *len)
{
	dhcp_t *dp = (dhcp_t*)buf;
	int optlen=0;
	int t1,t2;

	bzero(dp,sizeof(dhcp_t));
	dp->op		= BOOTREPLY;
	dp->htype	= ETH_10MB;
	dp->hlen	= ETH_10MB_LEN;
	dp->flags	= htons(0x8000);
	dp->xid		= odp->xid;
	dp->ciaddr	= (odp->ciaddr>0)? req:0;
	dp->yiaddr	= req;
	
	t1	= lease * 0.5;
	t2	= lease * 0.875;
	memcpy(&dp->chaddr, odp->chaddr, 16);
	memcpy(&dp->cookie, "\x63\x82\x53\x63", 4);
	dp->options[0] = 0xFF;							/* 0xFF: end-of-options */

	add_dhcp_opt(dp->options, dhcpMessageType, 		1, (char*)"\x05");		optlen+=3;		/* DHCPACK */
	add_dhcp_opt(dp->options, dhcpServerIdentifier,	4, (char*)&server);		optlen+=6;
	add_dhcp_opt(dp->options, subnetMask, 			4, (char*)&subnet);		optlen+=6;
	add_dhcp_opt(dp->options, routersOnSubnet, 		4, (char*)&gateway);	optlen+=6;
	add_dhcp_opt(dp->options, dnsServer, 			4, (char*)&dns);		optlen+=6;
	add_dhcp_opt(dp->options, dhcpIPaddrLeaseTime, 	4, (char*)&lease); 		optlen+=6;
	add_dhcp_opt(dp->options, dhcpT1value, 	4, (char*)&t1); 		optlen+=6;
	add_dhcp_opt(dp->options, dhcpT2value, 	4, (char*)&t2); 		optlen+=6;
	add_dhcp_opt(dp->options, domainName, 	strlen(domain), (char*)domain); 		optlen+=strlen(domain);

	*len = DHCPHDRSZ + optlen+1;						/* Options are all not zero. */
	return 1;
}

int dhcp_nak_build(
	dhcp_t *odp,
	unsigned int server,
	char *buf,int *len)
{
	dhcp_t *dp = (dhcp_t*)buf;
	int optlen=0;

	bzero(dp,sizeof(dhcp_t));
	dp->op		= BOOTREPLY;
	dp->htype	= ETH_10MB;
	dp->hlen	= ETH_10MB_LEN;
	dp->flags	= htons(0x8000);
	dp->xid		= odp->xid;
	dp->yiaddr	= odp->yiaddr;
	dp->ciaddr	= odp->ciaddr;
	
	memcpy(&dp->chaddr, odp->chaddr, 16);
	memcpy(&dp->cookie, "\x63\x82\x53\x63", 4);
	dp->options[0] = 0xFF;							/* 0xFF: end-of-options */

	add_dhcp_opt(dp->options, dhcpMessageType, 		1, (char*)"\x06");		optlen+=3;		/* DHCPNAK */
	add_dhcp_opt(dp->options, dhcpServerIdentifier,	4, (char*)&server);		optlen+=6;

	*len = DHCPHDRSZ + optlen+1;							/* Options are all not zero. */
	return 1;
}

int dhcp_release_build(
	unsigned int req,
	unsigned int relay,
	unsigned int server,
	char *hostname,
	char *mac,
	char *buf,int *len)
{
	dhcp_t *dp = (dhcp_t*)buf;
	int optlen=0;

	bzero(dp,sizeof(dhcp_t));
	dp->op		= BOOTREQUEST;
	dp->htype	= ETH_10MB;
	dp->hlen	= ETH_10MB_LEN;
	dp->flags	= htons(0x8000);
	dp->hops	= 1;
	dp->xid		= random();								/* static value ? rand() */
	dp->ciaddr	= req;
	dp->giaddr	= relay;
	
	memcpy(&dp->chaddr, mac, 16);
	memcpy(&dp->cookie, "\x63\x82\x53\x63", 4);
	dp->options[0] = 0xFF;							/* 0xFF: end-of-options */

	add_dhcp_opt(dp->options, dhcpMessageType, 		1, (char*)"\x07");		optlen+=3; 	/* DHCPRELEASE */
	add_dhcp_opt(dp->options, dhcpServerIdentifier,	4, (char*)&server);		optlen+=6; 	/* ???? */

	*len = DHCPHDRSZ + optlen+1;							/* Options are all not zero. */
	return 1;
}
