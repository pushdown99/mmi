#include <stdio.h>
#include <strings.h>
#include <stdlib.h>

#include "make_radius.h"

#define CLI_PORT 10000

int main()
{
	struct sockaddr_in sin, from_addr;
	struct timeval tout;
	fd_set fds;
	char accbuf[256],key[16],buf[256];
	int acc_len, id, type, code, opt_code, opt_len, sock, len, from_len = sizeof(from_addr);
	u_int xid, client_addr;

	if ((sock = udpsock(CLI_PORT)) == -1) {
        printf("udp socket open fail\n");
        exit(0);
    }	
	
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family      = AF_INET;
    sin.sin_addr.s_addr = inet_addr("10.120.32.3");
    sin.sin_port        = htons(ACCOUNT_PORT);

	bzero(accbuf, 256);
	bzero(buf, 256);
	bzero(key, 16);

	strcpy(key, "admin");

	printf("=========================================================================\n");
	/* make account request message - type=4 / id=1 */
	acc_len = make_account_msg(accbuf, 4, 1);

	/* add account attribute - Account Start(type=40 / code=2) */
	acc_len += add_radius_att_int(accbuf, acc_len, 40, 1);

	/* make account request authenticator */
	strcpy(&accbuf[acc_len],key);	
	finish_account_msg(accbuf, acc_len, key);
	
	/* delete key in account buffer */
	bzero(&accbuf[acc_len],256-acc_len);

	/* send to RADIUS Server */
	len = sendto(sock,accbuf,acc_len, 0,(struct sockaddr *)&sin, sizeof(sin));

	printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");	
	printf("< Accounting Request Message to RADIUS Server >\n");
	dump(accbuf,acc_len);
	printf("=========================================================================\n");

	for(;;) { /* loop */
        tout.tv_sec = 1;
        tout.tv_usec = 0;

        FD_ZERO(&fds);
        FD_SET(sock,&fds);

        if (select(FD_SETSIZE,&fds,NULL,NULL,&tout) <= 0) continue;
        if (FD_ISSET(sock,&fds)) {
            len = recvfrom(sock,buf,BUFSIZ,0,(struct sockaddr *)&from_addr,&from_len);

            if (len < 0) continue;
            sleep(1);
            printf("< Accounting Response From RADIUS Server>\n");
            dump(buf, len);
			printf("=========================================================================\n");

			return 0;
		}
	}
}
