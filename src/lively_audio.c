#include "lively_app.h"
#include "lively_audio.h"
#include "lively_audio_backend.h"
#include "lively_audio_config.h"

#include "platform.h"

static const char *module = "audio";

static void
audio_logger (void *user, enum lively_log_level level, const char *fmt, ...);

/**
* The main function for the Lively audio component.
*
* This function begins by initializing the configuration structure and creating
* a new audio backend.
*
* @param thread The Lively Thread
*/
void lively_audio_main (lively_thread_t *thread) {
	lively_audio_config_t config;
	lively_audio_backend_t *backend;
	lively_audio_block_t block;

	if (lively_thread_get_state (thread) == THREAD_STOP)
		return;

	lively_audio_config_init (&config);

	backend = lively_audio_backend_new (&config);
	if (!backend) {
		lively_app_log (thread->app, LIVELY_FATAL, module,
			"Could not allocate audio backend");
		return;
	}

	lively_app_log (thread->app, LIVELY_INFO, module,
		"Starting audio (%s)", lively_audio_backend_name (backend));


	lively_audio_backend_set_logger (backend, audio_logger, thread->app);
	
	if (lively_audio_backend_connect (backend)) {
		float latency = lively_audio_config_get_latency (&config);
		lively_app_log (thread->app, LIVELY_INFO, module,
			"Audio is configured for latency of %.2fms", latency * 1000.0);

		if (!lively_audio_block_init (&block, &config)) {
			lively_app_log (thread->app, LIVELY_ERROR, module,
				"Could not allocate audio channel block structure");
		} else {

			lively_audio_block_silence_output (&block);
			if (lively_audio_backend_start (backend, &block)) {
				while (lively_audio_backend_wait (backend)) {
					if (!lively_audio_backend_read (backend, &block)) {
						lively_app_log (thread->app, LIVELY_ERROR, module,
							"Read failed");
						break;
					}
					if (!lively_audio_backend_write (backend, &block)) {
						lively_app_log (thread->app, LIVELY_ERROR, module,
							"Write failed");
						break;
					}

					if (lively_thread_get_state (thread) == THREAD_STOP)
						break;
				}
				lively_audio_backend_stop (backend);
			}

		}

		lively_audio_block_destroy (&block);
		lively_audio_backend_disconnect (backend);
	}

	lively_audio_backend_delete (&backend);

	lively_app_log (thread->app, LIVELY_INFO, module, "Stopping audio");
}

static void
audio_logger (void *user, enum lively_log_level level, const char *fmt, ...) {
	va_list args;
	lively_app_t *app = (lively_app_t *) user;

	va_start (args, fmt);
	lively_app_log_va (app, level, module, fmt, args);
	va_end (args);
}
