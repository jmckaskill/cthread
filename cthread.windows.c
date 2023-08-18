#include "cthread.h"
#include <windows.h>

static DWORD g_tls;

int cthread_init(void)
{
	g_tls = TlsAlloc();
	return g_tls == TLS_OUT_OF_INDEXES;
}

int cthread_create(cthread_t *c, int stack_size, cthread_fn fn)
{
	if (!TlsGetValue(g_tls)) {
		ConvertThreadToFiber(NULL);
	}
}