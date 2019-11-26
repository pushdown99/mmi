#include <stdio.h>

#include "mib2.h"

int
main()
{
    char dat[BUFSIZ],*p;
    int nbyte,i,n;
#ifdef SunOS
	mib2_ifEntry_t *mp;

	while(1) {
    	nbyte = get_mib2_if(dat,BUFSIZ);
    	prn_mib2_if(dat,nbyte);

		n = nbyte/sizeof(mib2_ifEntry_t);
		for(i=0;i<n;i++) {
			mp = (mib2_ifEntry_t*)&dat[i*sizeof(mib2_ifEntry_t)];
			p = &dat[i*sizeof(mib2_ifEntry_t)+100];
			printf("%d: %d %d \n"
				,i*sizeof(mib2_ifEntry_t)+100
				,mp->ifInOctets
				,*(int*)p);
		}
	}
#endif

#ifdef Linux
	printf("\nLinux Version Interface Stat\n");
	nbyte = get_mib2_if(dat, BUFSIZ);	
	prn_mib2_if(dat, nbyte);
#endif


}

