#include "evloop.windows.h"
#include "tls.h"
#include <windows.h>
#include <assert.h>
#include <limits.h>
#include <string.h>

static_assert(sizeof(((struct evwatch_data *)0)->overlappedl) ==
		      sizeof(struct OVERLAPPED),
	      "");

static_assert(alignof(((struct evwatch_data *)0)->overlappedl) ==
		      alignof(struct OVERLAPPED),
	      "");

static tls_key_t g_evloop;
static volatile long g_evloop_once;

void evloop_init(struct evloop *e)
{
	g_evloop = create_tls_key(&g_evloop_once);
	tls_set(g_evloop, e);
}

struct evloop *evloop_get(void)
{
	return tls_get(g_evloop);
}

void ev_recv(struct evwatch *w, evfd_t fd, void *buf, size_t sz)
{
	if (sz > INT_MAX) {
		sz = INT_MAX;
	}
	DWORD rd;
	OVERLAPPED *pol = (OVERLAPPED *)&w->d.ol;
	if (ReadFile(fd, buf, (DWORD)sz, &rd, pol)) {
		w->d.next = e->ready_list;
		e->ready_list = w;
		w->d.finished = (int)rd;
	} else {
		w->d.finished = -1;
	}
}

void ev_send(struct evwatch *w, evfd_t fd, const void *p, size_t sz)
{
	if (sz > INT_MAX) {
		sz = INT_MAX;
	}
	DWORD wr;
	OVERLAPPED *pol = (OVERLAPPED *)&w->d.ol;
	if (WriteFile(fd, buf, (DWORD)sz, &wr, pol)) {
		struct evloop *e = evloop_get();
		w->d.next = e->ready_list;
		e->ready_list = w;
		w->d.finished = (int)rd;
	} else {
		w->d.finished = -1;
	}
}

int ev_finish(struct evwatch *w)
{
	return w->d.finished;
}

void ev_close(evfd_t fd)
{
	// this will also cancel any pending IO and deregister the handle from
	// the io completion port
	CloseHandle(fd);
}