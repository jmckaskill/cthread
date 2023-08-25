#pragma once
#include "evloop.h"
#include "heap.h"
#include <stdint.h>

typedef struct kh_evloop_poll_s kh_evloop_poll_t;
struct evloop_poll;

struct evloop {
	struct evloop_poll *dirty_list;
	kh_evloop_poll_t *pollers;
	struct heap_mod32 timers;
	uint32_t now;
	int efd;
	char *sigstack;
};

struct evwatch_data {
	struct evloop_poll *poll;
};

static inline uint32_t timeout(struct evloop *e, int msecs)
{
	return e->now + msecs;
}