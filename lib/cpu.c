#include <stdio.h>
#include <string.h>
#include <strings.h>

#ifdef SunOS

#include <kstat.h>
#include <sys/cpuvar.h>

#define loaddouble(la) ((double)(la) / FSCALE)

int getcpuuse(int *ncpus,int *min01,int *min05,int *min15,int *min60)
{
	kstat_t *ks;
	kstat_ctl_t *kc = NULL;
	kstat_named_t *kn;
	double lf1,lf2,lf3,lf4;

	if(!(kc = kstat_open())) return 0;
	ks = kstat_lookup(kc, "unix", 0, "system_misc");
	if(kstat_read(kc, ks, 0) < 0) return 0;
	kn = kstat_data_lookup(ks, "ncpus"); 	if(kn) *ncpus = kn->value.ui32;
	kn = kstat_data_lookup(ks, "avenrun_1min"); if(kn) *min01 = (int)(loaddouble(kn->value.ui32)*100.0);
	kn = kstat_data_lookup(ks, "avenrun_5min"); if(kn) *min05 = (int)(loaddouble(kn->value.ui32)*100.0);
	kn = kstat_data_lookup(ks, "avenrun_15min");if(kn) *min15 = (int)(loaddouble(kn->value.ui32)*100.0);
	kn = kstat_data_lookup(ks, "avenrun_60min");if(kn) *min60 = (int)(loaddouble(kn->value.ui32)*100.0);
	kstat_close(kc);
	return 1;	
}
#endif

#ifdef Linux

#if 1

#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <pthread.h>

#define TM_MIN			60	
#define TM_SEC			1000

int 		load_avg_init   = 0 ;
int         cpu_count       = 1 ;

double 		cpu_load_avg    = 0.0 ;
double 		cpu_1min_avg    = 0.0 ;
double 		cpu_5min_avg    = 0.0 ;
double 		cpu_15min_avg   = 0.0 ;
double 		cpu_60min_avg   = 0.0 ;

pthread_t   cpu_load_t ;

/* for debug    */
void show_cpu_load()
{
        printf("\n===================================================\n");
        printf("%-15s%-15s%-15s%-15s\n","1MIN", "5MIN", "15MIN","60MIN");
        printf("===================================================\n");
        printf("%-15f%-15f%-15f%-15f\n",  cpu_1min_avg
                                ,cpu_5min_avg
                                ,cpu_15min_avg
				,cpu_60min_avg
        );

        printf("===================================================\n");
}

unsigned int cpu_cur_idle      = 0 ;
unsigned int cpu_last_idle     = 0 ;

void get_cpu_load() 
{
	FILE *      fp;
	double 	    cpu_load_avg;
	char 	    line[BUFSIZ], cpu[20], *pp;
    uint32_t    idle ;
	uint32_t 	utime, nice, stime; 
    double      idle_avg;
    static      int     count = 1;
	static 	    int     run = 0;
	int	        i;

    memset ( (void*) line ,     0x00,   BUFSIZ );
    memset ( (void*) cpu,       0x00,   20 );

    fp = fopen("/proc/stat", "r");

    if ( fp == NULL ) {
        /* for time check       */
        count ++ ;
        return ;
    }

	while(fgets(line, BUFSIZ, fp)) {
		if(!strstr(line, "cpu")) continue;
		if((pp=strchr(line,'\n'))) *pp=0;	
		if(sscanf(line, "%s%u%u%u%u", cpu, &utime, &nice, &stime, &idle)!=5) {
            count ++ ;
            return;
        }
		break;
	}
	fclose(fp);

	cpu_cur_idle    = idle;

    if ( cpu_last_idle != 0 ) {
        idle_avg = 1.0 * (cpu_cur_idle - cpu_last_idle) / (cpu_count * count * TM_MIN * HZ ) ;
     //   printf ( "+++ idle-avg = %f ++ \n",  idle_avg );
        if ( idle_avg > 1.0) idle_avg = 1.0 ;
        else if ( idle_avg < 0.0 ) idle_avg = 0.0 ;
        cpu_load_avg = 1.0 - idle_avg ;
    } else {
        cpu_load_avg = 0.0 ;
    }

    /* debug */
    //printf ( " cur-idle-time = %d, last-idle-time = %d, cur-last = %d , time-diff = %d , count = %d, cpu-load-avg = %f \n", cpu_cur_idle, cpu_last_idle, cpu_cur_idle - cpu_last_idle, cpu_count * count * TM_MIN * HZ,count, cpu_load_avg );
    cpu_last_idle = cpu_cur_idle ;  


	/* save cpu load avg	*/
	cpu_1min_avg    = cpu_load_avg;
	cpu_5min_avg    = (run)?	( cpu_5min_avg*(4.0/5)      + cpu_1min_avg*(1.0/5)  )	:cpu_1min_avg;
	cpu_15min_avg   = (run)?	( cpu_15min_avg*(14.0/15)   + cpu_1min_avg*(1.0/15) )	:cpu_5min_avg;
	cpu_60min_avg   = (run)?	( cpu_60min_avg*(59.0/60)   + cpu_1min_avg*(1.0/60) )	:cpu_15min_avg;

    /* reset env    */
	run     = 1;
    count   = 1;

#if defined(DEBUG) 
	show_cpu_load();
#endif 
}


#endif


void *cpuload_task(void *arg)
{
	struct timeval t;

	get_cpu_load();

	while(1) {
		t.tv_sec = TM_MIN;
		t.tv_usec = 0;

		select(0,0,0,0,&t);
		get_cpu_load();
	}
}

pthread_attr_t load_avg_attr;

int getcpuuse(int *ncpus,int *min01,int *min05,int *min15,int *min60)
{
    char s[BUFSIZ];
    float f1,f2,f3;
    int  cnt = 0;
    FILE *fp;

	if(!load_avg_init) {
		load_avg_init=1;
		pthread_attr_init(&load_avg_attr);
		pthread_attr_setstacksize(&load_avg_attr,2);
		pthread_create(&cpu_load_t, &load_avg_attr, cpuload_task, NULL);
	}

    if(!(fp=fopen("/proc/cpuinfo","r"))) return 0;
    for(bzero(s,BUFSIZ),cnt=0;fgets(s,BUFSIZ,fp);bzero(s,BUFSIZ)) {
        strlwr(s);
        if(strstr(s,"processor")) cnt++;
    }
    fclose(fp);

    *ncpus  = cnt ;
    cpu_count = cnt ;

	*min01 = (int)(cpu_1min_avg*100);
	*min05 = (int)(cpu_5min_avg*100);
	*min15 = (int)(cpu_15min_avg*100);
	*min60 = (int)(cpu_60min_avg*100);

    return cnt;
}


#endif


