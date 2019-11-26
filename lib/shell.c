#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

#include "conf.h"
#include "proc.h"
#include "shell.h"

#define MAX_CRZ_CNT     3

proc_t   proctab[MAXPROCESS];
int      nproc;

pthread_mutex_t	proc_mutex;

struct timeval lasttime;

void 	(*proc_cb_up)(char*) = NULL;
void 	(*proc_cb_dn)(char*) = NULL;

#ifdef __cplusplus
extern "C"
#endif

char *skip_ws(const char *p)
{
    while (isspace(*p)) p++;
    return (char *)p;
}

char *skip_token(const char *p)
{
    while (isspace(*p)) p++;
    while (*p && !isspace(*p)) p++;
    return (char *)p;
}

#ifdef Linux
int read_proc_stat(proc_t *pp, pid_t pid)
{
#define MAX_PROC_PATH   128 
#define MAX_PROC_BUF    4096 
#define MAX_PROC_NAME   128  
        int fd;
        FILE *fp;
        char path[MAX_PROC_PATH];
        char buf[MAX_PROC_BUF];
		char name[MAX_PROC_NAME];
        char *p, *q;
        struct stat b;
        int len=0;

        unsigned long cputime;

        if(!pid || !pp->pid) return 0; 
//logprn(0, 0, 0, "(STI-LIB:shell.c) func=read_proc_stat() start \n");

        bzero(name, MAX_PROC_NAME);
        bzero(path, MAX_PROC_PATH);
        bzero(buf,  MAX_PROC_BUF);

        sprintf(path, "/proc/%d/stat",pid);

        if(stat(path, &b)<0)           return 0;

        fd = open(path, O_RDONLY);
		if (fd<=0) return 0;		
        len = read(fd, buf, MAX_PROC_BUF);
        close(fd);
        p=buf;

        if(!(p=strchr(p, '(')))   return 0;  /* cannot find '(' */ 
        else p+=1;

        if(!(q=strrchr(p, ')')))   
            return 0;  /* cannot find ')'  */
        else len=q-p;

        if(len<=0 || len>=MAX_PROC_NAME) return 0;  /* proc name length check   */

		strncpy(name, p, len);
		name[len]='\0';

		if (strcmp(pp->name, name)) {
//logprn(0, 0, 0, "(STI-LIB:shell.c) func=read_proc_stat() return pp->name=NULL \n");
            return 0;
        }

        p = q+1;

        p = skip_ws(p);

        switch (*p++)
        {
            case 'R': pp->state = 1; break;
            case 'S': pp->state = 2; break;
            case 'D': pp->state = 3; break;
            case 'Z': pp->state = 4; break;
            case 'T': pp->state = 5; break;
            case 'W': pp->state = 6; break;
            default : pp->state =0;

        }

        p = skip_token(p); /* proc->ppid = strtoul(p, &p, 10);        skip ppid */
        p = skip_token(p);              /* skip pgrp */
        p = skip_token(p);              /* skip session */
        p = skip_token(p);              /* skip tty */
        p = skip_token(p);              /* skip tty pgrp */
        p = skip_token(p);              /* skip flags */
        p = skip_token(p);              /* skip min flt */
        p = skip_token(p);              /* skip cmin flt */
        p = skip_token(p);              /* skip maj flt */
        p = skip_token(p);              /* skip cmaj flt */

        cputime = strtoul(p, &p, 10);        /* utime */
        cputime += strtoul(p, &p, 10);       /* stime */

        pp->cputime += cputime;

//logprn(0, 0, 0, "(STI-LIB:shell.c) func=read_proc_stat() end \n");
}

int get_proc_info(void)
{
    static int first=0;
    DIR *dir;
    FILE *fp;
    struct dirent *ent;
    struct timeval thistime;
    static struct timeval lasttime;
    double timediff, alpha, beta;
    pid_t pid;
    proc_t *p;
    int i, ncpus=0;
    char    s[BUFSIZ];

    //logprn(0, 0, 0, "(STI-LIB:shell.c) func=get_proc_info() start \n");

    if(!first)  {
        bzero(&lasttime, sizeof(struct timeval));
        first=1;
    }

   if(!(fp=fopen("/proc/cpuinfo","r"))) return 0;
   for(bzero(s,BUFSIZ),ncpus=0;fgets(s,BUFSIZ,fp);bzero(s,BUFSIZ)) {
       strlwr(s);
       if(strstr(s,"processor")) ncpus++;
   }
   fclose(fp);

    gettimeofday(&thistime, 0);

    if (lasttime.tv_sec) {
        timediff = ((thistime.tv_sec - lasttime.tv_sec) +
                        (thistime.tv_usec - lasttime.tv_usec) * 1e-6);
    } else
        timediff = 1e9;

    lasttime = thistime;

    if (timediff < 30.0) {
        alpha = 0.5 * (timediff / 30.0);
        beta = 1.0 - alpha;
    } else {
        alpha = beta = 0.5;
    }

    timediff *= HZ;

	for(i=0;i<nproc;i++) {
		dir = opendir("/proc");
		proctab[i].cputime=0;
		proctab[i].state=0;	
		proctab[i].ccpu=0.0;
		proctab[i].dcpu=0;
		if(proctab[i].run) {
			    while((ent = readdir(dir)) != NULL) {
			        if (!isdigit(ent->d_name[0])) continue;
					read_proc_stat(&proctab[i], atoi(ent->d_name));
				}
		}

		if (proctab[i].otime) 
			proctab[i].ccpu = ((proctab[i].cputime - proctab[i].otime) / timediff) / ncpus;

		if (proctab[i].ccpu>=1.0) proctab[i].ccpu=1.0;

		proctab[i].dcpu = (unsigned long)(proctab[i].ccpu *100000);

		if (proctab[i].cputime>=0)
			proctab[i].otime = proctab[i].cputime;

		closedir(dir);

        proctab[i].crz_cnt = 0;
	}
    //logprn(0, 0, 0, "(STI-LIB:shell.c) func=get_proc_info() end \n");
}

#endif

void *pstattask(void *args)
{
#ifdef Linux
	struct timeval tm;

	while(1) {
		tm.tv_sec=1;
		tm.tv_usec=0;
		if (select(0, 0, 0, 0, &tm) <0) ;
		get_proc_info();
	}
#endif
}

void sigchld(int signo)
{
    int pid,stat,i;

    pid = wait3(&stat, WNOHANG, NULL);
	for(i=0;i<nproc;i++) {
		if(!proctab[i].run) continue;
		if(proctab[i].pid!=pid || !pid) continue;
		if(proc_cb_dn) proc_cb_dn(proctab[i].name);
		proctab[i].pid = 0;
		proc_start(i);
	}
    signal(SIGCLD,sigchld);
}

void sigchld2(int sig)
{
    pid_t pid;
    int stat;

    pid = wait3(&stat, WNOHANG, NULL);

    signal(SIGCLD,sigchld2);
}

void proc_show()
{
    char tm[BUFSIZ];
	int i;

    mprintf("----------------------------------------------------------------\n");
    mprintf("%-6s%-4s%-20s%-10s%-20s%-10s\n","Pid","Run","Path","Name","Time", "CPU(\%)");
    mprintf("----------------------------------------------------------------\n");
    for(i=0;i<nproc;i++) {
        gfmttm(&proctab[i].stime,"%y/%m/%d %H:%M:%S",tm);
        mprintf("%-6d%-4d%-20s%-10s%-20s%-10f\n"
			,proctab[i].pid
			,proctab[i].run
			,proctab[i].path
			,proctab[i].name
			,tm
			,proctab[i].ccpu
		);
    }
    mprintf("\n\n");
}

int proc_up(char* name)
{
	int i;
	int ret=0;

	for(i=0;i<nproc;i++) {
		if(strcmp("all",name) && strcmp(proctab[i].name,name)) continue;
		proctab[i].run	= 0;
		if(proctab[i].pid>0) kill(proctab[i].pid,SIGTERM);
		proc_start(i);
		if(strcmp("all",name)) return proctab[i].pid;
		else ret=1;
	}
	return ret;
}

int proc_dn(char* name)
{
    int i;
    int ret=0;

    for(i=0;i<nproc;i++) {
        if(strcmp("all",name) && strcmp(proctab[i].name,name)) continue;
        proctab[i].run  = 0;
        if(proctab[i].pid>0) kill(proctab[i].pid,SIGTERM);
    	proctab[i].etime= time(0);
		if(proc_cb_dn) proc_cb_dn(proctab[i].name);
        if(strcmp("all",name)) return proctab[i].pid;
	else ret=1;
    }
    return ret;
}

int proc_start(int i)
{
	char cmd[BUFSIZ];

	if (proctab[i].name[0]) {
		sprintf(cmd, "pkill -9 %s", proctab[i].name);   /* SIGKILL  */
		system(cmd);
	}

	/* detect abnormal process	*/
    if (proctab[i].crz_cnt >= MAX_CRZ_CNT) {       
        /* proc_dn("all");		-- prev (2005/01/13) --	*/
		proctab[i].run = 0;
		if(proctab[i].pid) kill(proctab[i].pid, SIGKILL);	/* send sigkill	*/	
		proctab[i].etime = time(0);
		if(proc_cb_dn) proc_cb_dn(proctab[i].name);
        return 0;
    }

	if(proctab[i].pid>0) kill(proctab[i].pid,SIGTERM);
    proctab[i].pid 	= exeproc(proctab[i].path,proctab[i].name,proctab[i].tag,NULL);
    proctab[i].run 	= 1;
    proctab[i].stime= time(0);
    proctab[i].crz_cnt += 1;
	if(proc_cb_up) proc_cb_up(proctab[i].name);
    return proctab[i].pid;
}

int proc_stop(int i)
{
	proctab[i].run	= 0;
    proctab[i].etime= time(0);
	if(proctab[i].pid>0) kill(proctab[i].pid,SIGTERM);	
	if(proc_cb_dn) proc_cb_dn(proctab[i].name);
    proctab[i].etime= time(0);
	return 1;
}

int	proc_quit()
{
	int i;

	signal(SIGCLD,SIG_IGN); 
	for(i=0;i<nproc;i++) {
		if(!proctab[i].run) continue;

		if(proctab[i].pid>0) kill(proctab[i].pid,SIGTERM);
	}
	return 1;
}

int proc_conf(char* f,char* tag)
{
    char buf[BUFSIZ],*p;
	char path[BUFSIZ],dir[BUFSIZ],name[BUFSIZ];
    FILE *fp;
	int  i;

	PROC_MUTEX_INIT();

    signal(SIGCLD,sigchld);

	if(!(fp=fopen(f,"r"))) return 0;
	/* read tbl.process	*/
    for(nproc=0;fgets(buf,BUFSIZ,fp)!=NULL;bzero(buf,BUFSIZ)) {
        fflush(fp);
        if(p=strchr(buf,'#')) 	*p=0;
        if(p=strchr(buf,'\n')) 	*p=0;
        if(p=strchr(buf,'\r')) 	*p=0; btrim(buf);
        if(!*buf) continue;
        strcpy(path,buf);
        if(p=strrchr(buf,'/')) *p=' ';
        if(sscanf(buf,"%s%s",dir,name)!=2) break;

		/* fill-in process.tbl 	*/
		strcpy(proctab[nproc].path,path);
		strcpy(proctab[nproc].name,name);
		strcpy(proctab[nproc].tag ,tag ); 
        proctab[nproc].crz_cnt=0;       nproc++;
    }
    fclose(fp);

	for(i=0;i<nproc;i++) proc_start(i);

    return nproc;
}

int daemon_dn(char* name)
{
    int i;
    char cmd[256];

    for(i=0;i<nproc;i++) {
        if(strcmp("all",name) && strcmp(proctab[i].name,name)) continue;
        proctab[i].run  = 0;
        if(proctab[i].pid>0) {
            sprintf(cmd, "pkill -9 %s", proctab[i].name);
            system(cmd);
        }
        proctab[i].etime= time(0);
        if(proc_cb_dn) proc_cb_dn(proctab[i].name);
        if(strcmp("all",name)) return 1;
    }
    return 1;
}

int daemon_up(char* name)
{
    proc_up(name);
}

int daemon_start(int i)
{
    proc_start(i);
}

int daemon_conf(char* f, char* tag)
{
    char buf[BUFSIZ],*p;
    char path[BUFSIZ],dir[BUFSIZ],name[BUFSIZ];
    FILE *fp;
    int  i;

	PROC_MUTEX_INIT();

    signal(SIGCLD,sigchld2);

    if(!(fp=fopen(f,"r"))) return -1;
    /* read tbl.process */
    for(nproc=0;fgets(buf,BUFSIZ,fp)!=NULL;bzero(buf,BUFSIZ)) {
        fflush(fp);
        if(p=strchr(buf,'#'))   *p=0;
        if(p=strchr(buf,'\n'))  *p=0;
        if(p=strchr(buf,'\r'))  *p=0; btrim(buf);
        if(!*buf) continue;
        strcpy(path,buf);
        if(p=strrchr(buf,'/')) *p=' ';
        if(sscanf(buf,"%s%s",dir,name)!=2) break;

        /* fill-in process.tbl  */
        strcpy(proctab[nproc].path,path);
        strcpy(proctab[nproc].name,name);
        strcpy(proctab[nproc].tag ,tag ); nproc++;
    }
    fclose(fp);

    for(i=0;i<nproc;i++) daemon_start(i);

    return 1;
}

int daemon_stat()
{
    char path[64], buf[1024], pname[32], *p,*q;
    DIR *dir;
    struct dirent *ent;
    pid_t pid;
    int i, fd;
    int len;


    for(i=0;i<nproc;i++) {
        proctab[i].run=0;
        proctab[i].pid=0;
    }

    dir = opendir("/proc");

    while((ent=readdir(dir))!=NULL) {
        if (!isdigit(ent->d_name[0])) continue;

        bzero(buf, 1024);
        sprintf(path, "/proc/%s/stat", ent->d_name);
        fd = open(path, O_RDONLY);
        if (fd<=0) continue;
        len = read(fd, buf, 1024-1);
        close(fd);
        p=buf;

        p = strchr(p, '(')+1;
        q = strrchr(p, ')');
        len = q-p;

        memcpy(pname, p, len);
        pname[len]='\0';

        for(i=0;i<nproc;i++) {
            if (strcmp(proctab[i].name, pname)) continue;

            proctab[i].pid = atoi(ent->d_name);
            proctab[i].run = 1;
        }
    }

    closedir(dir);

}

void *dstattask(void *arg)
{
    struct timeval tm;
    while(1) {
        tm.tv_sec=1;
        tm.tv_usec=0;
        if (select(1, 0, 0, 0, &tm) <0) ;
        daemon_stat();
    }
}

