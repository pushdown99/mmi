#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "make_dhcp.h"

int
make_dhcp_msg(char *buf,unsigned int xid,unsigned int yiaddr,char *tmpbuf)
{
    dhcp_t *d;

    d = (struct dhcpformat *)buf;
    d->op = 1;
    d->htype = 1;
    d->hlen = 6;
    d->hops = 0;
    d->xid = xid;
    d->flags = htons(32768);
	d->yiaddr = yiaddr;
    memcpy(d->chaddr,tmpbuf,16);
    d->magic = inet_addr("99.130.83.99");

    return sizeof(dhcp_t);
}

int
add_dhcp_option_type(char *buf, int len, int code, int length, int type)
{
    dhcpopt_type_t *opt;

    opt = (struct dhcpopt_type *)(buf+len);
    opt->code = code;
    opt->length = length;
    opt->type = type;

    return length+2;;
}

int add_dhcp_option_int(char *buf, int len, int code, int length, u_int addr)
{
    dhcpopt_int_t *opt;

    opt = (struct dhcpopt_int *)(buf+len);
    opt->code = code;
    opt->length = length;
    opt->addr = addr;

    return length+2;
}

int add_dhcp_option_string(char *buf, int len, int code, int length, char *value)
{
    dhcpopt_string_t *opt;

    opt = (struct dhcpopt_string *)(buf+len);
    opt->code = code;
    opt->length = length;
    memcpy(opt->value, value, length);

    return length+2;

}

unsigned int
make_random_number()
{
    time_t t;
    unsigned int i;

    t = time(NULL);
    srandom(t%9999);
    i = (unsigned int)random();

    return i;
}
