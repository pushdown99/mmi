#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "__ipc.h"
#include "__time.h"

typedef struct msgq {
    long mtype;
    char mtext[BUFSIZ];
} msgq_t;

void
initq(const key_t key)
{
    msgq_t q;
    struct msqid_ds stat;
    int qid;

    qid = msgget(key, 0666 | IPC_CREAT);
    msgctl(qid,IPC_RMID,&stat);
}

int
recvq(const key_t key, char *s)
{
    msgq_t q;
    int qid, n;

    qid = msgget(key, 0666 | IPC_CREAT);
    n = msgrcv(qid, &q, BUFSIZ, 0, MSG_NOERROR);
    memcpy(s, q.mtext, BUFSIZ);

    return n;
}

int
sendq(const key_t key, char *s, int len)
{
    msgq_t q;
    int qid, n;

    qid = msgget(key, 0666 | IPC_CREAT);
    q.mtype = 1;
    memcpy(q.mtext, s, len);
    n=msgsnd(qid, &q, len, IPC_NOWAIT);

    return n;
}

/*
    shared memory get & attach
*/
char*
memget(key_t key, int size)
{
    char *p;
    int id;

    if((id=shmget(key,size,0666 | IPC_CREAT))<0) {
        return NULL;
    }
    if((p=(char*)shmat(id,0,0))==(void*)-1) {
        return NULL;
    }

    return p;
}

/*
    shared memory free
    : one process program use this
*/
int
memfree(key_t key)
{
    struct shmid_ds buf;
    int id;

    if((id=shmget(key,0,IPC_CREAT))<0) {
        return 0;
    }
    shmctl(id,IPC_RMID,&buf);
	return 1;
}

