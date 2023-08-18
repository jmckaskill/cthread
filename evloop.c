#include "evloop.h"

uint32_t ev_timeout(int msecs)
{
	struct evloop *e = evloop_get();
	return e->now + msecs;
}

void ev_alarm(struct evtimer *t, uint32_t wakeup, evtimer_fn fn)
{
	struct evloop *e = evloop_get();
	t->fn = fn;
	heap_insert(&e->timers, &t->heap, wakeup);
}

void ev_stop(struct evtimer *t)
{
	if (t->heap.index != HEAP_INVALID) {
		struct evloop *e = evloop_get();
		heap_remove(&e->timers, t->heap.index);
		t->heap.index = HEAP_INVALID;
		t->fn = NULL;
	}
}

static int next_timeout(struct evloop *e)
{
	if (!e->timers.size) {
		return -1;
	}
	uint32_t deadline = e->timers.v[HEAP_HEAD].value;
	int32_t diff = (int32_t)(deadline - e->now);
	if (diff > 0) {
		return (int)diff;
	}
	return 0;
}

int ev_step_timers(struct evloop *e)
{
	int diff = next_timeout(e);
	if (diff) {
		return diff;
	}

	do {
		struct heap_mod32_node *n = e->timers.v[HEAP_HEAD].node;
		struct evtimer *t = container_of(n, struct evtimer, heap);
		heap_remove(&e->timers, HEAP_HEAD);
		t->fn(t);
	} while (next_timeout(e) == 0);

	return 0;
}