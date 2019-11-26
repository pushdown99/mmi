#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "md5.h"

#include "make_rrq.h"

int make_presuf_md5(char *buf, int len, char *digest, char *key);

int rrq_msg_generator(char *buf,char bits,int lifetime,unsigned int home_address,unsigned int home_agent,unsigned int coa,unsigned int high,unsigned int low)
{
	int k;
	request_t *r;

	/* make rrq packet */
	r = (struct req_format *)buf;
	r->type = 1;
	r->bits = bits;
	r->lifetime = lifetime;
	r->home_address = home_address;
	r->home_agent = home_agent;
	r->coa = coa;
	r->id_high = htonl(high);
	r->id_low = htonl(low);

	return sizeof(request_t);
}

int add_rrq_extension(char *buf,int len, char type, int spi, char *key)
{
	extension_t *e;
	char digest[16];

	bzero(digest,16);

	e = (struct extension *)(buf+len);
	e->type = type;
	e->spi = htonl(spi);
	e->length = 20;

	make_presuf_md5(buf, len+6, digest, key);
	memcpy(e->authenticator, digest, 16);
	
	return sizeof(extension_t);
}

int add_nai_extension(char *buf, int len, int length, char *mn_nai)
{
	nai_t *nai_ext;

	nai_ext = (struct nai *)(buf+len);
	nai_ext->type = 131;
	nai_ext->length = length;
	memcpy(nai_ext->mn_nai, mn_nai,length);

	return length+2;
}

int add_udp_tunnel_request(char *buf, int len, int f, int r, int encap)
{
	udp_request_t *ureq;

	ureq = (struct udp_request*)(buf+len);
	ureq->type = 144;
	ureq->length = 6;
	ureq->sub_type = 0;
#if 0
	ureq->f = f;
	ureq->r = r;
#endif
	ureq->flag = 0;
	if(f) ureq->flag += 128;
	if(r) ureq->flag += 64;
	ureq->encapsulation = encap;
	ureq->reserved3 = 0;

	return 8;	
}

int add_general_mip_ext(char *buf, int len, int subtype, int spi, char *key)
{
	char authentication[16];
	general_t *g;

	bzero(authentication,16);

	g = (struct general *)(buf+len);
	g->type = 36;
	g->subtype = subtype;
	g->length = htons(20);
	g->spi = htonl(spi);

	hmac_md5((char*)buf, len+8, key, strlen(key), authentication);
	memcpy(g->auth, authentication, 16);

	return 24;
}

int add_my_ext(char *buf, int len, int type, int length, char *temp_buf)
{
	my_ext_t *mt;

	mt = (struct my_ext *)(buf+len);
	mt->type = type;
	mt->length = length;
	memcpy(mt->buf, temp_buf, length);

	return length+2;
}

int add_myint_ext(char *buf, int len, int type, int length, unsigned int value)
{
	myint_ext_t *mt;

	mt = (struct myint_ext *)(buf+len);
	mt->type = type;
	mt->length = length;
	mt->value = value;

	return length+2;
}

int make_presuf_md5(char *buf, int len, char *digest, char *key)
{
    MD5_CTX     context;

    MD5Init (&context);
    MD5Update (&context, key, strlen(key));
    MD5Update (&context, buf, len);
    MD5Update (&context, key, strlen(key));
    MD5Final (digest, &context);

    return 1;
}
