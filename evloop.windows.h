#pragma once
#include "evloop.h"
#include "heap.h"
#include <stdint.h>

struct evloop {
	struct evwatch *ready_list;
	struct evloop_poll *dirty_list;
	struct heap_mod32 timers;
	uint32_t now;
};

struct evwatch_data {
	struct ewvatch *next;
	struct {
		uint32_t internal[2];
		union {
			uint32_t offset[2];
			void *ptr;
		} u;
		void *handle;
	} overlapped;
	int finished;
	void *handle;
};