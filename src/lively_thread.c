#include <stdarg.h>

#include <pthread.h>

#include <stdio.h>
#include "lively_app.h"
#include "lively_thread.h"

static void *thread_start_routine (void *arg) {
	lively_thread_t *thread = arg;
	thread->main (thread);
	return NULL;
}

/**
* Initialize and begin a new Lively thread
*
* @param thread The Lively Thread
* @param app The Lively Application
* @param main The main function of the thread
*
* @return A success value
*/
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

/**
* Reads an atomic state variable used to control a Lively thread.
*
* @param thread The Lively Thread
*
* @return The value of the state variable
*/
lively_thread_state_t
lively_thread_get_state (lively_thread_t *thread) {
	// #atomic load
	return thread->state;
}

/**
* Sets an atomic state variable used to control a Lively thread.
*
* @param thread The Lively Thread
* @param state The value of the state variable
*/
void
lively_thread_set_state (lively_thread_t *thread, lively_thread_state_t state) {
	// #atomic store
	thread->state = state;
}

bool
lively_thread_acquire_realtime (lively_thread_t *thread) {
	int err;
	struct sched_param param;

	param.sched_priority = 10;

	err = pthread_setschedparam (thread->pthread, SCHED_FIFO, &param);
	if (err) {
		return false;
	}

	return true;
}

/**
* Joins a Lively Thread.
*
* This function blocks until the thread terminates.
*
* @param thread The Lively thread
*/
void lively_thread_join (lively_thread_t *thread) {
	pthread_join (thread->pthread, NULL);
}


/**
* Joins multiple Lively Threads.
*
* This function is called with a series of pointers to Lively threads, with
* the end of the series represented by a NULL pointer.
*
* @see lively_thread_join()
*
* @param start The Lively Thread
* @param ...
*/
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

/**
* Sets the state variable on multiple Lively Threads
*
* This function is called with a series of pointers to Lively threads, with
* the end of the series represented by a NULL pointer.
*
* @see lively_thread_set_state()
*
* @param state The state value to set
* @param start The Lively Threads
*
* @param ...
*/
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
