#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <netinet/in.h>

char* BASE64encode(char* s,int size,int *len)
{
	char *es = (char*)0;
	int i=0,j=0;
	int cnt=0;
	int ch;
	int base64cnt = 0;

	if(!s) return NULL;
	es = (char*)malloc(sizeof(char)*(size*2));

	while(1) {
		switch(cnt++) {
		case 0	:
			if(i<size) ch=(s[i]&0xFC)>>2;
			else ch=-1;
			break;
		case 1	:
			if(i<size) 
				if(i+1<size) ch=((s[i]&0x03)<<4)|((s[i+1]&0xF0)>>4);
				else ch=((s[i]&0x03)<<4);
			else ch=-1;
			i++;
			break;
		case 2	:
			if(i<size)
				if(i+1<size) ch=((s[i]&0x0F)<<2)|((s[i+1]&0xC0)>>6);
				else ch=((s[i]&0x0F)<<2);
			else ch=-1;
			i++;
			break;
		case 3	:
			if(i<size) ch=(s[i]&0x3F);
			else ch=-1;
			i++;
			cnt=0;
			break;
		}
		if(ch>=0&&ch<=25) es[j++]='A'+ch;
		else if(ch>=26&&ch<=51) es[j++]='a'+ch-26;
		else if(ch>=52&&ch<=61) es[j++]='0'+ch-52;
		else if(ch==62) es[j++]='+';
		else if(ch==63) es[j++]='/';
		else if(ch==-1) es[j++]='=';

		base64cnt++;

#define BASE64_SIZE	64

		if(!(j%4)) {
			if(base64cnt == BASE64_SIZE)
				base64cnt = 0, es[j++]=0x0A;
			if(i>=size) break;
		}
	}
	es[j]=0;
	if(len) *len=j;

	return es;
}

char* BASE64decode(char* es,int *len)
{
	char *ds;
	long btmp=0;
	int i=0,j=0;
	int cnt=0;
	int padcnt=0;

	ds=(char*)malloc(sizeof(char)*strlen(es));
	if(!ds) return NULL;

	while(es[i]) {
		if(isupper(es[i])) btmp=(btmp<<6)|(es[i]-'A');
		else if(islower(es[i])) btmp=(btmp<<6)|(es[i]-'a'+0x1A);
		else if(isdigit(es[i])) btmp=(btmp<<6)|(es[i]-'0'+0x34);
		else if(es[i]=='+') btmp=(btmp<<6)|0x3E;
		else if(es[i]=='/') btmp=(btmp<<6)|0x3F;
		else if(es[i]=='=') padcnt++, btmp=(btmp<<6);
		else btmp=(btmp<<6);

		if(++cnt>=4) {
			ds[j++]=(char)((btmp&0x00FF0000)>>16);
			ds[j++]=(char)((btmp&0x0000FF00)>>8);
			ds[j++]=(char)(btmp&0x000000FF);

			cnt=0;
			btmp=0;

			if(es[i+1]==0x0A) i++;
		}
		i++;
	}
	ds[j-padcnt]=0;
	if(len) *len=j-padcnt;

	return ds;
}

uint64_t
long2bcd(uint64_t l)
{
	uint64_t bcd = 0;
    char buf[BUFSIZ],*p,tmp;
    int len,i;

    p = (char*)&bcd;

    sprintf(buf,"%lu",l);
    len = strlen(buf);
    for(i=0;i<16-len;i++) {
		buf[len+i]=15+'0';
    }
    for(i=0;i<8;i++) {
		tmp = buf[0+(2*i)];
		buf[0+(2*i)] = buf[1+(2*i)];
		buf[1+(2*i)] = tmp;
    }
    for(i=0;i<16;i++) {
		if(i%2) p[i/2] |= buf[i]-'0';
		else p[i/2] = (buf[i]-'0')<<4;
    }
    return bcd;
}


#if 0
int
main()
{
	char s[BUFSIZ],es[BUFSIZ],ds[BUFSIZ];
	int len;

	strcpy(s,"Hello! Mr.Monkey");
	printf("Original: %s (%d bytes)\n",s,strlen(s));

	sprintf(es,"%s",BASE64encode(s,strlen(s),&len));
	printf("Encode	: %s (%d bytes)\n",es,len);

	sprintf(ds,"%s",BASE64decode(es,&len));
	printf("Decode	: %s (%d bytes)\n",ds,len);
}
#endif


