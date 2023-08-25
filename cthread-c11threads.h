#pragma once
#include "cthread.h"
#include <stdatomic.h>
#include <stdlib.h>

typedef struct {
	cthread_t *waitlist;
} ct_mtx_t;

typedef struct {
	cthread_t *waitlist;
	ct_mtx_t *lock;
} ct_cnd_t;

typedef cthread_t *ct_thrd_t;
typedef cthread_fn ct_thrd_start_t;

#define ct_thrd_success 0
#define ct_thrd_error -1
#define ct_thrd_nomem -2
#define ct_thrd_timedout 1

static inline int ct_thrd_create(ct_thrd_t *thr, ct_thrd_start_t fn, void *arg)
{
	char *buf = malloc(4096);
	if (!buf) {
		return ct_thrd_nomem;
	}
	*thr = cthread_start(fn, arg, buf, 4096);
	return 0;
}

static inline int ct_thrd_equal(ct_thrd_t a, ct_thrd_t b)
{
	return a == b;
}

static inline ct_thrd_t ct_thrd_current(void)
{
	return cthread_self();
}

static inline int ct_thrd_sleep(const struct timespec *d, struct timespec *rem)
{
	if (rem) {
		rem->tv_sec = 0;
		rem->tv_nsec = 0;
	}
	struct cthread_timer t;
	int ms = d->tv_sec * 1000;
	ms += d->tv_nsec / 1000 / 1000;
	cthread_alarm(&t, ev_timeout(ms), NULL);
	cthread_yield();
	return 0;
}

static inline void ct_thrd_yield(void)
{
	cthread_yield();
}

static inline void ct_thrd_exit(int code)
{
	cthread_exit((uintptr_t)code);
}

static inline int ct_thrd_detach(ct_thrd_t t)
{
	cthread_set_dtor(t, &free, )
}