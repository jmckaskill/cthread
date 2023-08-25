#include "cthread-evloop.h"
#include <stdatomic.h>
static void timer_wakeup(struct evtimer *t)
{
	struct cthread_timer *ct = container_of(t, struct cthread_timer, timer);
	cthread_call(ct->thread, ct->arg);
}

void cthread_alarm(struct cthread_timer *t, uint32_t wakeup, void *arg)
{
	t->timer.fn = &timer_wakeup;
	t->arg = arg;
	t->thread = cthread_self();
	ev_alarm(&t->timer, wakeup);
}

static void fd_wakeup(struct evwatch *w)
{
	struct cthread_watch *cw = container_of(w, struct cthread_watch, watch);
	cthread_call(cw->thread, cw->arg);
}

int cthread_recv(struct cthread_watch *w, evfd_t fd, void *buf, size_t sz)
{
	w->watch.fn = &fd_wakeup;
	w->thread = cthread_self();
}

int cthread_send(struct cthread_watch *w, evfd_t fd, const void *buf,
		 size_t sz);
void cthread_cancel(struct cthread_watch *w)
{
	ev_cancel(&w->watch);
}
