#include "cthread.h"
#include "evloop.h"
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct cthread_timer {
	struct evtimer timer;
	cthread_t *thread;
	void *arg;
};

static void timer_wakeup(struct evtimer *t)
{
	struct cthread_timer *ct = container_of(t, struct cthread_timer, timer);
	cthread_call(ct->thread, ct->arg);
}

static void do_sleep(int msecs)
{
	struct cthread_timer ct;
	ct.thread = cthread_self();
	ev_alarm(&ct.timer, ev_timeout(msecs), &timer_wakeup);
	cthread_yield(NULL);
}

static int test2_fn(void *p)
{
	cthread_t *c = cthread_self();
	(void)c;
	return 3;
}

static int test_func(void *p)
{
	cthread_t *c = cthread_self();
	(void)c;
	char *stack = malloc(4096);
	cthread_start(&test2_fn, NULL, stack, 4096);
	do_sleep(2000);
	do_sleep(1000);
	cthread_exit(2);
	return 2;
}

static int join_func(void *p)
{
	cthread_t *c = cthread_self();
	(void)c;
	cthread_t *tgt = p;
	cthread_join(tgt);
	return 0;
}

#if 0

struct cthread_watch {
	struct evwatch watch;
	cthread_t *thread;
	void *arg;
};

static void watch_wakeup(struct evwatch *w)
{
	struct cthread_watch *cw = container_of(w, struct cthread_watch, watch);
	cthread_call(cw->thread, cw->arg);
}

static int do_recv(int fd, char *buf, size_t sz, int msecs)
{
	int ret;

	struct cthread_timer ct;
	ct.thread = cthread_self();
	ct.arg = NULL;
	ev_alarm(&ct.timer, ev_timeout(msecs), &timer_wakeup);

	struct cthread_watch cw;
	cw.thread = cthread_self();
	cw.arg = &cw;
	ev_watch(&cw.watch, fd, EV_READ, &watch_wakeup);

	for (;;) {
		ret = recv(fd, buf, sz, MSG_DONTWAIT);
		if (ret < 0 && errno == EINTR) {
			continue;
		} else if (ret < 0 && errno == EWOULDBLOCK) {
			if (!cthread_yield(NULL)) {
				errno = ETIMEDOUT;
				ret = -1;
				break;
			}
			continue;
		} else {
			break;
		}
	}

	ev_stop(&ct.timer);
	ev_unwatch(&cw.watch);
	return ret;
}

static int read_from_input(void *p)
{
	for (;;) {
		char buf[128];
		int n = do_recv(0, buf, sizeof(buf), 1000);
		fprintf(stderr, "recv %d %.*s\n", n, (n > 0 ? n : 0), buf);
	}
	return 0;
}
#endif

int main(void)
{
	struct cthread_main m;
	cthread_main_init(&m);

	struct evloop ev;
	evloop_init(&ev);

	char *stack1 = malloc(4096);
	cthread_t *thrd1 =
		cthread_main_start(&m, &test_func, NULL, stack1, 4096);

	char *stack2 = malloc(4096);
	cthread_t *thrd2 =
		cthread_main_start(&m, &join_func, thrd1, stack2, 4096);

	// char *stack3 = malloc(16 * 1024);
	//  cthread_t *thrd3 = cthread_main_start(&m, &read_from_input, NULL,
	//  				      stack3, 16 * 1024);
	(void)thrd2;
	// (void)thrd3;
	while (m.num_threads) {
		evloop_step(&ev);
	}
	evloop_destroy(&ev);
	return 0;
}