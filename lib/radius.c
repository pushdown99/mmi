#include <stdio.h>
#include <string.h>

#include "md5.h"
#include "radius.h"

int add_sattr(char *buf,int type,int len,void *val)
{
	char *cp;
	attr_t *ap = (attr_t*)buf;

	ap->type	= type;
	ap->len		= len+2;		/* 2: type(1)+len(1)	*/

	cp = (char*)(ap+1);
	bcopy(val,cp,len);

	return (ap->len);
}

int get_sattr(char *buf,int type,void *val)
{
	char *cp;
	attr_t *ap = (attr_t*)buf;

	for(;ap && ap->type; ap=(attr_t*)cp) {
		cp = (char*)ap + ap->len;
		if(ap->type != type) continue;

		cp = (char*)(ap+1);
		bcopy(cp,val,ap->len-2);
		return 1;	
	}
	return 0;
}

int add_radius_vattr(char *buf,int type,int len,void *val)
{
	char *cp;
	vend_attr_t *vp;
	attr_t *ap = (attr_t*)buf;

	ap->type	= AT_3GPP2;
	ap->len		= len+8;		/* 8: type(1)+len(1)+vendor[id(4)+type(1)+len(1)]	*/

	vp = (vend_attr_t*)(ap+1);
	vp->id		= VID_3GPP2;
	vp->type	= type;
	vp->len		= len;
	cp = (char*)(vp+1);
	bcopy(val,cp,len);

	return (ap->len);
}

int radius_auth_check(char *auth, char *secret,unsigned char *buf,int len)
{
	MD5_CTX context;
	char *cp;
	char tmp[BUFSIZ],digest[16];
	radius_t *rp = (radius_t*)buf;

	/* request/reply  authenticator check
	 * ==================================
	 *	(request) 	MD5(code + id + length  +16 zero octets + request attributes + secret)
	 *	(reply) 	MD5(code + id + length + request authenticator + request attributes + secret)
	 */
	cp = (char*)(rp+1);	/* offset 20 bytes */

	bcopy(rp,		tmp,sizeof(*rp));
	bcopy(cp,		tmp+sizeof(*rp),len-sizeof(*rp));
	bcopy(secret,	tmp+len, strlen(secret));

	MD5Init(&context);
	MD5Update(&context,tmp,len+strlen(secret));
	MD5Final(digest,&context);

	return (!bcmp(digest,auth,16))? 1:0;
}

int radius_account_request_build (
	int id,
	int type,
	char *secret,
	rattr_t *ap,
	char *buf,int *len)
{
	MD5_CTX context;
	int n,nbyte;
	char digest[16];
	char *cp;
	rattr_t *app;
	radius_t *rp = (radius_t*)buf;
	
	rp->code	= PW_ACCOUNTING_REQUEST;
	rp->id		= id;

	cp = (char*)(rp+1);

	/* set attribute -- radius */
	for(nbyte=sizeof(*rp),app=ap;app && app->type;app+=1) {
		n = add_sattr(cp, app->type, app->len, app->val);
		nbyte += n; cp += n;
	}
	bcopy(secret,cp,strlen(secret));
	rp->len = nbyte;
	
	MD5Init(&context);
	MD5Update(&context,rp,nbyte+strlen(secret));
	MD5Final(digest,&context);

	bcopy(digest, rp->auth, 16);

	*len = rp->len;
	return 1;
}

int radius_account_parse (
	char *buf,
	rattr_t *ap
	)
{
	char *cp;
	rattr_t *app;
	radius_t *rp = (radius_t*)buf;

	cp = (char*)(rp+1);

	/* get attribute -- radius */
	for(app=ap;app && app->type;app+=1) {
		get_sattr(cp, app->type, app->val);
	}
	return 1;
}

void radius_get_auth(char *auth)
{
    int i;

    for(i = 0; i < 16; i++)
        *auth++ = (char)(drand48() * 0xff);
}

int radius_access_request_build (
	int id,
	char *secret,
	rattr_t *ap,
	char *buf,int *len)
{
	MD5_CTX context;
	int n,nbyte;
	char digest[16],auth[16];
	char *cp;
	rattr_t *app;
	radius_t *rp = (radius_t*)buf;
	
	rp->code	= PW_ACCESS_REQUEST;
	rp->id		= id;

	radius_get_auth(auth);
	bcopy(auth, rp->auth, 16);

	cp = (char*)(rp+1);

	/* set attribute -- radius */
	for(nbyte=sizeof(*rp),app=ap;app && app->type;app+=1) {
		n = add_sattr(cp, app->type, app->len, app->val);
		nbyte += n; cp += n;
	}
	bcopy(secret,cp,strlen(secret));
	rp->len = nbyte;
	
	MD5Init(&context);
	MD5Update(&context,rp,nbyte+strlen(secret));
	MD5Final(digest,&context);

	bcopy(digest, rp->auth, 16);

	*len = rp->len;
	return 1;
}

