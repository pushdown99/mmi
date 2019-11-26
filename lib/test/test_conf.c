#include <stdio.h>

#include "conf.h"

#define MYCONF "./my.conf"

int
main()
{
	char val[BUFSIZ];

	if(getconf(MYCONF,"E-MAIL",val)<0) {
		printf("getconf: read fail \n");
		return 0;
	}
	printf("E-MAIL : %s \n",val);
}


