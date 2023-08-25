#include "cthread.h"
#include "mem.h"
#include <stddef.h>
#include <assert.h>
#include <string.h>

#ifdef _WIN32
#define _WIN32_WINNT 0x0600
#include <windows.h>
#else
#include <pthread.h>
#endif

////////////////////
// assembly functions

// void *cthread_asm_start(void *udata, cthread_t *dst,
// 			struct cthread_stack *psavestack, cthread_fn fn);
void *cthread_asm_switch(void *udata, void *loadstack, void **psavestack,
			 struct cthread_caller *pcaller);
uintptr_t cthread_asm_altstack(uintptr_t arg0, uintptr_t arg1, void *pload_sp,
			       cthread_altstack_fn fn);

extern void __morestack(void);

/////////////////////
// current thread pointer

#ifdef _WIN32
static DWORD g_cthread_key;
static volatile long g_cthread_once = 0;
static void do_init(void)
{
	g_cthread_key = TlsAlloc();
	if (g_cthread_key == TLS_OUT_OF_INDEXES) {
		abort();
	}
}
void create_tls_key(void)
{
	if (InterlockedCompareExchange(&g_cthread_once, 1, 0) == 0) {
		do_init();
		InterlockedExchange(&g_cthread_once, 2);
	} else {
		while (g_cthread_once == 1) {
			Sleep(0);
		}
	}
}

static inline void set_thread(cthread_t *c)
{
	TlsSetValue(g_cthread_key, c);
}

cthread_t *cthread_self(void)
{
	return TlsGetValue(g_cthread_key);
}
#else
static pthread_key_t g_cthread_key;
static pthread_once_t g_cthread_once = PTHREAD_ONCE_INIT;
static void do_init(void)
{
	if (pthread_key_create(&g_cthread_key, NULL)) {
		abort();
	}
}
static void create_tls_key(void)
{
	pthread_once(&g_cthread_once, &do_init);
}

static inline void set_thread(cthread_t *c)
{
	pthread_setspecific(g_cthread_key, c);
}

cthread_t *cthread_self(void)
{
	return pthread_getspecific(g_cthread_key);
}

#endif

void cthread_main_init(struct cthread_main *m)
{
	memset(m, 0, sizeof(*m));
	create_tls_key();
}

//////////////////////////
// altstack allocators

uintptr_t cthread_altstack(cthread_altstack_fn fn, uintptr_t arg0,
			   uintptr_t arg1)
{
	cthread_t *c = cthread_self();
	if (c) {
		set_thread(NULL);
		uintptr_t ret =
			cthread_asm_altstack(arg0, arg1, c->main->stack, fn);
		set_thread(c);
		return ret;
	} else {
		return fn(arg0, arg1);
	}
}

void *xmalloc(size_t sz)
{
	void *p = (void *)cthread_altstack((cthread_altstack_fn) & (malloc), sz,
					   0);
	if (!p) {
		abort();
	}
	return p;
}

void *xcalloc(size_t num, size_t sz)
{
	void *p = (void *)cthread_altstack((cthread_altstack_fn) & (calloc),
					   num, sz);
	if (!p) {
		abort();
	}
	return p;
}

void *xrealloc(void *p, size_t sz)
{
	p = (void *)cthread_altstack((cthread_altstack_fn) & (realloc),
				     (uintptr_t)p, sz);
	if (!p) {
		abort();
	}
	return p;
}

void xfree(void *p)
{
	cthread_altstack((cthread_altstack_fn) & (free), (uintptr_t)p, 0);
}

//////////////////////////
// cthread functions

void *cthread_call(cthread_t *to, void *data)
{
	cthread_t *from = cthread_self();
	void **psave = from ? &from->stack : &to->main->stack;
	to->yield_to = from;
	set_thread(to);
	return cthread_asm_switch(data, to->stack, psave, &to->caller);
}

void *cthread_tailcall(cthread_t *to, void *data)
{
	cthread_t *from = cthread_self();
	to->yield_to = from->yield_to;
	memcpy(&to->caller, &from->caller, sizeof(to->caller));
	set_thread(to);
	struct cthread_caller dummy;
	return cthread_asm_switch(data, to->stack, &from->stack, &dummy);
}

static void *do_yield(void *data, cthread_t *from)
{
	cthread_t *to = from->yield_to;
	void *load = to ? to->stack : from->main->stack;
	set_thread(to);
	struct cthread_caller dummy;
	return cthread_asm_switch(data, load, &from->stack, &dummy);
}

void *cthread_yield(void *data)
{
	return do_yield(data, cthread_self());
}

int cthread_join(cthread_t *t)
{
	cthread_t *c = cthread_self();
	assert(c);
	if (!t->finished) {
		t->exit_to = c;
		do_yield(NULL, c);
		assert(t->finished);
		t->exit_to = NULL;
	}
	return (int)t->exit_code;
}

void cthread_exit(uintptr_t code)
{
	cthread_t *c = cthread_self();
	assert(c && !c->finished);
	c->main->num_threads--;
	c->finished = true;
	c->exit_code = code;
	if (c->exit_to) {
		cthread_tailcall(c->exit_to, c->exit_data);
	} else {
		do_yield(NULL, c);
	}
}

static cthread_t *create_cthread(struct cthread_main *m, cthread_fn fn,
				 char *stack, size_t sz)
{
	// 16B align the structure so that the thread stack is 16B aligned
	cthread_t *c = (cthread_t *)((uintptr_t)(stack + sz - sizeof(*c)) &
				     ~(uintptr_t)15U);
	memset(c, 0, sizeof(*c));
	void **p = (void **)c;
	p[-8] = NULL; // initial R15
	p[-7] = NULL; // initial R14
	p[-6] = NULL; // initial R13
	p[-5] = NULL; // initial R12
	p[-4] = NULL; // initial RBX
	p[-3] = &c->caller.frame; // initial RBP
	p[-2] = (void *)fn;
	p[-1] = ((char *)&__morestack) + 1 /* skip over nop */;

	// in cthread_asm_switch after restoring variables but before leave
	// rsp = p[-3]
	// rbp = p[-3]
	// p[-2] = function
	// p[-1] = &cthread_main

	// in the main function
	// rsp = p[-n]
	// ... - main function stack contents
	// rbp = p[-2]
	// p[-2] = &c->caller.frame - frame of cthread_main - set by main
	// function p[-1] = &cthread_main

	// in cthread_main
	// rsp = p
	// rbp = &c->caller.frame
	c->stack = p - 8; // initial rsp inside cthread_asm_switch
	c->main = m;
	m->num_threads++;
	return c;
}

cthread_t *cthread_start(cthread_fn fn, void *arg, char *stack, size_t sz)
{
	cthread_t *from = cthread_self();
	cthread_t *to = create_cthread(from->main, fn, stack, sz);
	to->yield_to = from;
	set_thread(to);
	return cthread_asm_switch(arg, to->stack, &from->stack, &to->caller);
}

//////////////////////////
// main functions

cthread_t *cthread_main_start(struct cthread_main *m, cthread_fn fn, void *arg,
			      char *stack, size_t sz)
{
	assert(!cthread_self());
	cthread_t *to = create_cthread(m, fn, stack, sz);
	assert(!to->yield_to);
	to->yield_to = NULL;
	set_thread(to);
	return cthread_asm_switch(arg, to->stack, &m->stack, &to->caller);
}

// void __morestack(void) __attribute__((weak));

// void __morestack(void)
// {
// }