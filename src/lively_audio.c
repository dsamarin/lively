/**
 * @file lively_audio.c
 * Lively Audio: Manages audio interface with driver
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <jack/jack.h>

#include "lively_app.h"
#include "lively_audio.h"
#include "lively_node.h"

/* Demonstration purposes */
static jack_port_t *jack_input_left;
static jack_port_t *jack_input_right;
static jack_port_t *jack_output_left;
static jack_port_t *jack_output_right;
static lively_node_io_t stereo_left_in;
static lively_node_io_t stereo_right_in;
static lively_node_io_t stereo_left_out;
static lively_node_io_t stereo_right_out;

static int audio_process (jack_nframes_t nframes, void *arg) {
	lively_audio_t *audio = (lively_audio_t *) arg;
	lively_app_t *app = audio->app;
	lively_scene_t *scene = &app->scene;

	jack_default_audio_sample_t *il, *ir, *ol, *or;
	unsigned int length = nframes;

	il = jack_port_get_buffer (jack_input_left, nframes);
	ir = jack_port_get_buffer (jack_input_right, nframes);
	ol = jack_port_get_buffer (jack_output_left, nframes);
	or = jack_port_get_buffer (jack_output_right, nframes);

	float *lil = lively_node_io_get_buffer ((lively_node_t *) &stereo_left_in, LIVELY_MONO);
	float *lir = lively_node_io_get_buffer ((lively_node_t *) &stereo_right_in, LIVELY_MONO);

	memcpy (lil, il, length * sizeof (float));
	memcpy (lir, ir, length * sizeof (float));

	lively_scene_process (scene, length);

	float *lol = lively_node_io_get_buffer ((lively_node_t *) &stereo_left_out, LIVELY_MONO);
	float *lor = lively_node_io_get_buffer ((lively_node_t *) &stereo_right_out, LIVELY_MONO);

	memcpy (ol, lol, length * sizeof (float));
	memcpy (or, lor, length * sizeof (float));

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

	// TODO: This may change while running, so use callback.
	jack_nframes_t buffer_size = jack_get_buffer_size (client);
	lively_app_log (app, LIVELY_DEBUG, "audio",
		"Jack buffer size is set to %" PRIu32 " frames", buffer_size);
	audio->buffer_length = buffer_size;
	lively_scene_set_buffer_length (&app->scene, buffer_size);

	/*
	 * 2. Connect Jack audio API to Lively
	 */
	jack_set_process_callback (client, audio_process, audio);
	jack_on_shutdown (client, audio_shutdown, audio);

	/*
	 * 3. Create Jack ports for Lively (demo)
	 */
	jack_input_left = jack_port_register (client,
		"input_left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
	jack_input_right = jack_port_register (client,
		"input_right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
	jack_output_left = jack_port_register (client,
		"output_left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	jack_output_right = jack_port_register (client,
		"output_right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	if (!jack_input_left || !jack_input_right
		|| !jack_output_left || !jack_output_right) {
		lively_app_log (app, LIVELY_FATAL, "audio", "Too many Jack ports");
		return false;
	}

	// Initialize fields
	audio->app = app;
	audio->client = client;
	audio->started = false;

	/* Create input and output nodes */
	lively_node_io_init (&stereo_left_in, LIVELY_NODE_INPUT);
	lively_node_io_init (&stereo_right_in, LIVELY_NODE_INPUT);
	lively_node_io_init (&stereo_left_out, LIVELY_NODE_OUTPUT);
	lively_node_io_init (&stereo_right_out, LIVELY_NODE_OUTPUT);

	/* Add nodes to scene */
	lively_scene_add_node (&app->scene, (lively_node_t *) &stereo_left_in);
	lively_scene_add_node (&app->scene, (lively_node_t *) &stereo_right_in);
	lively_scene_add_node (&app->scene, (lively_node_t *) &stereo_left_out);
	lively_scene_add_node (&app->scene, (lively_node_t *) &stereo_right_out);

	/* Connect nodes together */
	lively_scene_connect (&app->scene,
		(lively_node_t *) &stereo_left_in, LIVELY_MONO,
		(lively_node_t *) &stereo_left_out, LIVELY_MONO);
	lively_scene_connect (&app->scene,
		(lively_node_t *) &stereo_right_in, LIVELY_MONO,
		(lively_node_t *) &stereo_right_out, LIVELY_MONO);

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

	const char **ports;

	// Connect input ports left and right
	ports = jack_get_ports (audio->client, NULL, NULL,
		JackPortIsPhysical | JackPortIsOutput);
	if (!ports) {
		lively_app_log (audio->app, LIVELY_FATAL, "audio",
			"No Jack system capture ports");
	}
	if (jack_connect (audio->client,
		ports[0], jack_port_name (jack_input_left))) {
		lively_app_log (audio->app, LIVELY_ERROR, "audio",
			"Could not connect input left");
		return false;
	}
	if (jack_connect (audio->client,
		ports[1], jack_port_name (jack_input_right))) {
		lively_app_log (audio->app, LIVELY_ERROR, "audio",
			"Could not connect input right");
		return false;
	}
	jack_free (ports);
	// Connect output ports left and right
	ports = jack_get_ports (audio->client, NULL, NULL,
		JackPortIsPhysical | JackPortIsInput);
	if (!ports) {
		lively_app_log (audio->app, LIVELY_FATAL, "audio",
			"No Jack system playback ports");
	}
	if (jack_connect (audio->client,
		jack_port_name (jack_output_left), ports[0])) {
		lively_app_log (audio->app, LIVELY_ERROR, "audio",
			"Could not connect input left");
		return false;
	}
	if (jack_connect (audio->client,
		jack_port_name (jack_output_right), ports[1])) {
		lively_app_log (audio->app, LIVELY_ERROR, "audio",
			"Could not connect input right");
		return false;
	}
	jack_free (ports);
	
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
