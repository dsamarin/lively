/**
 * @file lively_audio.c
 * @brief Lively Audio: Manages audio interface with driver
 */

#include <stdlib.h>
#include <stdbool.h>

#include <jack/jack.h>

#include "lively_app.h"
#include "lively_audio.h"

static int audio_process (jack_nframes_t nframes, void *arg) {
	lively_audio_t *audio = (lively_audio_t *) arg;
	(void) audio;

	return 0;
}

static void audio_shutdown (void *arg) {
	lively_audio_t *audio = (lively_audio_t *) arg;
	lively_app_t *app = audio->app;

	lively_app_log (app, LIVELY_ERROR, "audio",
		"Jack audio server has shut down or disconnected us");

	lively_audio_destroy (audio);
	
	// TODO: Consider recovering by maybe restarting Jack and re-initializing
}

bool lively_audio_init (lively_audio_t *audio, lively_app_t *app) {
	const char *client_name = "lively";

	jack_client_t *client;
	jack_options_t options = JackNullOption;
	jack_status_t status;

	/*
	 * 1. Connect to the Jack audio server
	 */
	client = jack_client_open (client_name, options, &status);
	if (client == NULL) {
		// TODO: Provide more useful message with status flags
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Unable to connect to Jack audio server");
		return false;
	}

	if (status & JackServerStarted) {
		lively_app_log (app, LIVELY_INFO, "audio",
		"Jack audio server is started");
	}

	if (status & JackNameNotUnique) {
		char *client_name_new = jack_get_client_name (client);
		lively_app_log (app, LIVELY_DEBUG, "audio",
			"Jack client name \"%s\" is not unique; using \"%s\"",
			client_name, client_name_new);
		client_name = client_name_new;
	}

	jack_nframes_t sample_rate = jack_get_sample_rate (client);
	lively_app_log (app, LIVELY_DEBUG, "audio",
		"Jack sample rate is set to %" PRIu32 " Hz", sample_rate);
	audio->sample_rate = (unsigned int) sample_rate;

	/*
	 * 2. Connect Jack audio API to Lively
	 */
	jack_set_process_callback (client, audio_process, audio);
	jack_on_shutdown (client, audio_shutdown, audio);

	// Initialize fields
	audio->app = app;
	audio->client = client;
	audio->started = false;

	return true;
}

bool lively_audio_start (lively_audio_t *audio) {

	if (audio->started) {
		return false;
	}
	
	if (jack_activate (audio->client)) {
		lively_app_log (audio->app, LIVELY_ERROR, "audio",
			"Jack client could not be activated");
		return false;
	}

	audio->started = true;

	// TODO: Connect ports here

	return true;
}

bool lively_audio_stop (lively_audio_t *audio) {

	if (!audio->started) {
		return false;
	}

	if (jack_deactivate (audio->client)) {
		lively_app_log (audio->app, LIVELY_ERROR, "audio",
			"Jack client could not be deactivated");
		return false;
	}

	audio->started = false;

	return true;
}

void lively_audio_destroy (lively_audio_t *audio) {
	// Shutdown our app if running
	if (audio->app) {
		lively_app_stop (audio->app);
		audio->app = NULL;
	}

	// Disconnect from Jack
	if (audio->client) {
		jack_client_close (audio->client);
		audio->client = NULL;
	}
}
