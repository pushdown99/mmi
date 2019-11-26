#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

#include "base64.h"

char *base64(const unsigned char *data, int len, unsigned char *buf)
{
    BIO *bmem, *b64;
    BUF_MEM *bp;

    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, data, len);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bp);

    memcpy(buf, bp->data, bp->length-1);
    buf[bp->length-1] = 0;

    BIO_free_all(b64);

    return (buf);
}

char *unbase64(const unsigned char *data, int len, unsigned char *buf)
{
    BIO *b64, *bmem;

    memset(buf, 0, len);

    b64  = BIO_new(BIO_f_base64());
    bmem = BIO_new_mem_buf((void*)data, len);
    bmem = BIO_push(b64, bmem);

    BIO_read(bmem, buf, len);

    BIO_free_all(bmem);

    return (buf);
}

