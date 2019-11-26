/* 
 * Author : hwang-hae-yeun <hyhwang@softteleware.com>
 *   Copyright 2004 hwang-hae-yeun
 *   
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#include "__ipc.h"
#include "ipp.h"
#include "bufq.h"

/* local memory or shared memory not allocated. */
ipplist_t* ipplist[MAXIPPLIST] = {NULL, }; 

int		ipptype=0;	/* ippool allocation method	*/
/* already shared memory allocated. */
ipplist_t* shmpre_ipplist = NULL;


int ippadd(char* netaddr, int nbit,char* xsaddr,char* xeaddr)
{
	return ippaddn(0, netaddr, nbit, xsaddr, xeaddr);
}

int ippaddn(int index, char* netaddr, int nbit,char* xsaddr,char* xeaddr)
{
	unsigned int mask,temp;
	unsigned int net,isip,ieip,xsip,xeip;
	int total,mutexcl;
	int i,j;
	ipp_t* ipp;

	index %= MAXIPPLIST;
	if(!ipplist[index]) {
		if (ipptype & IPP_PRESHARE) {
			ipplist[index] = (ipplist_t*)(shmpre_ipplist+index);
		} else if (ipptype & IPP_SHARE) {
			ipplist[index] = (ipplist_t*)memget(
				IPP_KEY+index, sizeof(ipplist_t)*MAXIPPLIST);
		} else {
			ipplist[index] = (ipplist_t*)malloc(sizeof(ipplist_t));
		}
		ensure(ipplist[index], return 0);

		((ipplist_t*)ipplist[index])->nipp = 0;/* number of ippool	*/
		((ipplist_t*)ipplist[index])->cipp = 0;/* current ippool pointer */

		bzero(&(ipplist[index])->ipp[0],sizeof(ipp_t)*MAXIPP);
	}
	ipp = ipplist[index]->ipp;

	/* Exception handle */
	ensure(netaddr, return 0);
	if(nbit>32) return 0;

	/* Pre-Processin' 	*/
	if(nbit>30) mask=0xFFFFFFFF;
	else {
		for(i=0,mask=0xFFFFFFFF;i<32-nbit;i++) mask=mask<<1;
	}
	net		= inet_addr(netaddr);
	xsip	= (xsaddr)? inet_addr(xsaddr):0;
	xeip	= (xeaddr)? inet_addr(xeaddr):0;
	net		= htonl(net) & mask;
	xsip	= htonl(xsip);
	xeip	= htonl(xeip);

	/* range check for exclusive ipp */
	if(xsip>xeip) {	/* swap */
		temp = xsip;
		xsip = xeip;
		xeip = temp;
	}

	total 	= ~mask-1;
	if(total<0)	total=1;

	if(total>1) {
		isip	= net+1;
		ieip	= (net|~mask)-1;
	}
	else {
		isip	= net;
		ieip	= net;
	}
	/* 
	 * 	isip		xsip		xeip		ieip
	 *   |           |           |           |
	 *	----------------------------------------
	 */
	if(xeip && xeip<isip) xsip = xeip = 0;
	if(xsip && isip>xsip) xsip = isip;
	if(xeip && ieip<xeip) xeip = ieip;

#if defined(DEBUG)
	printf("net 	: %08x\n",net);
	printf("mask 	: %08x\n",mask);
	printf("isip 	: %08x\n",isip);
	printf("ieip 	: %08x\n",ieip);
	printf("xsip 	: %08x\n",xsip);
	printf("xeip 	: %08x\n",xeip);
#endif

	for(i=0;i<MAXIPP;i++) {
		if(ipp[i].net) continue;
		ipp[i].net	= net;
		ipp[i].mask	= mask;
		ipp[i].nbit	= nbit;
		ipp[i].isip	= isip;
		ipp[i].ieip	= ieip;
		ipp[i].xsip	= xsip;
		ipp[i].xeip	= xeip;

		mutexcl	= (xsip && xeip)? (xeip-xsip)+1:0;

		ipp[i].cur		= 0;
		ipp[i].total	= total - mutexcl;
		ipp[i].end		= total;
		ipp[i].used		= 0;
		ipp[i].free		= 0;
		
		if(mutexcl) {
			for(j=ipp[i].xsip;j<=ipp[i].xeip;j++) {
				IPPSET((j-ipp[i].isip),&ipp[i]);
			}
		}
		ipplist[index]->nipp +=1;
		break;
	}
}

int ippdel(char* netaddr, int nbit)
{
	unsigned int net,mask;
	int i, j, nipp;
	ipp_t* ipp;

	/* Exception handle */
	ensure(netaddr, return 0);
	if(nbit>32) return 0;

	for(i=0,mask=0xFFFFFFFF;i<32-nbit;i++) mask=mask<<1;
	net		= inet_addr(netaddr);
	net		= htonl(net) & mask;

	for(i=0; i<MAXIPPLIST; i++) {
		if (!ipplist[i]) continue;
		nipp = ipplist[i]->nipp;
		ipp = ipplist[i]->ipp;
		for(j=0;j<MAXIPP;j++) {
			if(ipp[j].net!=net) continue;
			bzero(&ipp[j],sizeof(ipp_t));
			nipp -=1;
		}
	}
}

void ippflush()
{
	int i;
	for (i=0; i<MAXIPPLIST; i++) {
		if (ipplist[i]) {
			if (ipptype & IPP_PRESHARE) {
				/* do nothing */
			} else if (ipptype & IPP_SHARE) {
				memfree(IPP_KEY+i);
			} else {
				free(ipplist[i]);
			}
			ipplist[i] = NULL;
		}
	}
}

unsigned int ipalloc()
{
	unsigned int addr,skip;
	int nipp, cipp;
	ipp_t* ipp;

	ensure(ipplist[0], return 0);

	nipp = ipplist[0]->nipp;
	cipp = ipplist[0]->cipp;
	ipp = ipplist[0]->ipp;

	if(ipptype & IPP_ROUND) cipp = (cipp+1)%nipp;

	for(skip=0;skip<nipp;skip++,cipp=(cipp+1)%nipp) {
		if(ipp[cipp].total<=ipp[cipp].used) continue;

		if(ipptype & IPP_SEQ) ipp[cipp].cur = 0;

		while(IPPISSET(ipp[cipp].cur,&ipp[cipp])) {
			if(ipp[cipp].xsip && ipp[cipp].xeip
				&& ipp[cipp].cur>=(ipp[cipp].xsip-ipp[cipp].isip)
				&& ipp[cipp].cur<=(ipp[cipp].xeip-ipp[cipp].isip)) 
				ipp[cipp].cur = ipp[cipp].xeip-ipp[cipp].isip;

			ipp[cipp].cur=(ipp[cipp].cur+1)%ipp[cipp].end;
		}

		addr = ipp[cipp].cur+ipp[cipp].isip;
		IPPSET(ipp[cipp].cur,&ipp[cipp]);

		ipp[cipp].used	+= 1;
		ipp[cipp].cur 	= (ipp[cipp].cur+1)%ipp[cipp].end;

		return htonl(addr);
	}
	return 0;
}

/* sangjae 2005.01.12 */
int ipallocn(char *s, int index)
{
	unsigned int addr,skip;
	int nipp, cipp;
	ipp_t* ipp;
	_ipp_t *temp;

	ensure(ipplist[index], return 0);

	nipp = ipplist[index]->nipp;
	cipp = ipplist[index]->cipp;
	ipp = ipplist[index]->ipp;

	if(ipptype & IPP_ROUND) cipp = (cipp+1)%nipp;

	for(skip=0;skip<nipp;skip++,cipp=(cipp+1)%nipp) {
		if(ipp[cipp].total<=ipp[cipp].used) continue;

		if(ipptype & IPP_SEQ) ipp[cipp].cur = 0;

		while(IPPISSET(ipp[cipp].cur,&ipp[cipp])) {
			if(ipp[cipp].xsip && ipp[cipp].xeip
				&& ipp[cipp].cur>=(ipp[cipp].xsip-ipp[cipp].isip)
				&& ipp[cipp].cur<=(ipp[cipp].xeip-ipp[cipp].isip)) 
				ipp[cipp].cur = ipp[cipp].xeip-ipp[cipp].isip;

			ipp[cipp].cur=(ipp[cipp].cur+1)%ipp[cipp].end;
		}

		addr = ipp[cipp].cur+ipp[cipp].isip;
		IPPSET(ipp[cipp].cur,&ipp[cipp]);

		ipp[cipp].used	+= 1;
		ipp[cipp].cur 	= (ipp[cipp].cur+1)%ipp[cipp].end;

		temp = (_ipp_t*)s;	
		temp->addr = htonl(addr);
		temp->index = cipp;

		return 1;
	}
	return 0;
}

void ipfree(unsigned int addr)
{
	unsigned int ip;
	int i, j, nipp;
	ipp_t* ipp;

	ip = htonl(addr);
	for (i=0; i<MAXIPPLIST; i++) {
		if (!ipplist[i]) continue;
		nipp = ipplist[i]->nipp;
		ipp = ipplist[i]->ipp;
		for(j=0;j<nipp;j++) {
			if(ipp[j].isip>ip || ipp[j].ieip<ip) continue;

			if(IPPISSET(ip-ipp[j].isip,&ipp[j])) {
				ipp[j].used -= 1;
			}
			IPPCLR(ip-ipp[j].isip,&ipp[j]);
			return;
		}
	}
}

int ipset(unsigned int addr)
{
	unsigned int ip;
	int i, j, nipp;
	ipp_t* ipp;

	ip = htonl(addr);
	for (i=0; i<MAXIPPLIST; i++) {
		if (!ipplist[i]) continue;
		nipp = ipplist[i]->nipp;
		ipp = ipplist[i]->ipp;
		for(j=0;j<nipp;j++) {
			if(ipp[j].isip>ip || ipp[j].ieip<ip) continue;

			if(!IPPISSET(ip-ipp[j].isip,&ipp[j])) {
				IPPSET(ip-ipp[j].isip,&ipp[j]);
				ipp[j].used += 1;
				return 1;
			}
			return -1;
		}
	}
	return -1;
}

int ipptotal()
{
	int i,j,tot, nipp;
	ipp_t* ipp;

	for (i=0; i<MAXIPPLIST; i++) {
		if (!ipplist[i]) continue;
		nipp = ipplist[i]->nipp;
		ipp = ipplist[i]->ipp;
		for(j=0,tot=0;j<nipp;j++) tot += ipp[j].total;
	}
	return tot;
}

int ipptotaln(int n)
{
	int i,tot, nipp;
	ipp_t* ipp;

	if (!ipplist[n]) return 0;
	nipp = ipplist[n]->nipp;
	ipp = ipplist[n]->ipp;
	for(i=0,tot=0;i<nipp;i++) tot += ipp[i].total;
	return tot;
}

int ippused()
{
	int i,j,tot, nipp;
	ipp_t* ipp;

	for (i=0; i<MAXIPPLIST; i++) {
		if (!ipplist[i]) continue;
		nipp = ipplist[i]->nipp;
		ipp = ipplist[i]->ipp;
		for(j=0,tot=0;j<nipp;j++) tot += ipp[j].used;
	}
	return tot;
}

int ippusedn(int n)
{
	int i,tot, nipp;
	ipp_t* ipp;

	if (!ipplist[n]) return 0;
	nipp = ipplist[n]->nipp;
	ipp = ipplist[n]->ipp;
	for(i=0,tot=0;i<nipp;i++) tot += ipp[i].used;
	return tot;
}

int ippusednet(int n,unsigned int network,int nbit)
{
	int i,tot, nipp, net;
	ipp_t* ipp;

	net = htonl(network);
	if (!ipplist[n]) return 0;
	nipp = ipplist[n]->nipp;
	ipp = ipplist[n]->ipp;
	for(i=0,tot=0;i<nipp;i++) {
		if(ipp[i].net != net ) continue;
		if(ipp[i].nbit!= nbit)	continue;
		return ipp[i].used;
	}
	return 0;
}

int ippfree()
{
	int i,j,tot, nipp;
	ipp_t* ipp;

	for (i=0; i<MAXIPPLIST; i++) {
		if (!ipplist[i]) continue;
		nipp = ipplist[i]->nipp;
		ipp = ipplist[i]->ipp;
		for(j=0,tot=0;j<nipp;j++) tot += ipp[j].free;
	}
	return tot;
}
		
#if defined(EXAMPLECODE)

int
main()
{
	char 	ip1[20],ip2[20];
	int 	nb1,nb2;
	int 	i;
	int		d1,d2,d3,d4,d5,d6,d7,d8;

	strcpy(ip1,"192.168.1.0");
	strcpy(ip2,"192.168.2.0");
	nb1 = 28;
	nb2 = 28;

#if 1
	ipptype = IPP_ROUND;
#endif
	ippaddn(1, ip1,nb1,"192.168.1.3","192.168.1.13");
	ippaddn(2, ip2,nb2,"192.168.2.3","192.168.2.13");

	d1 = ipallocn(1);
	d2 = ipallocn(2);
	d3 = ipallocn(1);
	d4 = ipallocn(2);
	d5 = ipallocn(1);
	d6 = ipallocn(2);
	d7 = ipallocn(1);
	d8 = ipallocn(1);

	printf("A: ip1 %x \n",htonl(d1));
	printf("A: ip2 %x \n",htonl(d2));
	printf("A: ip3 %x \n",htonl(d3));
	printf("A: ip4 %x \n",htonl(d4));
	printf("A: ip5 %x \n",htonl(d5));
	printf("A: ip6 %x \n",htonl(d6));
	printf("A: ip7 %x \n",htonl(d7));
	printf("A: ip8 %x \n",htonl(d8));

	ipfree(d1);
	ipfree(d4);

	printf("F: ip1 %x \n",htonl(d1));
	printf("F: ip4 %x \n",htonl(d4));

	d7 = ipallocn(1);
	d8 = ipallocn(2);

	printf("A: ip7 %x \n",htonl(d7));
	printf("A: ip8 %x \n",htonl(d8));

	ippflush();
}

#endif
