#include <stdio.h>

int
main()
{
	char s[BUFSIZ];

	s[0] = 0x0a;
	s[1] = 0x00;
	s[2] = 0x00;
	s[3] = 0x00;
	s[4] = 0x5a;
	s[5] = 0x5a;
	s[6] = 0x5a;
	s[7] = 0x5a;
	s[8] = 0x72;

	printf("csum = %04x \n",htons(in_chksum(s,9)));
}

