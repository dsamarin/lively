/**
 * @file lively_app.c
 * @brief Lively Application: Manages the application lifecycle
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "lively_app.h"
#include "lively_audio.h"

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

	if (lively_audio_start (&app->audio)) {
		app->running = true;
		platform_sleep (3);
		lively_audio_stop (&app->audio);
	}
}
void lively_app_stop (lively_app_t *app) {
	if (!app->running) {
		return;
	}

	lively_app_log (app, LIVELY_INFO, "main", "Stopping lively");

	app->running = false;
	lively_audio_stop (&app->audio);
}

void lively_app_log (lively_app_t *app, enum lively_log_level level, const char *group, const char *fmt, ...) {
	FILE *out = stdout;
	va_list args;
	int color = 0;
	const char *level_text;

	switch (level) {
	case LIVELY_TRACE:
		level_text = "trace";
		color = 36; // cyan
		break;
	case LIVELY_DEBUG:
		level_text = "debug";
		color = 36; // cyan
		break;
	case LIVELY_INFO:
		level_text = "info";
		color = 34; // blue
		break;
	case LIVELY_WARN:
		level_text = "warning";
		color = 33; // yellow
		break;
	case LIVELY_ERROR:
		level_text = "error";
		color = 31; // red;
		out = stderr;
		break;
	case LIVELY_FATAL:
		level_text = "fatal";
		color = 31; // red
		out = stderr;
		break;
	}

	fprintf (out, "lively:%s \x1b[%d;1m%s\x1b[0m ", group, color, level_text);

	va_start (args, fmt);
	vfprintf (out, fmt, args);
	va_end (args);

	putchar ('\n');

	// If we had a fatal error, we need to shutdown immediately.
	// Let's try to do it cleanly.
	static bool fatal = false;
	if (level == LIVELY_FATAL) {
		if (fatal) {
			// We have already had a fatal error.
			// Forceful shutdown.
			exit (1);
		}
		fatal = true;
		// Attempt to destroy what has been initialized.
		lively_app_destroy (app);
	}
}
