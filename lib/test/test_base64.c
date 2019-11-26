#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#if 0
int
main(int argc,char *argv[])
{
	char s[BUFSIZ],es[BUFSIZ],ds[BUFSIZ];
	int len;

	if(argc<2) {
		printf("Usage: %s {string} \n",argv[0]);
		return 0;
	}
	strcpy(s,argv[1]);

	printf("Original: %s (%d bytes)\n",s,strlen(s));
	sprintf(es,"%s",BASE64encode(s,strlen(s),&len));
	printf("Encode  : %s (%d bytes)\n",es,len);
	sprintf(ds,"%s",BASE64decode(es,&len));
	printf("Decode  : %s (%d bytes)\n",ds,len);
}
#endif

#define BUFSIZE	20000
int
main(int argc,char *argv[])
{
	char s[BUFSIZE],es[BUFSIZE],ds[BUFSIZE];
	struct stat b;
	FILE *fp;
	int len;

	if(argc<2) {
		printf("Usage: %s {file} \n",argv[0]);
		return 0;
	}
	if(stat(argv[1], &b)) {
		printf("stat error \n");
		exit(0);
	}

	bzero(s, BUFSIZE);
	if(!(fp=fopen(argv[1], "r"))) {
		printf("file open error \n");
		exit(0);
	}
	fread(s, 1, b.st_size, fp);
	fclose(fp);

	printf("%s",BASE64decode(s,&len));
}
