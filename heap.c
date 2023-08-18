#include "heap.h"
#include "mem.h"
#include <stdbool.h>
#include <assert.h>

//    1
//   2 3
// 4 5 6 7

static inline int heap_parent(int idx)
{
	return idx / 2;
}
static inline int heap_left(int idx)
{
	return 2 * idx;
}

static inline bool heap_less(struct heap_mod32 *h, int a, int b)
{
	return (int32_t)(h->v[a].value - h->v[b].value) < 0;
}

static inline void heap_swap(struct heap_mod32 *h, int a, int b)
{
	assert(0 < a && a <= h->size && 0 < b && b <= h->size);
	struct heap_mod32_entry ae = h->v[a];
	struct heap_mod32_entry be = h->v[b];
	ae.node->index = b;
	be.node->index = a;
	h->v[a] = be;
	h->v[b] = ae;
}

static void heap_bubble_up(struct heap_mod32 *h, int idx, uint32_t value)
{
	h->v[idx].value = value;
	for (;;) {
		int parent = heap_parent(idx);
		if (!parent || !heap_less(h, idx, parent)) {
			break;
		}
		heap_swap(h, idx, parent);
		idx = parent;
	}
}

static void heap_bubble_down(struct heap_mod32 *h)
{
	int parent = HEAP_HEAD;
	for (;;) {
		int left = heap_left(parent);
		if (left > h->size) {
			break;
		}
		int right = left + 1;
		int smaller = left;
		if (right <= h->size && heap_less(h, right, left)) {
			smaller = right;
		}
		if (!heap_less(h, smaller, parent)) {
			break;
		}
		heap_swap(h, smaller, parent);
		parent = smaller;
	}
}

void heap_insert(struct heap_mod32 *h, struct heap_mod32_node *n,
		 uint32_t value)
{
	if (HEAP_HEAD + h->size >= h->cap) {
		h->cap = (h->cap * 2) + 16;
		h->v = xrealloc(h->v, h->cap * sizeof(h->v[0]));
	}
	int idx = HEAP_HEAD + h->size++;
	n->index = idx;
	h->v[idx].node = n;
	heap_bubble_up(h, idx, value);
}

void heap_remove(struct heap_mod32 *h, int idx)
{
	if (idx == HEAP_HEAD + h->size - 1) {
		// to remove item is the last item, just remove
		h->size--;
		return;
	}
	// first move the item to the top
	if (idx != HEAP_HEAD) {
		heap_bubble_up(h, idx, h->v[HEAP_HEAD].value - 1);
	}
	// then move last item to head and bubble that down
	struct heap_mod32_entry *last = &h->v[HEAP_HEAD + --h->size];
	h->v[HEAP_HEAD] = *last;
	last->node->index = HEAP_HEAD;
	heap_bubble_down(h);
}
