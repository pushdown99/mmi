#include <stdio.h>
#include <sys/timeb.h>

#define MAXCB   10

int tid;

void cbfn(char* d)
{
    struct timeb t;

    ftime(&t);
    printf("%d.%03d : (-) %s \n",t.time,t.millitm,d);
}

int main()
{
    int     tick=10;
    char    p[20];
    struct timeb t;

    ftime(&t);
    sprintf(p,"Hello-%02d",tick);
    tid = set_cb_timeout(cbfn,p,1000*tick);
    printf("%d.%03d : (+) %s (callback after %3d sec)\n",t.time,t.millitm,p,tick);

    while(1) sleep(1);
}

