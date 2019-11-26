#include <stdio.h>

int
main()
{
	char buf[BUFSIZ];

	fmttm("%Y/%m/%d %H:%M:%S.%t",buf);
	printf("%s \n",buf);

	fmttm("%Y/%m/%d %H:%M:%S",buf);
	printf("%s \n",buf);

	fmttm("%y/%m/%d %H:%M:%S",buf);
	printf("%s \n",buf);

	fmttm("%m/%d %H:%M:%S",buf);
	printf("%s \n",buf);

	fmttm("%H:%M:%S",buf);
	printf("%s \n",buf);

	fmttm("[%H:%M:%S]",buf);
	printf("%s \n",buf);
}

