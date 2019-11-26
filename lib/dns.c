#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>
#include <string.h>
#include <strings.h>

#include "dns.h"

int dns_query(char* zone,unsigned int *addr)
{
	char buf[BUFSIZ],host[BUFSIZ];
	unsigned char *msg, *eom, *cp;
	long ttl;
	int n;
	int ancount, qdcount;
	int dlen, type, aclass, pref, weight, port;
	HEADER *hp;

	if(!zone) return 0;

	if(res_init()) { perror("res_init"); return 0; }
	n = res_query(zone,C_IN,T_A,(unsigned char *) &buf, sizeof (buf));
	if(n<=0) { perror("res_query"); return 0; }

	hp = (HEADER*)buf;
	qdcount = ntohs (hp->qdcount);
	ancount = ntohs (hp->ancount);
	msg = (unsigned char *) buf;
	eom = (unsigned char *) buf + n;
	cp = (unsigned char *) buf + sizeof (HEADER);

	while (qdcount-- > 0 && cp < eom) {
		n = dn_expand (msg, eom, cp, (char *) host, 256);
		if (n < 0) return -1;
		cp += n + QFIXEDSZ;
	}
	while (ancount-- > 0 && cp < eom) {
		n = dn_expand (msg, eom, cp, (char *) host, 256);
		if (n < 0) return -1;
		cp += n;
		NS_GET16 (type, cp);
		NS_GET16 (aclass, cp);
		NS_GET32 (ttl, cp);
		NS_GET16 (dlen, cp);

		if(type!=T_SRV) {
			*addr = *(unsigned int*)cp;
			cp += dlen;
			continue;
		}
		NS_GET16 (pref, cp);
		NS_GET16 (weight, cp);
		NS_GET16 (port, cp);
	}
	return 1;
}

#define D_TTL	5

int dns_update_build (
	char* zone,
	unsigned int addr,
	char *buf,int *len)
{
	HEADER *hp;
	char *p,*cp;
	char name[BUFSIZ],realm[BUFSIZ],hostname[BUFSIZ];

	if(!buf) return 0;
	if(!len) return 0;

	hp = (HEADER*)buf;
	bzero(hp,sizeof(*hp));
	hp->id		= rand()%0xFFFF;
	hp->qr		= 0;				/* query */
	hp->opcode	= NS_UPDATE_OP;
	hp->qdcount	= htons(1);
	hp->ancount	= 0;
	hp->nscount	= htons(1);
	hp->arcount	= 0;

	name[0] = realm[0] = 0;
    strcpy(hostname,zone);
    if(cp=strchr(hostname,'@')) {
        *cp = ' ';
        sscanf(hostname,"%s%s",name,realm);
    }
	p = (char*)(hp+1);

	cp = (char*)strtok(realm, ".");
    if (cp) {
        *p++ = strlen(cp);
        memcpy(p, cp, strlen(cp)); p+=strlen(cp);
        while(cp = (char*)strtok(NULL, ".")) {
            *p++ = strlen(cp);
            memcpy(p, cp, strlen(cp)); p+=strlen(cp);
        }
    }

    *p++    = 0x00;

    *((unsigned short*)p)++     = htons(T_SOA);
    *((unsigned short*)p)++     = htons(C_IN);

    name[0]=realm[0]=0;
    strcpy(hostname,zone);
    if(cp=strchr(hostname,'@')) {
        *cp = ' ';
        sscanf(hostname,"%s%s",name,realm);
    }

    if (name) {
        *p++ = strlen(name);
        memcpy(p, name, strlen(name)); p+=strlen(name);
    }

    *p++= 0xC0; *p++ = 0x0C;

    *((unsigned short*)p)++     = htons(T_A);
    *((unsigned short*)p)++     = htons(C_IN);
    *((unsigned int*)p)++       = htonl(D_TTL);
    *((unsigned short*)p)++     = htons(4);
    *((unsigned int*)p)++       = addr;





    return (*len=(p-buf));
}

/* DNS update (delete) */
int dns_release_build(
	char* zone,
	unsigned int addr,
	char *buf,int *len)
{
	HEADER *hp;
	char *p,*cp;
	char name[BUFSIZ],realm[BUFSIZ],hostname[BUFSIZ];

	if(!buf) return 0;
	if(!len) return 0;

	hp = (HEADER*)buf;
	bzero(hp,sizeof(*hp));
	hp->id		= rand()%0xFFFF;
	hp->qr		= 0;				/* query */
	hp->opcode	= NS_UPDATE_OP;
	hp->qdcount	= htons(1);
	hp->ancount	= 0;
	hp->nscount	= htons(1);
	hp->arcount	= 0;

	name[0] = realm[0] = 0;
    strcpy(hostname,zone);
    if(cp=strchr(hostname,'@')) {
        *cp = ' ';
        sscanf(hostname,"%s%s",name,realm);
    }
	p = (char*)(hp+1);

	cp = (char*)strtok(realm, ".");
    if (cp) {
        *p++ = strlen(cp);
        memcpy(p, cp, strlen(cp)); p+=strlen(cp);
        while(cp = (char*)strtok(NULL, ".")) {
            *p++ = strlen(cp);
            memcpy(p, cp, strlen(cp)); p+=strlen(cp);
        }
    }

    *p++    = 0x00;

    *((unsigned short*)p)++     = htons(T_SOA);
    *((unsigned short*)p)++     = htons(C_IN);

    name[0]=realm[0]=0;
    strcpy(hostname,zone);
    if(cp=strchr(hostname,'@')) {
        *cp = ' ';
        sscanf(hostname,"%s%s",name,realm);
    }

    if (name) {
        *p++ = strlen(name);
        memcpy(p, name, strlen(name)); p+=strlen(name);
    }

    *p++= 0xC0; *p++ = 0x0C;

    *((unsigned short*)p)++     = htons(T_A);
    *((unsigned short*)p)++     = htons(C_ANY);
    *((unsigned int*)p)++       = htonl(0);
    *((unsigned short*)p)++     = htons(0);





    return (*len=(p-buf));
}

void dns_hdr_prn (dns_t *dp)
{
	dnsupd_t *dpp = (dnsupd_t*)dp;
	char *buf, t;

	buf = (char*)dpp;
#ifdef Linux
	t = buf[3];
	buf[3]= buf[2];
	buf[2] = t;
#endif

	printf("id          : %d \n",dp->id);
	printf("qr          : %s \n",(dp->qr)? "response":"query");
	printf("opcode      : %s \n",
							(dp->opcode==OP_QUERY)? "standard query":
							(dp->opcode==OP_IQUERY)? "inverse query":
							(dp->opcode==OP_STATUS)? "service status query":
							(dp->opcode==OP_NOTIFY)? "notify":
							(dp->opcode==OP_UPDATE)? "update":"unknown");
	printf("flags       : \n");
	if(dp->opcode!=OP_UPDATE) {
		printf("            : %c......\n",(dp->aa)? 'a':'x');
		printf("            : .%c.....\n",(dp->tc)? 't':'x');
		printf("            : ..%c....\n",(dp->rd)? 'r':'x');
		printf("            : ...%c...\n",(dp->ra)? 'r':'x');
		printf("            : ....%c..\n",(dp->z)?  'z':'x');
		printf("            : .....%c.\n",(dp->ad)? 'a':'x');
		printf("            : ......%c\n",(dp->cd)? 'c':'x');
	}
	else {
		printf("            : %c\n",(dpp->z)? 'z':'x');
	}
	printf("rcode       : %d \n",dp->rcode);
	printf("qdcount     : %d \n",dp->qdcount);
	printf("ancount     : %d \n",dp->ancount);
	printf("nscount     : %d \n",dp->nscount);
	printf("arcount     : %d \n",dp->arcount);

#ifdef Linux
    t = buf[3];
    buf[3]= buf[2];
    buf[2] = t;
#endif

}

