#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "bufq.h"

pthread_mutex_t bufq_cond_mutex = PTHREAD_MUTEX_INITIALIZER;

umsg_t* alloc_umsg(int n)
{
	umsg_t *mp;
	ensure(n,return NULL);
	if(mp=(umsg_t*)malloc(sizeof(umsg_t))) {
		mp->prev 	= NULL;
		mp->next 	= NULL;
		mp->ct		= time(0);
		mp->n 		= n;
		mp->p 		= (char*)malloc(n);
		mp->prv		= NULL;
	}
	return mp;
}

umsg_t* dupl_umsg(umsg_t* p)
{
	umsg_t *mp;
	ensure(p,return NULL);

	if(mp=(umsg_t*)malloc(sizeof(umsg_t))) {
		mp->prev 	= NULL;
		mp->next 	= NULL;
		mp->ct		= p->ct;
		mp->n 		= p->n;
		mp->p 		= (char*)malloc(p->n);
		mp->prv		= NULL;
		memcpy(mp->p,p->p,p->n);
	}
	return mp;
}

umsg_t* copy_umsg(char *p,int n1,int n2)
{
	umsg_t *mp;
	ensure(p,return NULL);
	ensure(n1,return NULL);
	ensure(n2,return NULL);
	if(mp=(umsg_t*)malloc(sizeof(umsg_t))) {
		mp->prev 	= NULL;
		mp->next 	= NULL;
		mp->ct		= time(0);
		mp->n 		= n1;
		if(mp->p=(char*)malloc(n1)) {
			memcpy(mp->p,p,n2);
		}
		mp->prv 	= NULL;
	}
	return mp;
}

void free_umsg(umsg_t *mp)
{
	ensure(mp,return);
	if(mp->p) free(mp->p);
	if(mp->prv) free(mp->prv);
	free(mp);	
}

void bufq_init(bufq_t *q)
{
	ensure(q,return);
	q->q_head	= NULL;
	q->q_tail	= NULL;
	q->q_msgs	= 0;
	q->q_count	= 0;
}

size_t bufq_length(bufq_t* q)
{
	ensure(q,return 0);
	return q->q_msgs;
}

size_t bufq_size(bufq_t* q)
{
	ensure(q,return 0);
	return q->q_count;
}

umsg_t* bufq_head(bufq_t* q)
{
	ensure(q,return NULL);
	return q->q_head;
}

umsg_t* bufq_tail(bufq_t* q)
{
	ensure(q,return NULL);
	return q->q_head;
}

void __bufq_add(bufq_t* q,umsg_t *mp)
{
	ensure(q,return);
	ensure(mp,return);
	q->q_msgs++;
	q->q_count += mp->n;
}

void __bufq_sub(bufq_t* q,umsg_t *mp)
{
	ensure(q,return);
	ensure(mp,return);
	if(q->q_count>=mp->n) q->q_count -= mp->n;
	else q->q_count = 0;
	if(!(--q->q_msgs)) q->q_count = 0;
}

void bufq_queue(bufq_t* q,umsg_t *mp)
{
	ensure(q,	return);
	ensure(mp,	return);
	if(mp->prev=q->q_tail) mp->prev->next = mp;
	else q->q_head = mp;
	mp->next	= NULL;
	q->q_tail 	= mp;
	__bufq_add(q,mp);
}

void bufq_queue_head(bufq_t* q,umsg_t *mp)
{
	ensure(q,return);
	ensure(mp,return);
	if(mp->next=q->q_head) mp->next->prev = mp;
	else q->q_tail = mp;
	mp->prev 	= NULL;
	q->q_head	= mp;
	__bufq_add(q,mp);
}

void bufq_insert(bufq_t* q,umsg_t *mp,umsg_t *np)
{
	ensure(q,return);
	ensure(mp,return);
	ensure(np,return);
	if(np->prev=mp->prev) mp->prev->next = np;
	else q->q_head = np;
	mp->prev = np;
	np->next = mp;
	__bufq_add(q,np);
}

void bufq_append(bufq_t* q,umsg_t *mp,umsg_t *np)
{
	ensure(q,return);
	ensure(mp,return);
	ensure(np,return);
	if(np->next=mp->next) mp->next->prev = np;
	else q->q_tail = np;
	mp->next = np;
	np->prev = mp;
	__bufq_add(q,np);
}

umsg_t* bufq_dequeue(bufq_t* q)
{
	umsg_t *mp;
	ensure(q,return NULL);
	if(mp=q->q_head) {
		if(q->q_head=mp->next) mp->next->prev = NULL;
		else q->q_tail 	= NULL;
		mp->next		= NULL;
		mp->prev		= NULL;
		__bufq_sub(q,mp);
	}
	return mp;
}

umsg_t* bufq_unlink(bufq_t* q,umsg_t *mp)
{
	ensure(q,return NULL);
	ensure(mp,return NULL);
	if(mp->next) mp->next->prev = mp->prev;
	else q->q_tail = mp->prev;
	if(mp->prev) mp->prev->next = mp->next;
	else q->q_head = mp->next;
	mp->next = NULL;
	mp->prev = NULL;
	__bufq_sub(q,mp);
	return mp;
}

void bufq_freehead(bufq_t* q)
{
	ensure(q,return);
	if(q->q_head) FREEM(bufq_dequeue(q));
}

void bufq_purge(bufq_t* q)
{
	ensure(q,return);
	pthread_mutex_lock(&bufq_cond_mutex);
	while(q->q_head) FREEM(bufq_dequeue(q));
	pthread_mutex_unlock(&bufq_cond_mutex);
}

void bufq_supply(bufq_t* q,umsg_t* mp)
{
	umsg_t *md=mp,*md_next;
	ensure(q,return);
	ensure(mp,return);
	while(md) {
		md_next = mp->next;
		bufq_queue(q,md);
		md = md_next;
	}
}

umsg_t* bufq_resupply(bufq_t* q,umsg_t* mp,int maxsize,int maxcount)
{
	ensure(q,return NULL);
	ensure(mp,return NULL);
	if(bufq_length(q)>maxcount || bufq_size(q)>maxsize) return mp;
	bufq_supply(q,mp);
	return NULL;
}

/**
*/

void freechunks(umsg_t *mp)
{
	umsg_t *dp,*dp_next;
	ensure(mp,return);
	for(dp=mp;dp;dp_next=dp->next,FREEM(dp),dp=dp_next);
}







