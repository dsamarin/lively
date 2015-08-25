/**
 * @file lively_audio.c
 * Lively Audio: Manages audio interface with driver
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "lively_app.h"
#include "lively_audio.h"
#include "lively_node.h"

/* Demonstration purposes */
static lively_node_io_t stereo_left_in;
static lively_node_io_t stereo_right_in;
static lively_node_io_t stereo_left_out;
static lively_node_io_t stereo_right_out;

#if 0
static int audio_process (unsigned int frames, void *arg) {
	lively_audio_t *audio = (lively_audio_t *) arg;
	lively_app_t *app = audio->app;
	lively_scene_t *scene = &app->scene;

	lively_node_io_get_buffer ((lively_node_t *) &stereo_left_in, LIVELY_MONO);
	lively_node_io_get_buffer ((lively_node_t *) &stereo_right_in, LIVELY_MONO);

	lively_scene_process (scene, frames);

	lively_node_io_get_buffer ((lively_node_t *) &stereo_left_out, LIVELY_MONO);
	lively_node_io_get_buffer ((lively_node_t *) &stereo_right_out, LIVELY_MONO);

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
#endif

static void audio_handle_pcm_open_error(
	lively_app_t *app,
	const char *device,
	const char *stream,
	int err) {

	switch (err) {
	case EBUSY:
		lively_app_log (app, LIVELY_ERROR, "audio",
			"The %s device \"%s\" is already in use", stream, device);
		break;
	case EPERM:
		lively_app_log (app, LIVELY_ERROR, "audio",
			"You do not have permission to use %s device \"%s\"",
			stream, device);
		break;
	default:
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Unknown error attempting to open %s device \"%s\"",
			stream, device);
	}
}

static bool audio_configure_stream (
	lively_audio_t *audio,
	const char *stream,
	snd_pcm_t *handle,
	snd_pcm_hw_params_t *hw_params,
	snd_pcm_sw_params_t *sw_params,
	unsigned int *channels) {

	int err;
	unsigned int channels_max;
	unsigned int frames_per_second = audio->frames_per_second;
	lively_app_t *app = audio->app;

	// This function narrows down the possible configurations for this
	// stream by allowing all configurations and narrowing down based off
	// of a sequence of constraints given to it.

	err = snd_pcm_hw_params_any (handle, hw_params);
	if (err < 0) {
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Could not initialize hardware params structure (%s)",
			snd_strerror (err));
		return false;
	}

	err = snd_pcm_hw_params_set_periods_integer (handle, hw_params);
	if (err < 0) {
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Could not restrict period size to integer value");
		return false;
	}

	// This attempts to set the access type for this stream to one of the
	// following configurations, in order.

	bool found = false;
	static snd_pcm_access_t supported_access_types[] = {
		SND_PCM_ACCESS_MMAP_NONINTERLEAVED,
		SND_PCM_ACCESS_MMAP_INTERLEAVED,
		SND_PCM_ACCESS_MMAP_COMPLEX
	};
	size_t length = sizeof supported_access_types / sizeof *supported_access_types;

	for (size_t i = 0; i < length; i++) {
		err = snd_pcm_hw_params_set_access (handle, hw_params,
			supported_access_types[i]);
		lively_app_log (app, LIVELY_TRACE, "audio", "Tried with (%p, %p, %d)",
			handle, hw_params, supported_access_types[i]);
		if (err == 0) {
			found = true;
			break;
		}
	}
	if (!found) {
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Could not get mmap-based access to %s stream", stream);
		return false;
	}

	err = snd_pcm_hw_params_set_format (handle, hw_params,
		SND_PCM_FORMAT_FLOAT_LE);
	if (err < 0) {
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Could not get 32-bit floating point audio support");
		return false;
	}

	err = snd_pcm_hw_params_set_rate_near (handle, hw_params,
		&frames_per_second, NULL);
	if (err < 0) {
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Could not set sample rate to %u Hz for %s stream",
			audio->frames_per_second, stream);
		return false;
	}

	err = snd_pcm_hw_params_get_channels_max (hw_params, &channels_max);
	if (err < 0) {
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Could not get maximum channel count available for %s stream",
			stream);
		return false;
	}

	err = snd_pcm_hw_params_set_channels (handle, hw_params, channels_max);
	if (err < 0) {
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Could not set channel count for %s stream", stream);
		return false;
	}

	err = snd_pcm_hw_params_set_period_size (handle, hw_params,
		audio->frames_per_period, 0);
	if (err < 0) {
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Could not set period size to %u frames for %s stream",
			audio->frames_per_period, stream);
		return false;
	}

	if (frames_per_second != audio->frames_per_second) {
		lively_app_log (app, LIVELY_WARN, "audio",
			"Setting sample rate to closest match %u Hz instead of %u Hz",
			frames_per_second, audio->frames_per_second);
		audio->frames_per_second = frames_per_second;
	}

	*channels = channels_max;

	return true;
}

static bool audio_configure_duplex (lively_audio_t *audio) {
	unsigned int channels;
	lively_app_t *app = audio->app;

	// Params structures
	if (snd_pcm_hw_params_malloc (&audio->playback_hw_params) < 0) {
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Could not allocate playback hardware params structure");
		return false;
	}
	if (snd_pcm_sw_params_malloc (&audio->playback_sw_params) < 0) {
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Could not allocate playback software params structure");
		return false;
	}
	if (snd_pcm_hw_params_malloc (&audio->capture_hw_params) < 0) {
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Could not allocate capture hardware params structure");
		return false;
	}
	if (snd_pcm_sw_params_malloc (&audio->capture_sw_params) < 0) {
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Could not allocate capture software params structure");
		return false;
	}

	if (!audio_configure_stream (audio, "playback", 
		audio->handle_playback,
		audio->playback_hw_params,
		audio->playback_sw_params,
		&channels)) {

		return false;
	}
	audio->playback_channels = channels;

	if (!audio_configure_stream (audio, "capture",
		audio->handle_capture,
		audio->capture_hw_params,
		audio->capture_sw_params,
		&channels)) {

		return false;
	}
	audio->capture_channels = channels;

	return true;
}

bool lively_audio_init (lively_audio_t *audio, lively_app_t *app) {
	int err;
	const char *device = "default";

	// Initialize fields
	audio->app = app;
	audio->handle_capture = NULL;
	audio->handle_playback = NULL;
	audio->playback_hw_params = NULL;
	audio->playback_sw_params = NULL;
	audio->capture_hw_params = NULL;
	audio->capture_sw_params = NULL;

	audio->started = false;
	audio->playback_channels = 0;
	audio->capture_channels = 0;
	audio->frames_per_second = 0;
	audio->frames_per_period = 0;
	audio->periods_per_buffer = 0;

	/*
	 * 1. Open playback and capture handles
	 */

	snd_pcm_t *handle_capture = audio->handle_capture;
	snd_pcm_t *handle_playback = audio->handle_playback;

	// Playback device
	err = snd_pcm_open (&handle_playback, device,
		SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if (err < 0) {
		audio_handle_pcm_open_error (app, device, "playback", err);
		return false;
	}
	audio->handle_playback = handle_playback;

	// Capture device
	err = snd_pcm_open (&handle_capture, device,
		SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
	if (err < 0) {
		audio_handle_pcm_open_error (app, device, "capture", err);
		return false;
	}
	audio->handle_capture = handle_capture;

	err = snd_pcm_nonblock (audio->handle_playback, 0);
	if (err < 0) {
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Could not set playback device to blocking mode");
		return false;
	}

	err = snd_pcm_nonblock (audio->handle_capture, 0);
	if (err < 0) {
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Could not set capture device to blocking mode");
		return false;
	}

	if (!audio_configure_duplex (audio)) {
		return false;
	}

	if (snd_pcm_link (audio->handle_playback, audio->handle_capture)) {
		lively_app_log (app, LIVELY_ERROR, "audio",
			"Could not link capture and playback handles");
		return false;
	}

	lively_scene_set_buffer_length (&app->scene, audio->frames_per_period);

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
	
	audio->started = true;

	return true;
}

bool lively_audio_stop (lively_audio_t *audio) {

	if (!audio->started) {
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

	if (audio->capture_sw_params)
		snd_pcm_sw_params_free (audio->capture_sw_params);
	if (audio->capture_hw_params)
		snd_pcm_hw_params_free (audio->capture_hw_params);
	if (audio->playback_sw_params)
		snd_pcm_sw_params_free (audio->playback_sw_params);
	if (audio->playback_hw_params)
		snd_pcm_hw_params_free (audio->playback_hw_params);

	if (audio->handle_playback) snd_pcm_close (audio->handle_playback);
	if (audio->handle_capture) snd_pcm_close (audio->handle_capture);
}
