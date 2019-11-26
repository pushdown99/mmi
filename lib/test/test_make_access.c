#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "make_radius.h"
#include "md5.h"

#define CLI_PORT 10000
#define IDENTIFY 1
#define MD5_CHALLENGE 4

int send_access_req(int sock,struct sockaddr_in sin,char *accbuf,char *key,int id,char *challenge,char *state, int state_len);

int main()
{
	struct sockaddr_in sin, from_addr;
	struct timeval tout;
	fd_set fds;
	char accbuf[256],key[16],buf[256],temp[16],name[64],eap[64], signal[16];
	char challenge[64], state[64];
	int acc_len, type, opt_code, opt_len, sock, len, from_len = sizeof(from_addr), id, state_len, eap_len;
	u_int addr;
	rad_t *radhdr;

	if ((sock = udpsock(CLI_PORT)) == -1) {
        printf("udp socket open fail\n");
        exit(0);
    }	
	
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family      = AF_INET;
    //sin.sin_addr.s_addr = inet_addr("11.0.0.3");
    //sin.sin_port        = htons(ACCESS_PORT);
    sin.sin_addr.s_addr = inet_addr("11.0.0.13");
    sin.sin_port        = htons(54321);

	bzero(accbuf, 256);
	bzero(buf, 256);
	bzero(key, 16);
	bzero(temp, 16);
	bzero(name, 16);
	bzero(eap, 64);
	bzero(signal, 16);
	bzero(challenge, 64);
	bzero(state, 64);

	strcpy(key, "admin");

	printf("=========================================================================\n");
	get_authenticator(temp);	/* 16byte random number */

	/* make access request message - Access Request (type=1 / id =1) */
	acc_len = make_access_msg(accbuf, 1, 1,temp);

	/* add access attribute User-Nmae (type = 1) */
	strcpy(name,"0162709100");
	acc_len += add_radius_att_string(accbuf, acc_len, 1, strlen(name)+2, name);

	/* add access attribute NAS-IP-Address (type = 4) */
	addr = htonl(inet_addr("10.120.31.2"));
	acc_len += add_radius_att_int(accbuf, acc_len, 4, addr);

	/* add access attribute NAS-Port (type = 5) */
	acc_len += add_radius_att_int(accbuf, acc_len, 5, 1);

	/* add access attribute Service-Type (type = 6) */
	acc_len += add_radius_att_int(accbuf, acc_len, 6, 2);

	/* add access attribute Framed-MTU (type = 12) */
	acc_len += add_radius_att_int(accbuf, acc_len, 12, 2304);

	/* add access attribute NAS-Port-Type (type = 61) */
	acc_len += add_radius_att_int(accbuf, acc_len, 61, 19);

	/* add access attribute Connect-Info (type = 77) */
	strcpy(name,"CONNECT 11Mbps 802.11b");
	acc_len += add_radius_att_string(accbuf, acc_len, 77, strlen(name)+2, name);

	/* add access attribute EAP-Message (type = 79) */
	strcpy(name,"0162709100");
	eap_len = make_eap_msg(eap, 2, 1, strlen(name), IDENTIFY, name);
	acc_len += add_radius_att_string(accbuf, acc_len, 79, eap_len + 2, eap);

	/* add access attribute Message_Authenticator (type = 80) */
	acc_len += add_radius_att_string(accbuf, acc_len, 80, 18, signal);

	/* finish access msg with eap */
	finish_access_eap(accbuf, acc_len, key, signal);

	/* send to RADIUS Server */
	len = sendto(sock,accbuf,acc_len, 0,(struct sockaddr *)&sin, sizeof(sin));

	printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
	printf("< Access Request #1 to RADIUS Server >\n");
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

			radhdr = (struct rad *)buf;
			if(radhdr->code == 11){
				printf("< Access Challenge From RADIUS Server >\n");
				dump(buf, len);
				printf("=========================================================================\n");

				id = get_challenge(buf, challenge);
				state_len = get_state(buf, state);
				send_access_req(sock,sin, accbuf, key, id, challenge, state, state_len);
			}
			else if(radhdr->code == 2){
				printf("< Access Accept From RADIUS Server >\n");
				dump(buf, len);
				printf("=========================================================================\n");
				return 0;
			}
			else if(radhdr->code == 3){
				printf("< Access Reject From RADIUS Server >\n");
				dump(buf, len);
				printf("=========================================================================\n");
				return 0;
			}
			else{
            	printf("< ??? From RADIUS Server>\n");
            	dump(buf, len);
				printf("=========================================================================\n");
				return 0;
			}
		}
	}
}

int send_access_req(int sock,struct sockaddr_in sin,char *accbuf,char *key,int id,char *challenge,char *state, int state_len)
{
    char temp[16],name[64],eap[64], signal[16],digest[16],req_challenge[64];
    int acc_len, type, opt_code, opt_len, len, eap_len;
    u_int addr;
    rad_t *radhdr;

	bzero(temp, 16);
	bzero(name, 64);
	bzero(eap, 64);
	bzero(signal, 16);
	bzero(digest, 16);
	bzero(req_challenge, 64);
	
    get_authenticator(temp);
	/* make access request message - Access Request (type=1 / id=2) */
    acc_len = make_access_msg(accbuf, 1, 2,temp);

    /* add access attribute User-Nmae (type = 1) */
    strcpy(name,"0162709100");
    acc_len += add_radius_att_string(accbuf, acc_len, 1, strlen(name)+2, name);

    /* add access attribute NAS-IP-Address (type = 4) */
    addr = inet_addr("10.120.31.2");
    acc_len += add_radius_att_int(accbuf, acc_len, 4, addr);

    /* add access attribute NAS-Port (type = 5) */
    acc_len += add_radius_att_int(accbuf, acc_len, 5, 1);

    /* add access attribute Service-Type (type = 6) */
    acc_len += add_radius_att_int(accbuf, acc_len, 6, 2);

    /* add access attribute Framed-MTU (type = 12) */
    acc_len += add_radius_att_int(accbuf, acc_len, 12, 2304);

	/* add access attribute State (type = 24) */
	acc_len += add_radius_att_string(accbuf, acc_len, 24, state_len, state);

    /* add access attribute NAS-Port-Type (type = 61) */
    acc_len += add_radius_att_int(accbuf, acc_len, 61, 19);

    /* add access attribute EAP-Message (type = 79) */
	strcpy(name,"0162709100");
	make_req_challenge(req_challenge, id, key, challenge, digest);
    eap_len = make_req_eap_msg(eap, 2, id, strlen(name), MD5_CHALLENGE, digest, name);
    acc_len += add_radius_att_string(accbuf, acc_len, 79, eap_len+2, eap);

    /* add access attribute Message_Authenticator (type = 80) */
    acc_len += add_radius_att_string(accbuf, acc_len, 80, 18, signal);

    /* finish access msg with eap */
    finish_access_eap(accbuf, acc_len, key, signal);

    /* send to RADIUS Server */
    len = sendto(sock,accbuf,acc_len, 0,(struct sockaddr *)&sin, sizeof(sin));

	printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
    printf("\n< Access Request #2 to RADIUS Server>\n");
    dump(accbuf,acc_len);
	printf("=========================================================================\n");

	return 0;
}
