#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>

int
main(int argc, char *argv[])
{
	unsigned char		macint[6];
	int			i;

	if (argc != 2) {
		printf("usage : ./test_mac_parse {mac address}\n");
		exit(0);
	}

	bzero(macint,6);

	mac_parse(argv[1],macint);

	for (i=0; i<6; i++) {
		printf("macint[%d]=%x\n",i,macint[i]);
	}
}

