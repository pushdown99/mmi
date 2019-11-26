#include <stdio.h>
#include <string.h>
#include "__ipc.h"

#define SHARED	0x50001L

int
main()
{
	char *p, *x;
	p = memget(SHARED, 10);	/* 10 byte allocation at shared memory */
	if(p) {
		strcpy(p,"Hello");
	}
	else {
		printf("Error\n");
	}

	x = memget(SHARED, 10);
	printf("%s \n", x);
	memfree(SHARED);
}

