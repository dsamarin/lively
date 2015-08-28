/**
 * @file lively_app.c
 * Lively Application: Manages the application lifecycle
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "../config.h"

#include "lively_thread.h"
#include "lively_app.h"
#include "lively_audio.h"
#include "lively_scene.h"

#include "platform.h"

/**
* Initializes a Lively Application.
*
* @param app The Lively Application
*/
void lively_app_init (lively_app_t *app) {
	lively_app_log (app, LIVELY_INFO, "main",
		"%s <%s>", PACKAGE_STRING, PACKAGE_URL);

	app->running = false;
}

/**
* Destroys a Lively Application
*
* This function will stop the Lively Application if it is already running.
*
* @param app The Lively Application
*/
void lively_app_destroy (lively_app_t *app) {
	if (app->running) {
		lively_app_shutdown (app);
	}
}

/**
* The main routine for a Lively Application
*
* This function prints log messages signaling the status change of the Lively
* Application.
*
* The function is designed to initialize, in separate threads, each of the
* core components required to run the Lively system. The function will block
* until each component has been terminated. The function #lively_app_stop can
* be used asyncronously in a signal handler or separate thread to shut down the
* Lively system and unblock this function.
*
* @param app The Lively Application
*/
void lively_app_run (lively_app_t *app) {
	if (app->running) {
		return;
	}

	lively_app_log (app, LIVELY_INFO, "main", "Running lively");

	// Start submodules
	if (!lively_thread_init (
		&app->thread_audio, app,
		lively_audio_main)) {
		return;
	}

	app->running = true;
	lively_thread_join_multiple (&app->thread_audio, NULL);
	app->running = false;

	lively_app_log (app, LIVELY_INFO, "main", "Stopping lively");
}

/**
* Shuts down the Lively system
*
* This function signals to each of the core components that make up the Lively
* system to shutdown. Each thread will terminate, causing the function
* #lively_app_run to return.
*
* This function does nothing if the Lively application is not running.
*
* @param app The Lively Application
*/
void lively_app_shutdown (lively_app_t *app) {
	if (!app->running) {
		return;
	}

	lively_thread_set_state_multiple (THREAD_STOP, 
		&app->thread_audio,
		NULL);
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
* @todo Make this function thread-safe.
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

	va_start (args, fmt);
	lively_app_log_va (app, level, group, fmt, args);
	va_end (args);
}

/**
* Logs an event that pertains to Lively (variadic).
*
* @see lively_app_log()
*
* @param app
* @param level
* @param group
* @param fmt
* @param args
*/
void lively_app_log_va (
	lively_app_t *app,
	enum lively_log_level level,
	const char *group,
	const char *fmt,
	va_list args) {

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

	vfprintf (*options[level].file, fmt, args);
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
