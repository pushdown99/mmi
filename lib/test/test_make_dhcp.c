#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "make_dhcp.h"

int send_dhcp_req(int sock, struct sockaddr_in sin, char *dhbuf, u_int xid, char *chaddr, int yiaddr);

int main()
{
	struct sockaddr_in sin, from_addr;
	struct timeval tout;
	fd_set	fds;
	char buf[512],dhbuf[512], chaddr[16], temp[16], id[7];
	int dh_len, opt_code, opt_len, opt_type, sock, len, from_len = sizeof(from_addr);
	u_int xid, client_addr;
	dhcp_t *dhcphdr;
	dhcpopt_type_t *dhcpopt;

	if ((sock = udpsock(DHCP_CLIENT_PORT)) == -1) {
        printf("udp socket open fail\n");
        exit(0);
    }	
	
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family      = AF_INET;
    sin.sin_addr.s_addr = inet_addr("10.120.32.3");
    sin.sin_port        = htons(DHCP_SERVER_PORT);

	bzero(buf, 512);
	bzero(dhbuf,512);
	bzero(chaddr,16);

	xid = make_random_number();

	chaddr[0] = 0;
	chaddr[1] = 2;
	chaddr[2] = 85;
	chaddr[3] = 198;
	chaddr[4] = 143;
	chaddr[5] = 206;

	dh_len = make_dhcp_msg(dhbuf,xid,0,chaddr);

	/* add option(TYPE) - code 53(DHCP Message Type) */	
	dh_len += add_dhcp_option_type(dhbuf, dh_len, 53,1,1);

	/* add option(INT) - code 50(DHCP Client Address) */
	client_addr = inet_addr("192.168.1.62");
	dh_len += add_dhcp_option_int(dhbuf, dh_len, 50,4,client_addr);

	/* add option(STRING) - code 12(Host Name) */
	strcpy(temp,"WEMS");
	dh_len += add_dhcp_option_string(dhbuf, dh_len, 12,strlen(temp),temp);

	/*************************/

	/* send to RADIUS Server */
	len = sendto(sock,dhbuf,dh_len, 0,(struct sockaddr *)&sin, sizeof(sin));
	printf("%d\n",len);

	printf("\n< DHCP DISCOVER to Server >\n");
	dump(dhbuf,dh_len);

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
            len = recvfrom(sock,buf,BUFSIZ,0,(struct sockaddr *)&from_addr,&from_len);

printf("DDDDDD\n");
            if (len < 0) continue;

			sleep(1);
			printf("< From DHCP Server>\n");

			dhcphdr = (struct dhcpformat *)buf;
			dhcpopt = (struct dhcpopt_type *)(buf + sizeof(dhcp_t));

			if(dhcpopt->code == 53 && dhcpopt->type == 2){
				printf("< DHCP OFFER from Server >\n");
				dump(buf, len);

				/* Make DHCP REQUEST && Send to Server */
				send_dhcp_req(sock, sin, dhbuf,xid, chaddr,dhcphdr->yiaddr);  
			}
			else if(dhcpopt->type == 5){
				printf("< DHCP ACK from Server >\n");
				dump(buf, len);
				return 0;
			}
			else if(dhcpopt->type == 6){
				printf("< DHCP NACK from Server >\n");
				dump(buf, len);
				return 0;
			}
			else{
				printf("< Who Are You??? >\n");
				dump(buf, len);
				return 0;
			}
        }
    }
}

int send_dhcp_req(int sock, struct sockaddr_in sin, char *dhbuf,u_int xid, char *chaddr, int yiaddr)
{
    struct timeval tout;
    fd_set  fds;
    char temp[16];
    int dh_len, opt_code, opt_len, len;
	uint32_t addr;

	dh_len = make_dhcp_msg(dhbuf,xid,yiaddr,chaddr);

	/* add option(TYPE) - code 53(DHCP Message Type) */
    dh_len += add_dhcp_option_type(dhbuf, dh_len, 53,1,3);

	/* add option(INT) - code 50(DHCP Client Address) */
    addr = yiaddr;
    dh_len += add_dhcp_option_int(dhbuf, dh_len, 50,4,addr);

    /* add option(STRING) - code 12(Host Name) */
    strcpy(temp,"wag");
    dh_len += add_dhcp_option_string(dhbuf, dh_len, 12,strlen(temp),temp);

	/*************************/

    /* send to RADIUS Server */
    len = sendto(sock,dhbuf,dh_len, 0,(struct sockaddr *)&sin, sizeof(sin));

    printf("\n< DHCP REQUEST to Server >\n");
    dump(dhbuf,dh_len);

	return 0;
}

