#pragma once
#include <stdlib.h>

extern void *xmalloc(size_t sz);
extern void *xcalloc(size_t num, size_t sz);
extern void *xrealloc(void *p, size_t sz);
extern void xfree(void *p);

#define kmalloc xmalloc
#define kcalloc xcalloc
#define krealloc xrealloc
#define kfree xfree

#define malloc(SZ) xmalloc(SZ)
#define calloc(N,Z) xcalloc(N,Z)
#define realloc(P,Z) xrealloc(P,Z)
#define free(P) xfree(P)