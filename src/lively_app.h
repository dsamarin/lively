#ifndef LIVELY_APP_H
#define LIVELY_APP_H

#include <stdbool.h>

typedef struct lively_app lively_app_t;

#include "lively_audio.h"

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
	lively_audio_t audio;
	bool running;
} lively_app_t;

bool lively_app_init (lively_app_t *);
void lively_app_destroy (lively_app_t *);
void lively_app_run (lively_app_t *);
void lively_app_stop (lively_app_t *);

void lively_app_log (lively_app_t *, enum lively_log_level, const char *group, const char *fmt, ...);

#endif
