#pragma once
#include "evloop.h"
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct cthread cthread_t;

struct cthread_main {
	void *stack;
	int num_threads;
};

typedef int (*cthread_fn)(void *);

void cthread_main_init(struct cthread_main *m);

cthread_t *cthread_main_start(struct cthread_main *m, cthread_fn fn, void *arg,
			      char *stack, size_t sz);

cthread_t *cthread_start(cthread_fn fn, void *arg, char *stack, size_t sz);
int cthread_join(cthread_t *t);
void *cthread_yield(void *data);
void *cthread_call(cthread_t *tgt, void *data);
void *cthread_tailcall(cthread_t *tgt, void *data);
void cthread_exit(uintptr_t code);
cthread_t *cthread_self(void);

typedef uintptr_t (*cthread_altstack_fn)(uintptr_t, uintptr_t);
uintptr_t cthread_altstack(cthread_altstack_fn fn, uintptr_t arg0,
			   uintptr_t arg1);
