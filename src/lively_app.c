/**
 * @file lively_app.c
 * Lively Application: Manages the application lifecycle
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "lively_app.h"
#include "lively_audio.h"
#include "lively_scene.h"

#include "platform.h"

bool lively_app_init (lively_app_t *app) {
	lively_app_log (app, LIVELY_INFO, "main", "Starting lively");

	// Initialize members
	app->running = false;

	// Start submodules
	if (!lively_audio_init (&app->audio, app)) {
		return false;
	}

	return true;
}
void lively_app_destroy (lively_app_t *app) {
	if (app->running) {
		lively_app_stop (app);
	}

	lively_audio_destroy (&app->audio);
}
void lively_app_run (lively_app_t *app) {
	if (app->running) {
		return;
	}

	lively_app_log (app, LIVELY_INFO, "main", "Running lively");

	lively_scene_init (&app->scene, app);
	
	if (!lively_audio_start (&app->audio)) {
		return;
	}

	app->running = true;
	platform_sleep (6);

	lively_audio_stop (&app->audio);
	lively_scene_destroy (&app->scene);
}
void lively_app_stop (lively_app_t *app) {
	if (!app->running) {
		return;
	}

	lively_app_log (app, LIVELY_INFO, "main", "Stopping lively");

	app->running = false;
	lively_audio_stop (&app->audio);
}

/**
* Logs an event that pertains to Lively. 
*
* This function will print the message to the standard output
* file stream for non-error messages and to the standard error file
* stream for error messages. Error messages are defined by levels of
* #LIVELY_ERROR and #LIVELY_FATAL.
*
* #LIVELY_FATAL messages will also trigger a clean shutdown of the
* Lively application. If during a clean shutdown, and this level is used,
* the application will force-quit to prevent any recursion loops.
*
* The message format is as follows: 
* <code>
* lively:<group> <level> <message>
* </code>
*
* @param app A reference to the currently running application
* @param level The type of message this is
* @param group The module within Lively that generated the message
* @param fmt A printf-style format string
* @param ...
*/
void lively_app_log (
	lively_app_t *app,
	enum lively_log_level level,
	const char *group,
	const char *fmt, ...) {

	va_list args;

	static const struct {
		FILE** file;
		const char *name;
		int color;
	} options[] = {
		[LIVELY_TRACE] = {&stdout, "trace", 36 /* cyan */},
		[LIVELY_DEBUG] = {&stdout, "debug", 36 /* cyan */},
		[LIVELY_INFO]  = {&stdout, "info",  34 /* blue */},
		[LIVELY_WARN]  = {&stdout, "warning", 33 /* yellow */},
		[LIVELY_ERROR] = {&stderr, "error", 31 /* red */},
		[LIVELY_FATAL] = {&stderr, "fatal", 31 /* red */},
	};

	fprintf (*options[level].file,
		"lively:%s \x1b[%d;1m%s\x1b[0m ",
		group,
		options[level].color,
		options[level].name);

	va_start (args, fmt);
	vfprintf (*options[level].file, fmt, args);
	va_end (args);

	putchar ('\n');

	// If we have a fatal error, we should shutdown cleanly.
	// Except when we are called twice, we shutdown immediately.
	if (level == LIVELY_FATAL) {
		static bool seen = false;
		if (seen) {
			// We have already had a fatal error.
			// Forceful shutdown.
			fprintf (stderr, "lively: forced shutdown\n");
			exit (1);
		} else {
			seen = true;
			// Attempt to destroy what has been initialized.
			lively_app_destroy (app);
		}
	}
}
