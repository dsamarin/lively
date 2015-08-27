#ifndef LIVELY_APP_H
#define LIVELY_APP_H

#include <stdarg.h>
#include <stdbool.h>

#include "lively_thread.h"

/**
 * Specifies the level used for logging messages within lively.
 */
enum lively_log_level {
	LIVELY_TRACE,
	/**< Information that is diagnostically helpful while developing Lively */
	LIVELY_DEBUG,
	/**< Information that is diagnostically helpful to administrators developing a Lively system */
	LIVELY_INFO,
	/**< Information that is generally helpful regarding state changes or assumptions */
	LIVELY_WARN,
	/**< Information regarding recoverable application oddities */
	LIVELY_ERROR,
	/**< Information regarding unrecoverable errors */
	LIVELY_FATAL
	/**< Information regarding errors that force an immediate shutdown */
};

typedef struct lively_app {
	bool running;
	lively_thread_t thread_audio;
	lively_thread_t thread_disk;
	lively_thread_t thread_server;
} lively_app_t;

void lively_app_init (lively_app_t *);
void lively_app_destroy (lively_app_t *);
void lively_app_run (lively_app_t *);
void lively_app_stop (lively_app_t *);

void lively_app_log (
	lively_app_t *,
	enum lively_log_level,
	const char *group,
	const char *fmt, ...);

void lively_app_log_va (
	lively_app_t *,
	enum lively_log_level,
	const char *group,
	const char *fmt,
	va_list);


#endif
