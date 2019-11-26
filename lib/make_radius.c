#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "md5.h"
#include "make_radius.h"

int make_md5(char *buf, int len, char *digest);

int make_access_msg(char *buf,int type,int id,char *temp)
{
	rad_t *radhdr;

	radhdr = (struct rad *)buf;
	radhdr->code = type;
	radhdr->id = id;
	memcpy(radhdr->authenticator, temp, 16);	

	return sizeof(rad_t); 
}

int make_account_msg(char *buf,int type,int id)
{
	rad_t *radhdr;

	radhdr = (struct rad *)buf;
	radhdr->code = type;
	radhdr->id = id;

	return sizeof(rad_t); 
}

int finish_access_msg(char *buf, int len)
{
	rad_t *radhdr;

	radhdr = (struct rad *)buf;
	radhdr->length = htons(len);

	return 1;
}

int finish_account_msg(char *buf, int len, char *key)
{
	rad_t *radhdr;
	char digest[16];

	radhdr = (struct rad *)buf;
	radhdr->length = htons(len);

	make_md5((char*)buf, len+strlen(key), (char*)digest);
	memcpy(radhdr->authenticator, digest, 16);	

	return 1;
}

int get_challenge(char *buf, char *challenge)
{
	rad_t *radhdr;
	att_t *atthdr;
	find_challenge_t *ph;
	int i, len, id;

	radhdr = (struct rad *)buf;
	len = radhdr->length;
	for(i=sizeof(rad_t);i<len; ){
		atthdr = (struct att *)(buf + i);

		if(atthdr->type == 79){
			ph = (struct find_challenge *)(buf + i + 2);
			id = ph->id;
			memcpy(challenge,ph->value,ph->value_size);

			return id;
		}
		else{
			i += atthdr->att_length;
		}
	}	
	return id;
}

int get_state(char *buf, char *state)
{
    rad_t *radhdr;
    att_t *atthdr;
    int i, len;

    radhdr = (struct rad *)buf;
    len = radhdr->length;
    for(i=sizeof(rad_t);i<len; ){
        atthdr = (struct att *)(buf + i);
        if(atthdr->type == 24){
            memcpy(state, atthdr->temp, atthdr->att_length - 2);

			return atthdr->att_length;
        }
        else{
            i += atthdr->att_length;
        }
    }
}

int make_req_challenge(char *buf, int id, char *key, char *challenge, char *digest)
{
	make_challenge_t *mch;

	mch = (struct make_challenge *)buf;
	mch->id = id;
	memcpy(mch->data, key, strlen(key));
	memcpy(&buf[1+strlen(key)], challenge, 16); 
	
	make_md5((char*)buf, 1+strlen(key)+16, digest); 

	return 1;
}

int get_authenticator(char *authenticator)
{
    int i;

    for(i = 0; i < 16; i++)
        *authenticator++ = (char)(mrand48() * 0xff);
    return 1;
}

int make_md5(char *buf, int len, char *digest)
{
	MD5_CTX context;

    MD5Init(&context);
    MD5Update(&context, buf, len);
    MD5Final(digest, &context);

    return 1;
}

int add_radius_att_string(char *buf, int len, int type, int length, char *tmpbuf)
{
	att_t *atthdr;

	atthdr = (struct att *)(buf+len);
	atthdr->type = type;
	atthdr->att_length = length;
	memcpy(atthdr->temp, tmpbuf, length-2);

	return length;
}

int add_radius_att_int(char *buf, int len, int type, uint32_t addr)
{
	att_addr_t *attaddr;

	attaddr = (struct att_addr *)(buf+len);
	attaddr->type = type;
	attaddr->att_length = 6;
	attaddr->address = htonl(addr);

	return 6;
}

int add_radius_att_vendor_string(char *buf,int len,uint32_t v_id,int v_type,int v_len,char *tempbuf)
{
	vendor_string_t *vt;

	vt = (struct vendor_string *)(buf+len);
	vt->type = 26;
	vt->att_length = 8+v_len;
	vt->vendor_id = htonl(v_id);
	vt->vendor_type = v_type;
	vt->vendor_length = 2+v_len;
	memcpy(vt->vendor_value, tempbuf, v_len);
	
	return 8+v_len;
}

int add_radius_att_vendor_int(char *buf,int len,uint32_t v_id,int v_type,uint32_t addr)
{
	vendor_int_t *vt;

	vt = (struct vendor_int *)(buf+len);
	vt->type = 26;
	vt->att_length = 12;
	vt->vendor_id = htonl(v_id);
	vt->vendor_type = v_type;
	vt->vendor_length = 6;
	vt->vendor_value = htonl(addr);
	
	return 12;
}

int make_eap_msg(char *buf, int code, int id, int length, int type, char *value)
{
	eap_t *e;
	int i;
	
	e = (struct eap *)buf;
	e->code = code;
	e->id = id;
	e->length = htons(5+length);
	e->type = type;
	memcpy(e->value, value, length);

	return (5+length);
}

int make_req_eap_msg(char *buf, int code, int id, int length, int type, char *value, char *name)
{
	find_challenge_t *e;

	e = (struct find_challenge *)buf;
	e->code = code;
	e->id = id;
	e->length = htons(6+16+length);
	e->type = type;
	e->value_size = 16;
	memcpy(e->value, value, 16);
	memcpy(e->name, name, strlen(name));

	return (6+16+length);
}

int finish_access_eap(char *buf, int len, char *key, char *digest)
{
	rad_t *radhdr;
	att_t *atthdr;

	radhdr = (struct rad *)buf;
	radhdr->length = htons(len);
	
	atthdr = (struct att *)(buf + len-18);
	hmac_md5((char*)buf, len, key, strlen(key), digest);
	memcpy(atthdr->temp, digest, 16);

	return 1; 
}

