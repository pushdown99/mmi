#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>

int
dumphex(char *s,int len)
{
	int i;
	for(i=0;i<len;i++) {
		if(i>0 && !(i % 4)) printf(" ");
		if(i>0 && !(i % 16)) printf("\n");
		printf("%02x",(unsigned char)s[i]); fflush(stdout);
	}
}

#define WIDTH   16
int
dump(char *s,int len)
{
    char buf[BUFSIZ],lbuf[BUFSIZ],rbuf[BUFSIZ];
    unsigned char *p;
    int line,i;

    p =(unsigned char *) s;
    for(line = 1; len > 0; len -= WIDTH,line++) {
        memset(lbuf,0,BUFSIZ);
        memset(rbuf,0,BUFSIZ);

        for(i = 0; i < WIDTH && len > i; i++,p++) {
            sprintf(buf,"%02x ",(unsigned char) *p);
            strcat(lbuf,buf);
            sprintf(buf,"%c",(!iscntrl(*p) && *p <= 0x7f) ? *p : '.');
            strcat(rbuf,buf);
        }
        printf("%04x: %-*s    %s\n",line - 1,WIDTH * 3,lbuf,rbuf);
    }
    if(!(len%16)) printf("\n");
    return line;
}

int
__dump(char *s,int len,char *sp)
{
    char buf[BUFSIZ],lbuf[BUFSIZ],rbuf[BUFSIZ];
    unsigned char *p;
    int line,i;

    p =(unsigned char *) s;
    for(line = 1; len > 0; len -= WIDTH,line++) {
        memset(lbuf,0,BUFSIZ);
        memset(rbuf,0,BUFSIZ);

        for(i = 0; i < WIDTH && len > i; i++,p++) {
            sprintf(buf,"%02x ",(unsigned char) *p);
            strcat(lbuf,buf);
            sprintf(buf,"%c",(!iscntrl(*p) && *p <= 0x7f) ? *p : '.');
            strcat(rbuf,buf);
        }
        printf("%s%04x: %-*s    %s\n",sp,line - 1,WIDTH * 3,lbuf,rbuf);
    }
    if(!(len%16)) printf("\n");
    return line;
}

