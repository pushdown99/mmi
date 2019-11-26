#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef _USE_STDARG
#include <stdarg.h>
#endif

/************************************************
 *  name : h2int_1byte
 *  parameter : char
 *  return value : int
 *  use : 1byte hexa value(char) ==> integer
 *        ex) mac address parsing
 ************************************************/
int
h2int_1byte(char hex)
{
    char    ch[4];

    bzero(ch,4);
    sprintf(ch,"%c",hex);

    switch(*ch) {
        case 'a' :
            return 10;
        case 'b' :
            return 11;
        case 'c' :
            return 12;
        case 'd' :
            return 13;
        case 'e' :
            return 14;
        case 'f' :
            return 15;
        default :
            return atoi(ch);
    }
}

/************************************************
 *  name : h2int_2byte
 *  parameter : char *
 *  return value : int
 *  use : 2byte hexa value(char) ==> integer
 *  ex) mac address parsing
 ************************************************/
int
h2int_2byte(char *hex)
{
    char    val[4];
    int     result = 0;

    bzero(val,4);

    bcopy(hex,val,2);
    val[2] = 0;

    result = (16*(h2int_1byte(val[0])))+(1*(h2int_1byte(val[1])));

    return result;
}


/************************************************
 *  name : mac_parse
 *  parameter : char *mac, unsigned char* macint
 *  return value : int
 *  use : mac address string ==> mac address integer array
 *  concerned fuction : h2int_1byte, h2int_1byte
 ************************************************/
int
mac_parse(char *mac, unsigned char* macint)
{
    char token[8][16],tmp[4];
    char *p,*q;
    int i,num=1;

    bzero(tmp,4); bzero(token,8*16);

    q=(char*)strtok(mac,":");
    if (strlen(q) == 1) {
		bzero(tmp, 4);
		sprintf(tmp, "0%s",q);
        bcopy(tmp,token[0],strlen(tmp));
    }
    else bcopy(q,token[0],strlen(q));

    while((p = (char*)strtok(NULL,":")) != NULL){
        if(strlen(p) == 1) {
            bzero(tmp,4);
            sprintf(tmp,"0%s",p);
            bcopy(tmp,token[num],strlen(tmp));
        }
        else bcopy(p,token[num],strlen(p));
        num++;
    }
    for(i=0;i<6;i++) {
        *macint = h2int_2byte(token[i]);
        macint++;
    }
    return 1;
}
