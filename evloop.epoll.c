#define _GNU_SOURCE
#include "evloop.epoll.h"
#include "mem.h"
#include "khash.h"
#include <sys/types.h>
#include <poll.h>
#include <sys/epoll.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

// use of bitfields in flags field
// EPOLLERR - indicates the entry is dirty
// EPOLLET - indicates that we've added the entry to the kernel
// EPOLLIN - want read events
// EPOLLOUT - want write events
#define FLAG_DIRTY EPOLLERR

struct evloop_poll {
	struct evloop_poll *next;
	struct evwatch *rd;
	struct evwatch *wr;
	int fd;
	int flags;
};

KHASH_MAP_INIT_INT(evloop_poll, struct evloop_poll *)

///////////////////////
// Time helpers

static inline uint32_t get_ticks(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	uint64_t t64 = ts.tv_sec * 1000;
	t64 += ts.tv_nsec / 1000 / 1000;
	return (uint32_t)t64;
}

/////////////////////////
// Init/destroy

static pthread_key_t g_evloop_key;
static pthread_once_t g_evloop_once = PTHREAD_ONCE_INIT;

static void create_key(void)
{
	if (pthread_key_create(&g_evloop_key, NULL)) {
		abort();
	}
}

struct evloop *evloop_get(void)
{
	return pthread_getspecific(g_evloop_key);
}

void evloop_init(struct evloop *e)
{
	pthread_once(&g_evloop_once, &create_key);
	pthread_setspecific(g_evloop_key, e);
	memset(e, 0, sizeof(*e));
	e->pollers = kh_init(evloop_poll);
	e->efd = epoll_create1(EPOLL_CLOEXEC);
	e->now = get_ticks();
	size_t sigstksz;
#ifdef _SC_SIGSTKSZ
	sigstksz = sysconf(_SC_SIGSTKSZ);
#else
	sigstksz = SIGSTKSZ;
#endif
	e->sigstack = xmalloc(sigstksz);
	stack_t ss = {
		.ss_sp = e->sigstack,
		.ss_size = sigstksz,
	};
	sigaltstack(&ss, NULL);
}

void evloop_destroy(struct evloop *e)
{
	for (khint_t ii = 0; ii < kh_end(e->pollers); ii++) {
		if (kh_exist(e->pollers, ii)) {
			struct evloop_poll *p = kh_val(e->pollers, ii);
			close(p->fd);
			xfree(p);
		}
	}
	kh_destroy(evloop_poll, e->pollers);
	xfree(e->timers.v);
	close(e->efd);
	stack_t ss = {
		.ss_flags = SS_DISABLE,
	};
	sigaltstack(&ss, NULL);
	xfree(e->sigstack);
}

///////////////////
// registrations

static inline void mark_dirty(struct evloop *e, struct evloop_poll *p)
{
	if (!(p->flags & FLAG_DIRTY)) {
		p->flags |= FLAG_DIRTY;
		p->next = e->dirty_list;
		e->dirty_list = p;
	}
}

void ev_watch(struct evwatch *w, evfd_t fd, enum ev_event ev)
{
	assert(w->fn);
	struct evloop *e = evloop_get();
	int sts;
	khint_t ii = kh_put(evloop_poll, e->pollers, fd, &sts);
	struct evloop_poll **pp = &kh_val(e->pollers, ii);
	if (sts) {
		*pp = xcalloc(1, sizeof(**pp));
		(*pp)->fd = fd;
	}
	struct evloop_poll *p = *pp;
	w->poll_data = p;
	if (ev == EV_READ) {
		assert(!p->rd);
		p->rd = w;
	} else {
		assert(!p->wr);
		p->wr = w;
	}
	mark_dirty(e, p);
}

void ev_unwatch(struct evwatch *w)
{
	struct evloop_poll *p = w->poll_data;
	if (p == NULL) {
		return;
	}
	w->poll_data = NULL;
	w->fn = NULL;
	if (w == p->rd) {
		p->rd = NULL;
	} else {
		assert(w == p->wr);
		p->wr = NULL;
	}
	mark_dirty(evloop_get(), p);
}

void ev_close(evfd_t fd)
{
	struct evloop *e = evloop_get();
	khint_t ii = kh_get(evloop_poll, e->pollers, fd);
	if (ii != kh_end(e->pollers)) {
		struct evloop_poll *p = kh_val(e->pollers, ii);
		kh_del(evloop_poll, e->pollers, ii);
		p->fd = -1;
		p->rd = NULL;
		p->wr = NULL;
		// defer freeing the structure itself until we hit the cleanup
		// phase of the main loop. That way we don't access free'd
		// memory as we iterate over the epoll_event items
		mark_dirty(e, p);
	}

	// Don't need to remove from kernel using epoll_ctl.
	// It will be automaticallly removed upon close.
	close(fd);
}

///////////////////
// run

int ev_step_timers(struct evloop *e);

int evloop_step(struct evloop *e)
{
	int timeout_ms = ev_step_timers(e);
	if (!timeout_ms) {
		return 0;
	}

	// update the kernel with any changes
	// and free any evloop_poll structures no longer required

	for (struct evloop_poll *p = e->dirty_list; p != NULL;) {
		struct evloop_poll *next = p->next;
		p->flags &= ~FLAG_DIRTY;
		p->next = NULL;

		// use EPOLLET to indicate that we've told the kernel
		// about the fd

		int kernel = p->flags & (EPOLLHUP | EPOLLIN | EPOLLOUT);

		struct epoll_event ev;
		ev.data.ptr = p;
		ev.events = (p->rd ? (EPOLLIN | EPOLLHUP) : 0) |
			    (p->wr ? EPOLLOUT : 0);

		if (p->fd < 0) {
			xfree(p);
		} else if (ev.events && !(p->flags & EPOLLET)) {
			ev.events |= EPOLLET;
			epoll_ctl(e->efd, EPOLL_CTL_ADD, p->fd, &ev);
			p->flags |= EPOLLET;
		} else if (ev.events != kernel) {
			ev.events |= EPOLLET;
			epoll_ctl(e->efd, EPOLL_CTL_MOD, p->fd, &ev);
		}
		p = next;
	}
	e->dirty_list = NULL;

	// then get the list of events from the kernel

try_again:
	struct epoll_event ev[16];
	int n = epoll_wait(e->efd, ev, 16, timeout_ms);
	if (n < 0 && errno == EINTR) {
		goto try_again;
	} else if (n < 0) {
		return -1;
	}

	// the only call that should "take time" as such should be the
	// epoll_wait call. Everything else we can treat as happening
	// instantly.

	e->now = get_ticks();

	// Dispatch the events. After each call to evloop_wakeup, the
	// evloop_poll structure may change, but they won't be free'd as
	// that's only done after updating the kernel above.

	for (int i = 0; i < n; i++) {
		struct evloop_poll *p = ev[i].data.ptr;
		if (p->rd && (ev[i].events & (EPOLLIN | EPOLLHUP))) {
			p->rd->fn(p->rd);
		}
		if (p->wr && (ev[i].events & EPOLLOUT)) {
			p->wr->fn(p->wr);
		}
	}

	return 0;
}