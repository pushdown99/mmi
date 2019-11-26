#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/termios.h>
#include <sys/signal.h>

#include "proc.h"

extern int errno;

/* Runs jobs in the background */
int
background()
{
	int pid, fd;
	int i;

	/* run it background */
	if((pid=fork())<0) {
		perror("background");
		return -1;
	}
	if(pid) exit(0);

	/* close all open file descriptor */
	for(i=0;i<NOFILE;i++) close(i);

	/* change current working directory */
	chdir("/");

	/* reset the file access creation mask */
	umask(0);

#if defined(BSD)
	/* control terminal */
	if((fd=open("/dev/tty",O_RDWR,0))>=0) {
		if (ioctl(fd, TIOCNOTTY, &tio)<0) perror("tty");
		close(fd);
	}

	/* deassociate from process group */
	setpgrp(0,getpid());

	/* ignore terminal I/O signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);
#else
	setpgrp();

	signal(SIGHUP, SIG_IGN);
	if(fork()!=0) exit(0);

	/* secondary child process continue as daemon */
#endif
	return 0;
}

int
exeproc(char* p,char* s,char* tag,char* envp[])
{
	char *argv[3];
    int pid,fd;

    pid = fork();
    if(pid==0) {
    	fd = open("/dev/null",O_RDWR);
    	dup2(fd,1);
    	dup2(fd,2);
        execl(p,s,tag,NULL);
		exit(0);
    }
    return pid;
}

int
lockpid(char *path,char *f,int signo)
{
	char s[BUFSIZ],cmd[BUFSIZ],*p;
	int pid,eol;
	FILE* fp;

    eol = strlen(path)-1;
    if(path[eol]=='/') path[eol]=0;

	
	sprintf(s,"%s/%s",path,f);
	if((p=strstr(s,".pid"))!=NULL) *p=0;
	strcat(s,".pid");

	if((fp=fopen(s,"r"))!=NULL) {
		if(fscanf(fp,"%d",&pid)) {
			fflush(fp);
			kill(pid, signo);
#if 0
			sprintf(cmd,"kill -%d %d",signo,pid);
			system(cmd);
#endif
		}
		fclose(fp);
	}
	if((fp=fopen(s,"w"))==NULL) return 0;
	fprintf(fp,"%d",getpid());
	fclose(fp);

	return 1;
}

int
unlockpid(char *path,char *f)
{
	char cmd[BUFSIZ],*p;

	if((p=strstr(f,".pid"))!=NULL) *p=0;
	sprintf(cmd,"rm -f %s/%s.pid",path,f);
	system(cmd);
	
	return 1;
}
