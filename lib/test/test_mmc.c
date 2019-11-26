#include <stdio.h>

#include "mmc.h"

char trademark[]={
    "--------------------------------------------------\n"
    "MMC (Man-Machine Command) Program                 \n"
    "Copyright(C) 2015 MNL Solution CO., LTD.          \n"
    "                                                  \n"
    "                       http://www.mnlsolution.com \n"
    "--------------------------------------------------\n"
};

extern mmc_t _mmc[];

int
main()
{
	char buf[BUFSIZ],mml[BUFSIZ],*p;
	int prev,line,cont;

	regmmc(_mmc);

    /* Initialize */
    system("clear");
    printf("%s",trademark);

    /* MML */
    bzero(buf,BUFSIZ);
    printf("\rPress the <tab> key at any time for completions.\n\n");

    for(line=0,cont=1;;cont=1) {
        /* Command Line (Prompt) */
        printf("MMC:%d # %s",line,buf);
        mmcgets(buf);
        if((p=strchr(buf,'\n'))!=NULL) { *p=0; cont=0;  }
        if((p=strchr(buf,'\t'))!=NULL) { *p=' ';        }
        if(strlen(buf)<=0) continue;

        bzero(mml,BUFSIZ);
        strcpy(mml,buf); prev=line;
        line=pparse(buf,cont);

		if(!cont) bzero(buf,BUFSIZ);
	}
	return 1;
}

