/*
 * Author : albam <albamc@gmail.com>
 * Copyright 2004 albam
 *
 * This program is NOT-free software; You can redistribute it
 * and/or modify it under the terms of the Albam's General Public
 * License (AGPL) as published by the albam.
 * 
 * Albam's General Public License (AGPL)
 * DO NOT USE OR MODIFY MY SOURCES!!!
 * 
 * Changes :
 *  - 2005/01/10 albam <albamc@gmail.com> : make initial version.
 *  - 2005/01/25 albam <albamc@gmail.com> : use global lock.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/timeb.h>
#include <netinet/in.h>
#include <pthread.h>

#include "list.h"
#include "timer.h"

#define TIMER_HASH_SIZE 0xffff
#define TIMER_KEY_SIZE 16

#define _list_for_each_safe_prev(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; pos != (head); \
		pos = n, n = pos->prev)

typedef struct thash {
	struct list_head 	list;
	int cnt;
	int dirty;
} thash_t;

struct timeb _internal_time;
static thash_t _thlist[TIMER_HASH_SIZE]; /* time hash */
static thash_t _khlist[TIMER_HASH_SIZE]; /* key hash */

int cb_fd = 0;
int cb_thr = 0;
int cb_tid = 0;
struct sockaddr_in cb_snd;
pthread_t t_timer;

/* task queue */
static pthread_t t_tqueue;
static struct list_head _tqueue;

/* locks */
static pthread_mutex_t time_lock;
static pthread_mutex_t hash_lock;
static pthread_mutex_t task_lock;
static pthread_cond_t task_cond;
static pthread_mutex_t tid_lock;

typedef struct tentry {
	struct list_head	tlist;
	struct list_head	klist;

	struct timeb 		t; 					/* expired timestamp */
	char 				k[TIMER_KEY_SIZE]; 	/* key */
	int 				n; 					/* key len */
	unsigned short 		p; 					/* callback here (port) */
	void* 				d; 					/* data */
	int					myint;
	void 				(*fn)(char*); 		/* callback function */
	void 				(*ifn)(int); 		/* callback function */
} tentry_t;

/*************************************************************************
 * HASH Function (internal)
 */
static void
_hinit(void)
{
	int i;
	for (i=0; i<TIMER_HASH_SIZE; i++) {
		INIT_LIST_HEAD(&_thlist[i].list);
		_thlist[i].cnt = 0;
		_thlist[i].dirty = 0;
		INIT_LIST_HEAD(&_khlist[i].list);
		_khlist[i].dirty = 0;
	}
	ftime(&_internal_time);
	INIT_LIST_HEAD(&_tqueue);
	pthread_mutex_init(&time_lock, NULL);
	pthread_mutex_init(&task_lock, NULL);
	pthread_mutex_init(&hash_lock, NULL);
	pthread_cond_init(&task_cond, NULL);
	pthread_mutex_init(&tid_lock, NULL);
}

#define THASH_TERM 20 /* 1/20 sec */
static inline unsigned int
_thfn(struct timeb* t)
{
	unsigned long int time;
	time = t->time & 0x0000ffff;
	time = time * 1000;
	time += t->millitm;
	return (time%TIMER_HASH_SIZE);
}

static inline unsigned int
_khfn(char* k, int l)
{
	switch(l) {
	case 1:
	{
		unsigned char _tmp;
		_tmp = *(unsigned char*)k;
		return (_tmp%TIMER_HASH_SIZE);
	}
	break;
	case 2:
	case 3:
	{
		unsigned short _tmp;
		_tmp = *(unsigned short*)k;
		return (_tmp%TIMER_HASH_SIZE);
	}
	break;
	default:
	{
		unsigned int _tmp;
		_tmp = *(unsigned int*)k;
		return (_tmp%TIMER_HASH_SIZE);
	}
	break;
	}
	return 0;
}

static inline void
_thash(tentry_t* te)
{
	thash_t* th = &_thlist[_thfn(&te->t)];
	tentry_t* tp;
	struct list_head *l;
	/* reverse sorted by expire-time */
	list_for_each(l, &th->list) {
		tp = list_entry(l, tentry_t, tlist);
		if (tp->t.time < te->t.time) break;
		if (tp->t.time == te->t.time &&
				tp->t.millitm <= te->t.millitm) break;
	}
	/* add list */
	list_add_tail(&te->tlist, l);
	th->cnt++;
	th->dirty++;
}

static inline void
_khash(tentry_t* te)
{
	thash_t* th = &_khlist[_khfn(te->k, te->n)];
	list_add(&te->klist, &th->list);
	th->cnt++;
	th->dirty++;
}

static inline void
_tunhash(tentry_t* te)
{
	thash_t* th = &_thlist[_thfn(&te->t)];
	list_del(&te->tlist);
	th->cnt--;
}

static inline void
_kunhash(tentry_t* te)
{
	thash_t* th = &_khlist[_khfn(te->k, te->n)];
	list_del(&te->klist);
	th->cnt--;
}

static tentry_t*
_kfind(char* k, int n)
{
	thash_t* th;
	tentry_t* te;
	struct list_head* l;

	th = &_khlist[_khfn(k, n)];
	list_for_each(l, &th->list) {
		te = list_entry(l, tentry_t, klist);
		if (memcmp(&te->k[0], k, n)) continue;
		return te;
	}
	return NULL;
}

static void
_tsync(int time, int mtime)
{
	int i;
	struct list_head thead, *l, *tl;
	tentry_t* te;
	int tmp;
	
	pthread_mutex_lock(&hash_lock);
	INIT_LIST_HEAD(&thead);
	for (i=0; i<TIMER_HASH_SIZE; i++) {
		list_for_each_safe(l, tl, &_thlist[i].list) {
			te = list_entry(l, tentry_t, tlist);
			/* recalculate second */
			te->t.time += time;
			/* recalculate milli-second */
			tmp = te->t.time + mtime;
			if (tmp < 0) {
				te->t.time--;
				tmp += 1000;
			}
			te->t.millitm = tmp;
			list_del_init(l);
			list_add(l, &thead);
		}
	}
	list_for_each_safe(l, tl, &thead) {
		list_del(l);
		_thash(list_entry(l, tentry_t, tlist));
	}
	pthread_mutex_unlock(&hash_lock);
}

/**************************************************************************
 * HASH Interface Function
 */
static tentry_t*
timer_add(short pt, char* k, int n, int t)
{
	tentry_t* te;
	
	/* key length check */
	if (n > TIMER_KEY_SIZE)
		return NULL;

	/* alloc */
	te = (tentry_t*)malloc(sizeof(tentry_t));
	if (!te)
		return NULL;
	memset(te, 0, sizeof(tentry_t));
	INIT_LIST_HEAD(&te->tlist);
	INIT_LIST_HEAD(&te->klist);

	pthread_mutex_lock(&time_lock);
	memcpy(&te->t, &_internal_time, sizeof(struct timeb));
	pthread_mutex_unlock(&time_lock);

	te->t.time += (t/1000);
	te->t.millitm += (t%1000);
	if (te->t.millitm > 999) {
		te->t.time++;
		te->t.millitm -= 1000;
	}
	memcpy(&te->k[0], k, n);
	te->n = n;
	te->p = pt;
	te->myint = 0;
	te->d = NULL;
	te->fn = NULL;
	te->ifn = NULL;

	/* hashing */
	pthread_mutex_lock(&hash_lock);
	/* find same key entry */
	if (_kfind(k, n)) {
		free(te);
		te = NULL;
	} else {
		_thash(te);
		_khash(te);
	}
	pthread_mutex_unlock(&hash_lock);

	return te;
}

static void
timer_del(char* k, int n)
{
	tentry_t* te;
	pthread_mutex_lock(&hash_lock);
	te = _kfind(k, n);
	if (te) {
		_tunhash(te);
		_kunhash(te);
	}
	pthread_mutex_unlock(&hash_lock);
	free(te);
}

static tentry_t*
timer_cb_add(int tid, void (*fn)(char*), void* d, int t)
{
	tentry_t* te;

	/* alloc */
	te = (tentry_t*)malloc(sizeof(tentry_t));
	if (!te)
		return NULL;

	memset(te, 0, sizeof(tentry_t));
	INIT_LIST_HEAD(&te->tlist);
	INIT_LIST_HEAD(&te->klist);

	pthread_mutex_lock(&time_lock);
	memcpy(&te->t, &_internal_time, sizeof(struct timeb));
	pthread_mutex_unlock(&time_lock);

	te->t.time += (t/1000);
	te->t.millitm += (t%1000);
	if (te->t.millitm > 999) {
		te->t.time++;
		te->t.millitm -= 1000;
	}
	memcpy(&te->k[0], &tid, sizeof(int));
	te->n = sizeof(int);
	te->p = 0;
	te->d = d;
	te->myint = 0;
	te->fn = fn;
	te->ifn = NULL;

	/* hashing */
	pthread_mutex_lock(&hash_lock);
	/* find same key entry */
	if (_kfind((char*)&tid, sizeof(tid))) {
		free(te);
		te = NULL;
	} else {
		_thash(te);
		_khash(te);
	}
	pthread_mutex_unlock(&hash_lock);

	return te;
}

static tentry_t*
timer_cb_iadd(int tid, void (*fn)(int), int myint, int t)
{
	tentry_t* te;

	/* alloc */
	te = (tentry_t*)malloc(sizeof(tentry_t));
	if (!te)
		return NULL;

	memset(te, 0, sizeof(tentry_t));
	INIT_LIST_HEAD(&te->tlist);
	INIT_LIST_HEAD(&te->klist);

	pthread_mutex_lock(&time_lock);
	memcpy(&te->t, &_internal_time, sizeof(struct timeb));
	pthread_mutex_unlock(&time_lock);

	te->t.time += (t/1000);
	te->t.millitm += (t%1000);
	if (te->t.millitm > 999) {
		te->t.time++;
		te->t.millitm -= 1000;
	}
	memcpy(&te->k[0], &tid, sizeof(int));
	te->n = sizeof(int);
	te->p = 0;
	te->d = NULL;
	te->myint = myint;
	te->fn = NULL;
	te->ifn = fn;

	/* hashing */
	pthread_mutex_lock(&hash_lock);
	/* find same key entry */
	if (_kfind((char*)&tid, sizeof(tid))) {
		free(te);
		te = NULL;
	} else {
		_thash(te);
		_khash(te);
	}
	pthread_mutex_unlock(&hash_lock);

	return te;
}

static void 
timer_cb_del(int tid)
{
	tentry_t* te;
	pthread_mutex_lock(&hash_lock);
	te = _kfind((char*)&tid, sizeof(tid));
	if (te) {
		_tunhash(te);
		_kunhash(te);
		free(te);
	}
	pthread_mutex_unlock(&hash_lock);
}

static void*
timer_callback(void* args)
{
	tentry_t* te;
	struct list_head* l, *tl;
	while(1) {
		pthread_mutex_lock(&task_lock);
		pthread_cond_wait(&task_cond, &task_lock);
		list_for_each_safe(l, tl, &_tqueue) {
			te = list_entry(l, tentry_t, tlist);
			if (te->p) {
				cb_snd.sin_port = htons(te->p);
				sendto(cb_fd, te->k, te->n, 0
						, (struct sockaddr*)&cb_snd, sizeof(cb_snd));
			}
			if (te->fn) te->fn((char*)te->d);
			if (te->ifn) te->ifn(te->myint);

			list_del(&te->tlist);
			free(te);
		}
		pthread_mutex_unlock(&task_lock);
	}
}

#define MAX_TIME_DIFF 3 /* time difference - internal time from external time. (sec) */
static void*
timer_check(void* args)
{
	struct timeval tv;
	struct timeb _external_time;
	int time_diff, i;
	int phv, chv, ret = 0;
	struct list_head *l, *tl;
	tentry_t* te;

	ftime(&_external_time);
	phv = _thfn(&_external_time);
	while (1) {
		tv.tv_sec = 0;
		tv.tv_usec = 100; /* 100 usec == 0.1 msec */

		/* sleep for the millisec-time */
		if (select(0, 0, 0, 0, &tv) < 0) {
			perror("timer check select");
		}

		/* syncronize time (external-internal) */
		ftime(&_external_time);
		pthread_mutex_lock(&time_lock);
		time_diff = difftime(_internal_time.time, _external_time.time);
		if (time_diff > MAX_TIME_DIFF || time_diff < (-MAX_TIME_DIFF)) {
			/* Mother fucker!!! anyone starts fucking NTP? */
			/* syncronization whole tentry */
			struct timeb _dt;
			_dt.time = _external_time.time - _internal_time.time;
			_dt.millitm = _external_time.millitm - _internal_time.millitm;
			_tsync(_external_time.time-_internal_time.time, 
					_external_time.millitm-_internal_time.millitm);
			ftime(&_external_time);
			phv = _thfn(&_external_time);
		}
		_internal_time.time = _external_time.time;
		_internal_time.millitm = _external_time.millitm;
		chv = _thfn(&_internal_time);
		pthread_mutex_unlock(&time_lock);

		pthread_mutex_lock(&task_lock);
		pthread_mutex_lock(&hash_lock);
		for (i = phv; i != chv; i = (i+1)%TIMER_HASH_SIZE) {
			_list_for_each_safe_prev(l, tl, &_thlist[i].list) {
				te = list_entry(l, tentry_t, tlist);
				if ((te->t.time < _internal_time.time) ||
					((te->t.time == _internal_time.time) && 
					(te->t.millitm <= _internal_time.millitm))
				) {
					list_del_init(&te->tlist);
					list_add(&te->tlist, &_tqueue);
					_kunhash(te);		
					ret++;
					continue;
				}
				break;
			}
		}
		pthread_mutex_unlock(&hash_lock);
		if (ret) {
			pthread_cond_signal(&task_cond);
			ret = 0;
		}
		pthread_mutex_unlock(&task_lock);
		phv = chv;
	}
}

/**************************************************************************
 *  * Externally Referenced Function
 *   */
void
init_timer(void)
{
	if (!cb_thr) {
		_hinit();
		cb_fd = udpsock(0);
		if (cb_fd <= 0) {
			perror("callback socket open");
			return;
		}
		bzero((char*)&cb_snd, sizeof(cb_snd));
		cb_snd.sin_family = AF_INET;
		cb_snd.sin_addr.s_addr = inet_addr("127.0.0.1");
		pthread_create(&t_timer, NULL, timer_check, NULL);
		pthread_create(&t_tqueue, NULL, timer_callback, NULL);
		cb_thr = 1;
	}
}

int
set_timeout(unsigned short port, char* key, int len, int msec)
{
	int ret;
	if (!cb_thr) {
		_hinit();
		cb_fd = udpsock(0);
		if (cb_fd <= 0) {
			perror("callback socket open");
			return -1;
		}
		bzero((char*)&cb_snd, sizeof(cb_snd));
		cb_snd.sin_family = AF_INET;
		cb_snd.sin_addr.s_addr = inet_addr("127.0.0.1");
		pthread_create(&t_timer, NULL, timer_check, NULL);
		pthread_create(&t_tqueue, NULL, timer_callback, NULL);
		cb_thr = 1;
	}
	ret = (timer_add(port, key, len, msec))?0:-1;
	return ret;
}

void
del_timeout(char* key, int len)
{
	if (!cb_thr) {
		_hinit();
		cb_fd = udpsock(0);
		if (cb_fd <= 0) {
			perror("callback socket open");
			return;
		}
		bzero((char*)&cb_snd, sizeof(cb_snd));
		cb_snd.sin_family = AF_INET;
		cb_snd.sin_addr.s_addr = inet_addr("127.0.0.1");
		pthread_create(&t_timer, NULL, timer_check, NULL);
		pthread_create(&t_tqueue, NULL, timer_callback, NULL);
		cb_thr = 1;
	}
	timer_del(key, len);
}

void
init_cb_timer(void)
{
	if (!cb_thr) {
		_hinit();
		pthread_create(&t_timer, NULL, timer_check, NULL);
		pthread_create(&t_tqueue, NULL, timer_callback, NULL);
		cb_thr = 1;
	}
}

int
set_cb_timeout(void (*func)(char*), void* data, unsigned int msec)
{
	unsigned int _tid;
	if (!cb_thr) {
		_hinit();
		pthread_create(&t_timer, NULL, timer_check, NULL);
		pthread_create(&t_tqueue, NULL, timer_callback, NULL);
		cb_thr = 1;
	}
	pthread_mutex_lock(&tid_lock);
	_tid = (++cb_tid)?cb_tid:++cb_tid;
	pthread_mutex_unlock(&tid_lock);
	timer_cb_add(_tid, func, data, msec);
	return _tid;
}

int
set_cb_itimeout(void (*func)(int), int myint, unsigned int msec)
{
	unsigned int _tid;
	if (!cb_thr) {
		_hinit();
		pthread_create(&t_timer, NULL, timer_check, NULL);
		pthread_create(&t_tqueue, NULL, timer_callback, NULL);
		cb_thr = 1;
	}
	pthread_mutex_lock(&tid_lock);
	_tid = (++cb_tid)?cb_tid:++cb_tid;
	pthread_mutex_unlock(&tid_lock);
	timer_cb_iadd(_tid, func, myint, msec);
	return _tid;
}

void
del_cb_timeout(int tid)
{
	if (!cb_thr) {
		_hinit();
		pthread_create(&t_timer, NULL, timer_check, NULL);
		pthread_create(&t_tqueue, NULL, timer_callback, NULL);
		cb_thr = 1;
	}
	timer_cb_del(tid);
}


