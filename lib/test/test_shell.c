#include <stdio.h>
#include <signal.h>

#include "shell.h"

void procup(char* name)
{
	printf("[up] %s \n",name); 
}

void procdn(char* name)
{
	printf("[dn] %s \n",name); 
}

int init()
{
    char path[BUFSIZ];

    proc_cb_up = procup;
    proc_cb_dn = procdn;

    return proc_conf("proc.conf","ariatech");
}

void term(int signo)
{
	proc_quit();
	printf("bye.\n");
	exit(0);
}

int main()
{
	/* signal handler */
	signal(SIGINT,	term);
	signal(SIGHUP,	term);
	signal(SIGTERM,	term);

	printf("Runnable process started. \n");
	init();

	while(1) sleep(1);
}

