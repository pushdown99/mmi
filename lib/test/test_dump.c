#include <stdio.h>
#include <strings.h>

int main()
{
	char buf[BUFSIZ];

	strcpy(buf,"Hello, world\n");
	dump(buf,strlen(buf));
}

