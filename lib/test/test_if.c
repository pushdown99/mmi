#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "__if.h"

#if defined(Linux)
int
main()
{
	int i;

    getifs();

    for(i=0;ifs[i].fld[IF_NAME_FLD][0];i++) {
        printf("%s: %s/%d %s (%02x:%02x:%02x:%02x:%02x:%02x) speed:%d mode:%d stat:%d\n"
            ,ifs[i].fld[IF_NAME_FLD]
            ,ifs[i].fld[IF_ADDR_FLD]
            ,ifs[i].mbl
            ,ifs[i].fld[IF_BCAST_FLD]
            ,ifs[i].mac[0]
            ,ifs[i].mac[1]
            ,ifs[i].mac[2]
            ,ifs[i].mac[3]
            ,ifs[i].mac[4]
            ,ifs[i].mac[5]
			,getifspeed(ifs[i].fld[IF_NAME_FLD])
			,getifmode(ifs[i].fld[IF_NAME_FLD])
			,getifstat(ifs[i].fld[IF_NAME_FLD])
        );
    }

}
#else
main()
{
	int i;

    getifs();

    for(i=0;ifs[i].fld[IF_NAME_FLD][0];i++) {
        printf("%s: %s/%d %s (%02x:%02x:%02x:%02x:%02x:%02x)\n"
            ,ifs[i].fld[IF_NAME_FLD]
            ,ifs[i].fld[IF_ADDR_FLD]
            ,ifs[i].mbl
            ,ifs[i].fld[IF_BCAST_FLD]
            ,ifs[i].mac[0]
            ,ifs[i].mac[1]
            ,ifs[i].mac[2]
            ,ifs[i].mac[3]
            ,ifs[i].mac[4]
            ,ifs[i].mac[5]
        );
    }

}

#endif


