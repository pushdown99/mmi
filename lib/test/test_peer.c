#include <stdio.h>

#include "peer.h"

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

int main(int argc, char *argv[])
{
    unsigned int addr,base;
    int i;

	if(argc<3) {
		printf("Usage: %s {start address} {num} \n",argv[0]);
		exit(0);
	}

    base = inet_addr(argv[1]);

    for(i=0;i<atoi(argv[2]);i++) {
        addr = htonl(ntohl(base)+i);
		printf("mgmt(peer) : %s \n",inet_ntoa(*(struct in_addr*)&addr));
        peeradd(addr);
    }

    peerupcbfn = sysup;
    peerdncbfn = sysdn;

    while(1) {
        sleep(1);
    }
}

