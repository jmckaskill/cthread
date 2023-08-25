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
struct evwatch_data;

typedef void (*evtimer_fn)(struct evtimer *);
typedef void (*evwatch_fn)(struct evwatch *);

#ifdef _WIN32
#include "evloop.windows.h"
#elif defined __linux__
#include "evloop.epoll.h"
#else
#error
#endif

struct evtimer {
	struct heap_mod32_node heap;
	evtimer_fn fn;
};

struct evwatch {
	evwatch_fn fn;
	struct evwatch_data d;
};

void evloop_init(struct evloop *e);
void evloop_destroy(struct evloop *e);
int evloop_step(struct evloop *e);
struct evloop *evloop_get(void);

uint32_t ev_timeout(int msecs);
void ev_alarm(struct evtimer *t, uint32_t wakeup);
void ev_stop(struct evtimer *t);

void ev_connect(struct evwatch *w, evfd_t *pfd, const char *net,
		const char *addr);
void ev_accept(struct evwatch *w, evfd_t fd, evfd_t *pfd);
void ev_recv(struct evwatch *w, evfd_t fd, void *buf, size_t sz);
void ev_send(struct evwatch *w, evfd_t fd, const void *p, size_t sz);
int ev_finish(struct evwatch *w);
void ev_close(evfd_t fd);
