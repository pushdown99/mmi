#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include "log.h"
#include "__time.h"

#define DEFAULT_LOG_PATH		"/var/log"

log_t _log[MAX_LOG];

int check_path(int idx);

int initlog(int idx, char *path, char *sys, char *procs, int lev, int out, int mode)
{
	struct stat b;
	char date[BUFSIZ];
	int eol;

	if (idx>=MAX_LOG) return -1;

	bzero(_log[idx].fname, BUFSIZ);
	_log[idx].level = lev;
	_log[idx].stdio = out;
	_log[idx].fsize = MID_LOG_SIZE;
	_log[idx].fnum = MAX_FNUM;
	_log[idx].mode = mode;

	strcpy(_log[idx].path, path);
	strcpy(_log[idx].sys, sys);
	strcpy(_log[idx].procs, procs);
	strcpy(_log[idx].date, fmttm((char*)"%Y%m%d", date)); 


	eol = strlen(path)-1;
	if(path[eol]=='/') path[eol]=0;

	if (mode==MODE_NO_PATH) {
		sprintf(_log[idx].fname, "%s/%s.%s.log"
					,_log[idx].path
					,_log[idx].procs
					,fmttm((char*)"%Y%m%d", date)
		);

		sprintf(_log[idx].err_fname,"%s/%s.%s.err"
					,_log[idx].path
					,_log[idx].procs
					,fmttm((char*)"%Y%m%d",date)
		);
	} else 
		if(!check_path(idx)) return -1;

#if 0
	if(stat(_log[idx].fname, &b)<0) {
                printf("ERR-SYS-%5d %s \n", errno, strerror(errno));
				sprintf(_log[idx].fname, "%s/%s%s.log"
									,_log[idx].path
									,sys
									,fmttm((char*)"%Y%m%d", date)
				);

				sprintf(_log[idx].err_fname,"%s/%s%s.err"
									,_log[idx].path
									,sys
									,fmttm((char*)"%Y%m%d",date)
				);
	}
#endif
	return 1;
}

int logset(int idx, int maxfn, int maxfs)
{
	if (idx>=MAX_LOG) return -1;
	if (!maxfn || !maxfs) return -1;

	_log[idx].fnum=maxfn;
	_log[idx].fsize=maxfs;

	return 1;
}

int setlogf(int idx, int maxfn, int maxfs)
{
	return logset(idx, maxfn, maxfs);
}

int setloglevel(int idx, int level) 
{
	if (idx>=MAX_LOG) return -1;
	if (level>LOG_9 || level<LOG_0) return -1;
	_log[idx].level = level;
	
	return 1;
}

int getloglevel(int idx)
{
	return _log[idx].level;
}

int check_path(int idx)
{
	struct stat b;
	int eol;

	char path[BUFSIZ];
	char date[BUFSIZ];

	if ( _log[idx].path[0] == 0 ) strncpy ( _log[idx].path, DEFAULT_LOG_PATH, strlen(DEFAULT_LOG_PATH) );

	bzero(_log[idx].fname, BUFSIZ);

	sprintf(path, "%s/%s"   /* ~log/year            */
				,_log[idx].path
				,fmttm((char*)"%Y", date)
	);
	if (stat(path, &b)<0) {
		if(mkdir(path, 0755)==-1) return -1;
	}

	sprintf(path, "%s/%s"   /* ~log/year/month      */
					,path
					,fmttm((char*)"%m", date)
	);
	if(stat(path, &b)<0) {
		if(mkdir(path, 0755)==-1) return -1;
	}

	sprintf(path, "%s/%s"   /* ~log/year/month/day  */
					,path
					,fmttm((char*)"%d", date)
	);
	if(stat(path, &b)<0) {
		if(mkdir(path, 0755)==-1) return -1;
	}

	sprintf(_log[idx].fname, "%s/%s.%s.log"
						,path
						,_log[idx].procs
						,fmttm((char*)"%Y%m%d", date)
	);

	sprintf(_log[idx].err_fname, "%s/%s.%s.err"
						,path
						,_log[idx].procs
						,fmttm((char*)"%Y%m%d", date)
	);
	
	return 1;
}

int check_fsize(int idx)
{
	struct stat b;
	char path[BUFSIZ];
	int i, old=1;
	time_t oldtime;
	if(stat(_log[idx].fname, &b)<0) return 0;
	if(b.st_size<_log[idx].fsize) return 0;
	for(i=1, old=1;i<=_log[idx].fnum; i++) {
		sprintf(path, "%s.%.4d", _log[idx].fname, i);
		if(!stat(path, &b)) {
			if(i==1) oldtime = b.st_mtime;
			if(b.st_mtime<oldtime) {
					oldtime = b.st_mtime; old=i; 
			}
			continue;
		}
		sprintf(path, "%s.%.4d", _log[idx].fname, i);
		rename(_log[idx].fname, path);
		return 1;
	}
	sprintf(path, "%s.%.4d", _log[idx].fname, old);
	rename(_log[idx].fname, path);
	return 1;
}

int check_date(int idx)
{
	char date[BUFSIZ];
	
	if(strcmp(_log[idx].date, fmttm((char*)"%Y%m%d", date))) {
		initlog(idx
				,_log[idx].path
				,_log[idx].sys
				,_log[idx].procs
				,_log[idx].level
				,_log[idx].stdio
				,_log[idx].mode
		);
	}

	return 1;
}


/*
 *		lprintf - format and print log data
 *		===================================
 *
 *		- INPUT
 *		level   : debug level
 *		fmt     : format
 *		...     : variable parameter
 *
 *		- OUTPUT
 *		1       : success
 *		0       : error
 *
 */


int
lprintf(int idx, int level,char* fmt,...)
{
	va_list ap;
	FILE *fp;

	if(_log[idx].level<level) return 0;
	
	check_date(idx);
	check_fsize(idx);

	fp = fopen(_log[idx].fname,"a");
	if (fp==NULL) return 0;

	va_start(ap,fmt);
	if(_log[idx].stdio&LOG_FILE) vfprintf(fp,fmt,ap);
	if(_log[idx].stdio&LOG_CONSOLE) vprintf(fmt,ap);
	va_end(ap);
	fflush(fp); fclose(fp);

	return 1;
}

/*
 *		logprn - print log data with well-defined format
 *		================================================
 *
 *		- INPUT
 *		level   : debug level
 *		tid     : thread id
 *		fmt     : format
 *		...     : variable parameter
 *
 *		- OUTPUT
 *		1       : success
 *		0       : error
 * 
 */
int
logprn(int idx, int level,int tid,char* fmt,...)
{
	char tm[BUFSIZ],s[BUFSIZ];
	FILE *fp;
	va_list ap;

	if(_log[idx].level>level) return 0;
	check_date(idx);
	check_fsize(idx);

	fp = fopen(_log[idx].fname,"a");
	if (fp==NULL) return 0;

	va_start(ap,fmt);
	sprintf(s,"%s [%s:%s:%d] %s"
		,fmttm((char*)"%Y/%m/%d %H:%M:%S.%t",tm)
		,_log[idx].sys
		,_log[idx].procs
		,tid
		,fmt
	);

	if(_log[idx].stdio&LOG_FILE) vfprintf(fp,s,ap);
	if(_log[idx].stdio&LOG_CONSOLE) vprintf(s,ap);
	va_end(ap);
	fflush(fp); fclose(fp);
	return 1;
}

/*
 *		logdump - dump log data with well-defined format
 *		================================================
 *
 *		- INPUT
 *		p       : char pointer
 *		n       : dump length
 *
 *		- OUTPUT
 *		1       : success
 *		0       : error
 * 
 */
int
logdump(int idx, unsigned char* p, int n)
{
	char s[BUFSIZ], tb[2], lb[64], rb[64], ts[256];
	FILE *fp;
	int i;

	check_date(idx);
	check_fsize(idx);

	fp = fopen(_log[idx].fname,"a");
	if (fp==NULL) return 0;

	memset(s, 0, BUFSIZ);
	sprintf(s, "# DUMP\n");
	for (i=0; i<n; i++) {
		if (!(i%4))
			strcat(lb, " ");
		if (!(i%16)) {
			if (i) {
				sprintf(ts, "%04x: %-*s    %s\n", (i%16), 20, (char*)lb, (char*)rb);
				printf("%s", ts);
				strcat(s, ts);
			}
			memset(lb, 0, 64);
			memset(rb, 0, 64);
		}
		sprintf(tb, "%02x", p[i]);	
		strcat(lb, tb);
		sprintf(tb, "%c", (!iscntrl(p[i]) && p[i] <= 0x7f)?p[i]:'.');
		strcat(rb, tb);
	}
	if (i%16) {
		sprintf(ts, "%04x: %s    %s\n", (i/16), lb, rb);
		strcat(s, ts);
	}

	if(_log[idx].stdio&LOG_FILE) fprintf(fp,"%s", s);
	if(_log[idx].stdio&LOG_CONSOLE) printf("%s", s);
	fflush(fp); fclose(fp);
	return 1;
}

/*
 *		eprintf - format and print error data
 *		=====================================
 *
 *		- INPUT
 *		fmt     : format
 *		...     : variable parameter
 *
 *		- OUTPUT
 *		1       : success
 *		0       : error
 *    
 */ 
int
eprintf(int idx, char* fmt,...)
{
	va_list ap;
	FILE *fp;

	check_date(idx);
	check_fsize(idx);

	fp = fopen(_log[idx].err_fname,"a");
	if(fp==NULL) return 0;

	va_start(ap,fmt);
	if(_log[idx].stdio&LOG_FILE) vfprintf(fp,fmt,ap);
	if(_log[idx].stdio&LOG_CONSOLE) vprintf(fmt,ap);
	va_end(ap);
	fflush(fp); fclose(fp);
	return 1;
}


/*
 *		errprn - print error data with well-defined format
 *		==================================================
 *
 *		- INPUT
 *		level   : error level
 *		tid     : thread id
 *		errno   : error number
 *		fmt     : format
 *		...     : variable parameter
 *
 *		- OUTPUT
 *		1       : success
 *		0       : error
 *     
 */
int
errprn(int idx, int level,int tid,int err_num,char* fmt,...)
{
	char tm[BUFSIZ],s[BUFSIZ];
	va_list ap;
	FILE *fp;

	check_date(idx);
	check_fsize(idx);

	fp = fopen(_log[idx].err_fname,"a");
	if (fp==NULL) return 0;

	va_start(ap,fmt);
	sprintf(s,"%s [%s:%s:%d] %s-%05d %s\n"
		,fmttm((char*)"%Y/%m/%d %H:%M:%S.%t",tm)
		,_log[idx].sys
		,_log[idx].procs
		,tid
		,(level==ERR_SYS_PANIC)? "PANIC.ERR-SYS"
		:(level==ERR_USR_PANIC)? "PANIC.ERR-USR"
		:(level==ERR_SYS_NORMAL)? "ERR-SYS"
		:"ERR-USR"
		,err_num
	,fmt
	);
	if(_log[idx].stdio&LOG_FILE) vfprintf(fp,s,ap);
	if(_log[idx].stdio&LOG_CONSOLE) vprintf(s,ap);
	va_end(ap);
	fflush(fp); fclose(fp);
	return 1;
}

#if defined(EXAMPLECODE)
#define TEST_LOG0	0
#define TEST_LOG1	1
#define LOG1(f, a...) lprintf(TEST_LOG1,LOG_0,f, ## a)
int main()
{

	int loop;
	char date[BUFSIZ];
	char t[BUFSIZ];

	if (initlog(TEST_LOG0, "/root/log/logtest1", "ems", "logtest1", LOG_0, LOG_FILE, MODE_PATH)<=0) {
		printf("\nLog Init Fail\n");
		exit(0);
	}
#if 1	
	if (initlog(TEST_LOG1, "/root/log/logtest2", "ems", "logtest2", LOG_0, LOG_FILE, MODE_NO_PATH)<=0) {
		printf("\nLog Init Fail\n");
		exit(0);
	}
#endif

	logset(TEST_LOG0, 5, 4*1024*1024);
	logset(TEST_LOG1, 10, 8*1024*1024); 


	for(loop=0;loop<10000000;loop++) {
		logprn(TEST_LOG0, LOG_0,0,"Log0- Testing (%s:%d)\r\n"
			,__FILE__,__LINE__);

#if 0
		lprintf(TEST_LOG1, LOG_0, "Log1- Testing (%s:%d)\r\n"
            ,__FILE__,__LINE__);
#else
		LOG1("Log1- Testing (%s:%d)\r\n",__FILE__,__LINE__);

#endif

//		sleep(1);
	}

}


#endif

