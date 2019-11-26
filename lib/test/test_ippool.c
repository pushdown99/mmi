#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ipp.h"

int
main()
{
#define MAX_LOOP 	1000
    char    buf[16],ip1[20],ip2[20],ip3[20],ip4[20],ip5[20];
    int     nb1,nb2;
    int     i,m,n;
    int     d[MAX_LOOP];
	int 	at,as;
	int 	ft,fs;
	_ipp_t *_temp;

	bzero(buf,16);
    strcpy(ip1,"10.0.0.0");
    strcpy(ip2,"11.0.0.0");
    strcpy(ip3,"0.0.0.0");
    strcpy(ip4,"0.0.0.0");
    strcpy(ip5,"12.0.0.0");
    nb1 = 29;
    nb2 = 28;

	shmpre_ipplist = NULL;
#if 1
    ipptype = IPP_ROUND|IPP_SHARE;	/* round-robin between ippool resource | shared memory	*/
#endif
    //ipptype = IPP_ROUND;	/* round-robin between ippool resource | shared memory	*/

	printf("============================\n");
	printf("IPPOOL allocation/free demo \n");
	printf(" round[%c] \n",(ipptype&IPP_ROUND)? 'v':' ');
	printf(" share[%c] 		 \n",(ipptype&IPP_SHARE)? 'v':' ');
	printf(" preshare[%c] 	 \n",(ipptype&IPP_PRESHARE)? 'v':' ');
	printf("----------------------------\n");
	printf("pool[1] %s/%d\n",ip1,nb1);
	printf("pool[2] %s/%d\n",ip2,nb2);
	printf("pool[3] %s/%d\n",ip5,nb2);
	printf("============================\n\n");

	/* exclusive ippool range is not set */
    ippaddn(1,ip1,nb1,ip3,ip4);
    ippaddn(1,ip2,nb2,ip3,ip4);
    ippaddn(1,ip5,nb1,ip3,ip4);

	bzero(d, MAX_LOOP * sizeof(int));

#if 0
	for(i=0;i<MAX_LOOP;i++)  {
		_temp = (_ipp_t*)ipallocn(1);
		
		if(_temp){
			printf(" [%d][%d] ip-addr = %s \n", i,_temp->index,inet_ntoa(*(struct in_addr*)&_temp->addr));
		//	ipfree(d[i]);
		}

	}
#endif

	ipset(inet_addr("10.0.0.1"));

	ipallocn(buf,1);
	_temp = (_ipp_t*)buf;
	
	if(_temp){
		printf(" [%d][%d] \n", i,_temp->index);
		printf(" [%d][%d] ip-addr = %s \n", i,_temp->index,inet_ntoa(*(struct in_addr*)&_temp->addr));
	//	ipfree(d[i]);
	}
	printf("\n");
	printf("=======================\n");
	printf("Total : %d \n",ipptotal());
	printf("Used  : %d \n",ippused());
	printf("Free  : %d \n",ippfree());
	printf("=======================\n\n");
	printf("Random %d/%d allocation\n",as,at);
	printf("Random %d/%d free\n",fs,ft);
    printf("=======================\n\n");

    printf("total1: %d \n",ippusednet(1,inet_addr(ip1),nb1));
    printf("total2: %d \n",ippusednet(1,inet_addr(ip2),nb2));
    printf("total3: %d \n",ippusednet(1,inet_addr(ip5),nb1));

    ippflush();
}

