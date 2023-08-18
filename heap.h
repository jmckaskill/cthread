#pragma once
#include <stdint.h>
#include <stddef.h>

#ifndef container_of
#define container_of(ptr, type, member) \
	((type *)((char *)(ptr)-offsetof(type, member)))
#endif

struct heap_mod32_node {
	int index;
};

struct heap_mod32_entry {
	uint32_t value;
	struct heap_mod32_node *node;
};

struct heap_mod32 {
	struct heap_mod32_entry *v;
	int size;
	int cap;
};

#define HEAP_INVALID 0
#define HEAP_HEAD 1

void heap_insert(struct heap_mod32 *ph, struct heap_mod32_node *n,
		 uint32_t value);
void heap_remove(struct heap_mod32 *h, int i);
