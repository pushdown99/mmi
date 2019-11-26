#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/timeb.h>
#include <netinet/in.h>
#include <pthread.h>

pthread_t t_readit;

#define LOCAL_PORT  10001

void *readit(void* arg)
{
    struct sockaddr_in rcv;
    struct timeval  timeout;
    struct timeb    t;
    char    s[BUFSIZ];
    fd_set  fds,rfds;
    int     fd,ret,len,nbyte;

    fd = udpsock(LOCAL_PORT);

    FD_ZERO(&fds);
    FD_SET(fd,&fds);

    while(1) {
        timeout.tv_sec  = 0;
        timeout.tv_usec = 1000; /* 1000 usec = 1 msec */

        rfds = fds;
        ret = select(fd+1,&rfds,NULL,NULL,&timeout);
        if(ret<=0) continue;

        if(FD_ISSET(fd,&rfds)) {
            len = sizeof(rcv);
            nbyte = recvfrom(fd,s,BUFSIZ,0,(struct sockaddr*)&rcv,&len);
            ftime(&t);
            printf("%d.%03d (R) : %s %d\n"
                ,(int)t.time
                ,(int)t.millitm
                ,s
                ,nbyte
            );
        }
    }
}

void set_t(char *s,int msec)
{
    struct timeb t;

    ftime(&t); printf("%d.%03d (S) : %s (%d)\n",t.time,t.millitm,s,msec);
    set_timeout(LOCAL_PORT,s,strlen(s),msec);
}

void del_t(char *s)
{
    struct timeb t;

    ftime(&t); printf("%d.%03d (-) : %s \n",t.time,t.millitm,s);
    del_timeout(s,strlen(s));
}

int main()
{

    printf("%x \n",getip("hme0"));

    pthread_create(&t_readit,NULL,readit,NULL);
    set_t("Hello-001",  1);
    set_t("Hello-002",  2);
    set_t("Hello-003",  3);
    set_t("Hello-004",  4);
    set_t("Hello-005",  5);
    set_t("Hello-006",  6);
    set_t("Hello-007",  7);
    set_t("Hello-008",  8);
    set_t("Hello-009",  9);
    set_t("Hello-010", 10);
    set_t("Hello-011", 11);
    set_t("Hello-012", 12);
    set_t("Hello-013", 13);
    set_t("Hello-014", 14);
    set_t("Hello-015", 15);
    set_t("Hello-016", 16);
    set_t("Hello-017", 17);
    set_t("Hello-018", 18);
    set_t("Hello-019", 19);
    set_t("Hello-020", 20);
    set_t("Hello-021", 21);
    set_t("Hello-022", 22);
    set_t("Hello-013", 23);
    set_t("Hello-024", 24);
    set_t("Hello-025", 25);
    set_t("Hello-026", 26);
    set_t("Hello-027", 27);
    set_t("Hello-028", 28);
    set_t("Hello-029", 29);
    set_t("Hello-030", 30);

    del_t("Hello-011");
    del_t("Hello-021");

    while(1) sleep(1);
}


