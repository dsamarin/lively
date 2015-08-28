#ifndef LIVELY_THREAD_H
#define LIVELY_THREAD_H

#include <stdbool.h>

#include <pthread.h>

struct lively_app;

typedef enum lively_thread_state {
	THREAD_START,
	THREAD_STOP
} lively_thread_state_t;

typedef struct lively_thread {
	struct lively_app *app;
	pthread_t pthread;
	void (*main)(struct lively_thread *);

	int state; // #atomic
} lively_thread_t;

bool lively_thread_init (
	lively_thread_t *thread,
	struct lively_app *app,
	void (*main)(lively_thread_t *));

lively_thread_state_t lively_thread_get_state (lively_thread_t *);
void lively_thread_set_state (lively_thread_t *, lively_thread_state_t);

void lively_thread_join (lively_thread_t *);

void lively_thread_join_multiple (lively_thread_t *start, ...);
void lively_thread_set_state_multiple (
	lively_thread_state_t state,
	lively_thread_t *start, ...);

#endif
