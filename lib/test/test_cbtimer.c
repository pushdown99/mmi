#include <stdio.h>
#include <sys/timeb.h>

#define MAXCB   10

void cbfn(char* d)
{
    struct timeb t;

    ftime(&t);
    printf("%d.%03d : (-) %s \n",t.time,t.millitm,d);
}

int main()
{
    int     tid[MAXCB],i;
    char    p[MAXCB][20];
    struct timeb t;

    for(i=0;i<MAXCB;i++) {
        ftime(&t);
        sprintf(p[i],"Hello-%02d",i);
        tid[i] = set_cb_timeout(cbfn,p[i],1000*(i+1));
        printf("%d.%03d : (+) %s (callback after %7.3f sec)\n",t.time,t.millitm,p[i],(float)1000*(i+1)/1000);
    }

    while(1) sleep(1);
}

