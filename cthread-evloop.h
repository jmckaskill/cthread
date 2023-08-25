#pragma once
#include "cthread.h"
#include "evloop.h"

struct cthread_timer {
	struct evtimer timer;
	cthread_t *thread;
	void *arg;
};

void cthread_alarm(struct cthread_timer *t, uint32_t wakeup, void *arg);
static inline void cthread_stop(struct cthread_timer *t)
{
	ev_stop(&t->timer);
}

struct cthread_watch {
	struct evwatch watch;
	cthread_t *thread;
	void *arg;
};

int cthread_recv(struct cthread_watch *w, evfd_t fd, void *buf, size_t sz);
int cthread_send(struct cthread_watch *w, evfd_t fd, const void *buf,
		 size_t sz);
void cthread_cancel(struct cthread_watch *w)
{
	ev_cancel(&w->watch);
}
