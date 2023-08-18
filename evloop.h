#pragma once
#include "heap.h"
#include <stdint.h>

#ifdef _WIN32
typedef void *evfd_t;
#else
typedef int evfd_t;
#endif

struct evloop;
struct evtimer;
struct evwatch;

typedef void (*evtimer_fn)(struct evtimer *);
typedef void (*evwatch_fn)(struct evwatch *);

struct evtimer {
	struct heap_mod32_node heap;
	evtimer_fn fn;
};

enum ev_event {
	EV_READ = 0,
	EV_WRITE,
};

struct evwatch {
	evwatch_fn fn;
	void *poll_data;
};

void evloop_init(struct evloop *e);
void evloop_destroy(struct evloop *e);
int evloop_step(struct evloop *e);
struct evloop *evloop_get(void);

uint32_t ev_timeout(int msecs);
void ev_alarm(struct evtimer *t, uint32_t wakeup, evtimer_fn fn);
void ev_stop(struct evtimer *t);
void ev_watch(struct evwatch *w, evfd_t fd, enum ev_event ev, evwatch_fn fn);
void ev_unwatch(struct evwatch *w);
void ev_close(evfd_t fd);

#ifdef _WIN32
#else
#include "evloop.epoll.h"
#endif
