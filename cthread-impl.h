#pragma once
#include "cthread.h"

struct cthread_frame {
	struct cthread_frame *prev; // previous frame pointer
	void (*return_address)(void); // return address
};

struct cthread_caller {
	void *stack;
	struct cthread_frame frame;
};

struct cthread {
	void *stack;
	struct cthread_caller caller;
	struct cthread_main *main;
	cthread_t *wait_next;
	cthread_t *yield_to;
	cthread_t *exit_to;
	void *exit_data;
	uintptr_t exit_code;
	bool finished;
};