#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "make_rrq.h"
#include "__time.h"

#define CLI_PORT 10000
#define SVR_PORT 434

int main()
{
	struct timeval now;
	struct sockaddr_in sin, from_addr;
	struct timeval tout;
	fd_set fds;
	uint32_t home_address, home_agent, coa;
	int len, sock, spi, buf_len, nai_len, from_len=sizeof(from_addr);
	unsigned short lifetime;
	char bits, buf[256], reqbuf[256], type, *key, nai[64];
	request_t *rrqhdr;
	unsigned int high,low;

	if ((sock = udpsock(CLI_PORT)) == -1) {
        printf("udp socket open fail\n");
        exit(0);
    }

	bzero((char *)&sin, sizeof(sin));
    sin.sin_family      = AF_INET;
    sin.sin_addr.s_addr = inet_addr("10.120.32.1");
    sin.sin_port        = htons(SVR_PORT);

	bzero(reqbuf,256);
	bzero(buf,256);

	bits = 32;
	lifetime = htons(1024);
	home_address = inet_addr("192.168.1.62");
	home_agent = inet_addr("10.120.31.1");
	coa = inet_addr("10.120.31.1");
	key = "testtesttesttest";
	gettimeofday(&now,NULL);
	high = 1;
	low =now.tv_sec + JAN_1970;

	/* make mip request packet */
	buf_len = rrq_msg_generator(reqbuf,bits,lifetime, home_address, home_agent, coa,high,low);

	/* add NAI */
	strcpy(nai,"sangjae@softteleware.com");
	nai_len = strlen(nai);
	buf_len += add_nai_extension(reqbuf,buf_len,nai_len,nai);
	
    /* add mip request extension */
	type = 32;
    spi = 301;
    buf_len += add_rrq_extension(reqbuf,buf_len,type,spi,key);


	/* send to mobile agent */
	len = sendto(sock,reqbuf,buf_len,0,(struct sockaddr *)&sin, sizeof(sin));

	printf("\n< Registration Request (Fixed-Length Protion) >\n");
	dump(reqbuf,buf_len);

	for(;;) { /* loop */
printf("AAAAAA\n");
        tout.tv_sec = 1;
        tout.tv_usec = 0;

        FD_ZERO(&fds);
        FD_SET(sock,&fds);

printf("BBBBBB\n");
        if (select(FD_SETSIZE,&fds,NULL,NULL,&tout) <= 0) continue;
printf("CCCCCC\n");

        if (FD_ISSET(sock,&fds)) {
            len = recvfrom(sock,buf,256,0,(struct sockaddr *)&from_addr,&from_len);

printf("DDDDDD\n");
            if (len < 0) continue;

            sleep(1);
            printf("< From WAG >\n");
			rrqhdr = (struct req_format *)buf;

			if(rrqhdr->type == 3){
				printf("< Registration Response From WAG >\n");
			}
			else{
				printf("WHO ARE U\n");
			}

            dump(buf, len);
        }
	return 0;
	}
}
