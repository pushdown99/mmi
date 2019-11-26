#include <stdio.h>
#include <strings.h>

#ifdef SunOS
#include <kstat.h>
#include <sys/cpuvar.h>

int getmemuse(unsigned long *total,unsigned long *avail)
{
	kstat_t *ks;
	kstat_ctl_t *kc = NULL;
	kstat_named_t *kn;
	unsigned long avlmem, maxmem; /*, pagesize;	*/
	float	pagesize;	

	if(!(kc=kstat_open())) 	return 0;
	ks=kstat_lookup(kc,"unix",0,"system_pages");
	if(kstat_read(kc,ks,0)<0) return 0;

	maxmem 		= sysconf(_SC_PHYS_PAGES);
	pagesize 	= sysconf(_SC_PAGESIZE) / 1024;

	kn = kstat_data_lookup(ks, "availrmem");
	avlmem 		= kn->value.ul;
	kstat_close(kc);

	*total	= (unsigned long)(maxmem*pagesize);
	*avail	= (unsigned long)(avlmem*pagesize);
	return 1;
}
#endif

#ifdef Linux

#if 0	/* earlier Linux Kernel 2.6, old function	*/
int getmemuse(unsigned long *total,unsigned long *avail)
{
	char s[BUFSIZ],arg[BUFSIZ];
/* 
 *  d1=total, d2=used, d3=free, d4=shared, d5=buffer, d6=cache 
 *  total = total 
 *  free  = free + (buffer + cache)
 */

	unsigned long d1,d2,d3,d4,d5,d6,found;
	FILE *fp;

	if(!(fp=fopen("/proc/meminfo","r"))) return 0;
	for(found=0,bzero(s,BUFSIZ);fgets(s,BUFSIZ,fp);fflush(fp),bzero(s,BUFSIZ)) {
		strlwr(s);
		if(!strncmp(s,"mem:",4)) {
			if(sscanf(s,"%s%u%u%u%u%u%u",arg,&d1,&d2,&d3,&d4,&d5,&d6)<7) return 0;
			found=1;
            *total = d1;
            *avail = d3 + d5 + d6;
#ifdef DEBUG 
			printf("total:%u   used:%u   avail:%u  return-> total:%u   avail:%u\n", d1, d2, d3, *total, *avail);
#endif
			break;
		}
	}
	fclose(fp);
	return found;
}

/* include swap memory	*/
int getmemuses(unsigned long *total, unsigned long *avail)
{
    char s[BUFSIZ],arg[BUFSIZ];
	unsigned long d1,d2,d3,d4,d5,d6,found=0;
    FILE *fp;
	
	*total = *avail =0;

    if(!(fp=fopen("/proc/meminfo","r"))) return 0;
    for(found=0,bzero(s,BUFSIZ);fgets(s,BUFSIZ,fp);fflush(fp),bzero(s,BUFSIZ)) {
        strlwr(s);
		if(!strncmp(s,"mem:",4)) {
			if(sscanf(s,"%s%u%u%u%u%u%u",arg,&d1,&d2,&d3,&d4,&d5,&d6)<7) continue;
			found+=1;
            *total = d1;
            *avail = d3 + d5 + d6;
#ifdef DEBUG 
			printf("total:%u   used:%u   avail:%u  return-> total:%u   avail:%u\n", d1, d2, d3, *total, *avail);
#endif
			break;
		}

        if(!strncmp(s,"swap:",5)) {
			if(sscanf(s,"%s%u%u%u",arg,&d1,&d2,&d3)<4) continue;
            found+=1;
            *total += d1;
            *avail += d3;
#ifdef DEBUG 
            printf("total:%u   used:%u   avail:%u  return-> total:%u   avail:%u\n", d1, d2, d3, *total, *avail);
#endif
            break;
        }
    }
    fclose(fp);
    return found;
}

#else

unsigned long getmemval(char *s, unsigned long *found)
{
	char 			label[BUFSIZ];
	unsigned long 	val;

	if(sscanf(s, "%s%ld", label, &val)<2) return 0;

#ifdef DEBUG
	printf("%s = %u\n", label, val);
#endif
	*found++;
	return val;
}

int getmemuse(unsigned long *total,unsigned long *avail)
{
	char s[BUFSIZ],arg[BUFSIZ];
/* 
 *  d1=total, d2=used, d3=free, d4=shared, d5=buffer, d6=cache 
 *  total = total 
 *  free  = free + (buffer + cache)
 */
	unsigned long 	d1,d2,d3,d4,val,found;	/* d1=total, d2=free, d3=buffer, d4=cache */
	FILE *fp;

	if(!(fp=fopen("/proc/meminfo","r"))) return 0;
	for(found=0,bzero(s,BUFSIZ);fgets(s,BUFSIZ,fp);fflush(fp),bzero(s,BUFSIZ)) {
		strlwr(s);
		if (!strncmp(s, "memtotal:", 9))		d1 = getmemval(s, &found);
		else if (!strncmp(s, "memfree:", 8))	d2 = getmemval(s, &found);
		else if (!strncmp(s, "buffers:", 8))	d3 = getmemval(s, &found);
		else if (!strncmp(s, "cached:", 7)) 	d4 = getmemval(s, &found);

#ifdef DEBUG 
			printf("total:%u   used:%u   avail:%u  return-> total:%u   avail:%u\n", d1, d2, d3, *total, *avail);
#endif
	}

	*total = d1;		/* kb -> byte	*/
	*avail = (d2 + d3 + d4) ;

	fclose(fp);
	return found;
}

/* include swap memory	*/
int getmemuses(unsigned long *total,unsigned long *avail)
{
	char s[BUFSIZ],arg[BUFSIZ];
/* 
 *  d1=total, d2=used, d3=free, d4=shared, d5=buffer, d6=cache 
 *  total = total  + swap_total
 *  free  = free + (buffer + cache) + swap_free;
 */

	unsigned long 	d1,d2,d3,d4,d5,d6,val,found;	/* d1=total, d2=free, d3=buffer, d4=cache, d5=swap_total, d6=swap_free */
	FILE *fp;

	if(!(fp=fopen("/proc/meminfo","r"))) return 0;
	for(found=0,bzero(s,BUFSIZ);fgets(s,BUFSIZ,fp);fflush(fp),bzero(s,BUFSIZ)) {
		strlwr(s);
		if (!strncmp(s, "memtotal:", 9))		d1 = getmemval(s, &found);
		else if (!strncmp(s, "memfree:", 8))	d2 = getmemval(s, &found);
		else if (!strncmp(s, "buffers:", 8))	d3 = getmemval(s, &found);
		else if (!strncmp(s, "cached:", 7)) 	d4 = getmemval(s, &found);
		else if (!strncmp(s, "swaptotal:", 10))	d5 = getmemval(s, &found);
		else if (!strncmp(s, "swapfree:", 9))	d6 = getmemval(s, &found);

#ifdef DEBUG 
			printf("total:%u   used:%u   avail:%u  return-> total:%u   avail:%u\n", d1, d2, d3, *total, *avail);
#endif
	}

	*total = (d1 + d5);	/* kb -> byte	*/
	*avail = (d2 + d3 + d4 + d6);

	fclose(fp);
	return found;
}

#endif	/* for if 0	*/

#endif	/* for defined (Linux)	*/

#if defined(EXAMPLECODE)

int
main()
{
	unsigned long tot,avl;	

	getmemuse(&tot,&avl);
	printf("\ntotal:%u   avail:%u\n",total, avail);
}

#endif
