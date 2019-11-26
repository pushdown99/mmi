#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>

int
str2int(char *strhex, unsigned char *inthex)
{
	char token[8][16],tmp[4];
	char *p,*q;
	int i,num=1;

	bzero(tmp,4); bzero(token,8*16);

	q=(char*)strtok(strhex,":");
	if (strlen(q) == 1) {
		bzero(tmp, 4);
		sprintf(tmp, "0%s",q);
		bcopy(tmp,token[0],strlen(tmp));
	}
	else bcopy(q,token[0],strlen(q));

	while((p = (char*)strtok(NULL,":")) != NULL){
		if(strlen(p) == 1) {
			bzero(tmp,4);
			sprintf(tmp,"0%s",p);
			bcopy(tmp,token[num],strlen(tmp));
		}
		else bcopy(p,token[num],strlen(p));
		num++;
	}
	for(i=0;i<7;i++) {
		*inthex = h2int_2byte(token[i]);
		inthex++;
	}
	return 1;
}

int
main(int argc, char *argv[])
{
	int		len = 0, i;
	char	min[16];
	unsigned char	inputIMSI[7];

	if (argc != 2) {
		printf("Usage : ./test_imsi {IMSI}\n");
		printf("Ex) ./test_imsi 05:77:1f:13:7b:91:a8\n");
		exit(0);
	}

	bzero(min, 16);

	str2int(argv[1], inputIMSI);
	len = imsi2str(inputIMSI, min);	

	for (i=0; i<7; i++) {
		printf("input IMSI[%d] = %x\n", i, inputIMSI[i]);
	}
	printf("\n");
	printf("output IMSI = %s\n", min);
}

