#include <stdarg.h>

#define _GNU_SOURCE
#include <pthread.h>

#include <stdio.h>
#include "lively_app.h"
#include "lively_thread.h"

static void *thread_start_routine (void *arg) {
	lively_thread_t *thread = arg;
	thread->main (thread);
	return NULL;
}

bool lively_thread_init (
	lively_thread_t *thread,
	lively_app_t *app,
	void (*main)(lively_thread_t *)) {

	int err;
	thread->app = app;
	thread->main = main;
	lively_thread_set_state (thread, THREAD_START);

	err = pthread_create(
		&thread->pthread,
		NULL,
		thread_start_routine,
		thread);

	return err == 0;
}

lively_thread_state_t
lively_thread_get_state (lively_thread_t *thread) {
	// #atomic load
	return thread->state;
}

void
lively_thread_set_state (lively_thread_t *thread, lively_thread_state_t state) {
	// #atomic store
	thread->state = state;
}

void lively_thread_name (lively_thread_t *thread, const char *name) {
	pthread_setname_np (thread->pthread, name);
}

void lively_thread_join (lively_thread_t *thread) {
	pthread_join (thread->pthread, NULL);
}


void
lively_thread_join_multiple (lively_thread_t *start, ...) {
	va_list threads;
	lively_thread_t *thread = start;

	va_start (threads, start);

	while (thread) {
		lively_thread_join (thread);
		thread = va_arg (threads, lively_thread_t *);
	}

	va_end (threads);
}

void
lively_thread_set_state_multiple (
	lively_thread_state_t state,
	lively_thread_t *start,
	...) {

	va_list threads;
	lively_thread_t *thread = start;

	va_start (threads, start);

	while (thread) {
		lively_thread_set_state (thread, state);
		thread = va_arg (threads, lively_thread_t *);
	}

	va_end (threads);
}
