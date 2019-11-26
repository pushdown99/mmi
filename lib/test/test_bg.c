#include <stdio.h>
#include <signal.h>

#include "proc.h"

int
main()
{
	printf("Runs jobs in the background\n");
	signal(SIGHUP,SIG_IGN);
	background();

	sleep(10);
	printf("Exit.\n");
}

