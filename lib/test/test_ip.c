#include <stdio.h>

int
main()
{
    char dat[BUFSIZ];
    int nbyte;

    nbyte = get_mib2_ip(dat,BUFSIZ);
    prn_mib2_ip(dat, nbyte);

	dump(dat,nbyte);
#if 0
    nbyte = get_mib2_if(dat,BUFSIZ);
    prn_mib2_if(dat,nbyte);
#endif
}

