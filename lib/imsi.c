#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#ifdef _USE_STDARG
#include <stdarg.h>
#endif
/************************************************
 *  name : imsi2str
 *  parameter : char *imsi, char *strimsi
 *  return value : int
 *  use : IMSI decoding to string type
 *        ex) IMSI decoding from BSC in AIWU
 ************************************************/
int
imsi2str(char *imsi, char *str)
{
	unsigned int	D1, D2, D3, D4, D5, D6;
	unsigned int	d1, d2, d3;
	unsigned int	dec;
	unsigned char	th;
	int		len;

/* first digit */
	bcopy(&imsi[0], (char *)&dec, 4);
	dec = ((dec >> 18) & 0x3ff);
	d1 = (dec/100);
	d2 = (dec/10)%10;
	d3 = (dec/1)%10;

	d1 = (d1*100) + 100;
	if (d1 == 1000) d1 = 0;
	d2 = (d2*10) + 10;
	if (d2 == 100) d2 = 0;
	d3 = (d3*1) + 1;
	if (d3 == 10) d3 = 0;
	D1 = d1 + d2 + d3;

/* second digit */
	bcopy(&imsi[1], (char *)&dec, 4);
	dec = ((dec >> 19) & 0x7f);
	d1 = (dec/10);
	d2 = (dec/1)%10;

	d1 = (d1*10) + 10;
	if (d1 == 100) d1 = 0;
	d2 = (d2*1) + 1;
	if (d2 == 10) d2 = 0;
	D2 = d1 + d2;

/* third digit */
	bcopy(&imsi[2], (char *)&dec, 4);
	dec = ((dec >> 17) & 0x3ff);
	d1 = (dec/100);
	d2 = (dec/10)%10;
	d3 = (dec/1)%10;

	d1 = (d1*100) + 100;
	if (d1 == 1000) d1 = 0;
	d2 = (d2*10) + 10;
	if (d2 == 100) d2 = 0;
	d3 = (d3*1) + 1;
	if (d3 == 10) d3 = 0;
	D3 = d1 + d2 + d3;

/* fourth digit */	
	dec = 0;
	bcopy(&imsi[3], (char *)&dec, 4);
	dec = ((dec >> 15) & 0x3ff);
	
	d1 = (dec/100);
	d2 = (dec/10)%10;
	d3 = (dec/1)%10;

	d1 = (d1*100) + 100;
	if (d1 == 1000) d1 = 0;
	d2 = (d2*10) + 10;
	if (d2 == 100) d2 = 0;
	d3 = (d3*1) + 1;
	if (d3 == 10) d3 = 0;
	D4 = d1 + d2 + d3;

/* fifth digit */
	dec = 0;
	dec = imsi[5];
	D5 = (dec >> 3) & 0x0f;
	if (D3 == 10) D3 = 0;

/* sixth digit */
	dec = 0;
	bcopy(&imsi[3], (char *)&dec, 4);
	dec = ((dec >> 1) & 0x3ff);
	
	d1 = (dec/100);
	d2 = (dec/10)%10;
	d3 = (dec/1)%10;

	d1 = d1*100 + 100;
	if (d1 == 1000) d1 = 0;
	d2 = d2*10 + 10;
	if (d2 == 100) d2 = 0;
	d3 = d3*1 + 1;
	if (d3 == 10) d3 = 0;
	D6 = d1 + d2 + d3;

	sprintf(str, "%03d%02d%03d%03d%d%03d", D1, D2, D3, D4, D5, D6);
	len = strlen(str);
	
	return(len);
}

