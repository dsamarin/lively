#include "lively_app.h"
#include "lively_audio.h"
#include "lively_audio_backend.h"

#include "platform.h"

static const char *module = "audio";

static void
audio_logger (void *user, enum lively_log_level level, const char *fmt, ...);

void lively_audio_main (lively_thread_t *thread) {
	lively_audio_backend_t *backend;

	lively_thread_name (thread, module);

	backend = lively_audio_backend_new ();
	if (!backend) {
		lively_app_log (thread->app, LIVELY_FATAL, module,
			"Could not allocate audio backend");
		return;
	}


	lively_audio_backend_set_logger (backend, audio_logger, thread->app);

	if (lively_thread_get_state (thread) != THREAD_START)
		return;
	
	// TODO: Run until THREAD_STOP
	lively_app_log (thread->app, LIVELY_INFO, module, "Starting audio");
	if (!lively_audio_backend_connect (backend)) {
	}

	platform_sleep (3);

	lively_app_log (thread->app, LIVELY_INFO, module, "Stopping audio");
	lively_audio_backend_disconnect (backend);
}

static void
audio_logger (void *user, enum lively_log_level level, const char *fmt, ...) {
	va_list args;
	lively_app_t *app = (lively_app_t *) user;

	va_start (args, fmt);
	lively_app_log_va (app, level, module, fmt, args);
	va_end (args);
}
