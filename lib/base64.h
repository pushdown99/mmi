#ifndef BASE64_H
#define BASE64_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

char *base64(const unsigned char *data, int len, unsigned char* hash);
char *unbase64(const unsigned char *data, int len, unsigned char* hash);

#endif
