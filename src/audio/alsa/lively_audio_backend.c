#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../../lively_audio_backend.h"

#include "lively_audio_backend_alsa.h"

#define log(backend, type, ...) do { \
	if (backend->logger) { \
		backend->logger (backend->logger_data, type, __VA_ARGS__); \
	} \
} while (0)
#define log_error(backend, ...) log(backend, LIVELY_ERROR, __VA_ARGS__)
#define log_info(backend, ...) log(backend, LIVELY_INFO, __VA_ARGS__)
#define log_warn(backend, ...) log(backend, LIVELY_WARN, __VA_ARGS__)

static bool audio_configure_duplex (lively_audio_backend_t *backend);
static bool audio_configure_stream (
	lively_audio_backend_t *backend,
	const char *stream,
	snd_pcm_t *handle,
	snd_pcm_hw_params_t *hw_params,
	snd_pcm_sw_params_t *sw_params,
	unsigned int *channels);
static void audio_handle_pcm_open_error(
	lively_audio_backend_t *backend,
	const char *device,
	const char *stream,
	int err);

lively_audio_backend_t *
lively_audio_backend_new (void) {
	lively_audio_backend_t *backend = malloc (sizeof *backend);
	if (!backend) {
		return NULL;
	}

	backend->handle_playback = NULL;
	backend->handle_capture = NULL;
	backend->playback_hw_params = NULL;
	backend->playback_sw_params = NULL;
	backend->capture_hw_params = NULL;
	backend->capture_sw_params = NULL;

	backend->connected = false;
	backend->playback_channels = 2;
	backend->capture_channels = 2;

	backend->frames_per_second = 0;
	backend->frames_per_period = 0;
	backend->periods_per_buffer = 0;
	
	backend->logger = NULL;
	backend->logger_data = NULL;
	
	return backend;
}

void
lively_audio_backend_delete (lively_audio_backend_t **backend_ptr) {
	if (!backend_ptr)
		return;
	if (!*backend_ptr)
		return;

	lively_audio_backend_disconnect (*backend_ptr);

	free (*backend_ptr);
	*backend_ptr = NULL;
}

void
lively_audio_backend_set_logger (
	lively_audio_backend_t *backend,
	lively_audio_backend_logger_callback_t callback,
	void *data) {

	backend->logger = callback;
	backend->logger_data = data;
}

bool lively_audio_backend_connect (lively_audio_backend_t *backend) {
	int err;
	const char *device = "hw:0";

	if (backend->connected) {
		lively_audio_backend_disconnect (backend);
	}

	/*
	 * 1. Open playback and capture handles
	 */

	snd_pcm_t *handle_capture = backend->handle_capture;
	snd_pcm_t *handle_playback = backend->handle_playback;

	// Playback device
	err = snd_pcm_open (&handle_playback, device,
		SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if (err < 0) {
		audio_handle_pcm_open_error (backend, device, "playback", err);
		return false;
	}
	backend->handle_playback = handle_playback;

	// Capture device
	err = snd_pcm_open (&handle_capture, device,
		SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
	if (err < 0) {
		audio_handle_pcm_open_error (backend, device, "capture", err);
		return false;
	}
	backend->handle_capture = handle_capture;

	err = snd_pcm_nonblock (backend->handle_playback, 0);
	if (err < 0) {
		log_error (backend, "Could not set playback device to blocking mode");
		return false;
	}

	err = snd_pcm_nonblock (backend->handle_capture, 0);
	if (err < 0) {
		log_error (backend, "Could not set capture device to blocking mode");
		return false;
	}

	if (!audio_configure_duplex (backend)) {
		return false;
	}

	if (snd_pcm_link (backend->handle_playback, backend->handle_capture)) {
		log_error (backend,
			"Could not link capture and playback handles");
		return false;
	}

	backend->connected = true;

	return true;
}

bool
lively_audio_backend_disconnect (lively_audio_backend_t *backend) {
	if (!backend->connected) {
		return false;
	}

	backend->connected = false;

	if (backend->capture_sw_params)
		snd_pcm_sw_params_free (backend->capture_sw_params);
	if (backend->capture_hw_params)
		snd_pcm_hw_params_free (backend->capture_hw_params);
	if (backend->playback_sw_params)
		snd_pcm_sw_params_free (backend->playback_sw_params);
	if (backend->playback_hw_params)
		snd_pcm_hw_params_free (backend->playback_hw_params);

	if (backend->handle_playback) snd_pcm_close (backend->handle_playback);
	if (backend->handle_capture) snd_pcm_close (backend->handle_capture);

	return true;
}

static void audio_handle_pcm_open_error(
	lively_audio_backend_t *backend,
	const char *device,
	const char *stream,
	int err) {

	switch (err) {
	case -EBUSY:
		log_error (backend,
			"The %s device \"%s\" is already in use", stream, device);
		break;
	case -EPERM:
		log_error (backend,
			"You do not have permission to use %s device \"%s\"",
			stream, device);
		break;
	default:
		log_error (backend,
			"Unknown error attempting to open %s device \"%s\" (%s)",
			stream, device, snd_strerror (err));
	}
}

static bool audio_configure_stream (
	lively_audio_backend_t *backend,
	const char *stream,
	snd_pcm_t *handle,
	snd_pcm_hw_params_t *hw_params,
	snd_pcm_sw_params_t *sw_params,
	unsigned int *channels) {

	int err;
	bool found;
	snd_pcm_uframes_t stop_threshold;
	unsigned int channels_max;
	unsigned int frames_per_period = backend->frames_per_period;
	unsigned int frames_per_second = backend->frames_per_second;
	unsigned int periods_per_buffer = backend->periods_per_buffer;

	static const snd_pcm_access_t try_access[] = {
		SND_PCM_ACCESS_MMAP_NONINTERLEAVED,
		SND_PCM_ACCESS_MMAP_INTERLEAVED,
		SND_PCM_ACCESS_MMAP_COMPLEX
	};

	static const struct {
		snd_pcm_format_t pcm_format;
		const char *id;
	} try_formats[] = {
		{SND_PCM_FORMAT_FLOAT_LE, "32bit float little-endian"},
		{SND_PCM_FORMAT_S32_LE, "32bit integer little-endian"},
		{SND_PCM_FORMAT_S32_BE, "32bit integer big-endian"},
		{SND_PCM_FORMAT_S24_3LE, "24bit 3byte little-endian"},
		{SND_PCM_FORMAT_S24_3BE, "24bit 3byte big-endian"},
		{SND_PCM_FORMAT_S24_LE, "24bit little-endian"},
		{SND_PCM_FORMAT_S24_BE, "24bit big-endian"},
		{SND_PCM_FORMAT_S16_LE, "16bit little-endian"},
		{SND_PCM_FORMAT_S16_BE, "16bit big-endian"}
	};

// This function narrows down the possible configurations for this
	// stream by allowing all configurations and narrowing down based off
	// of a sequence of constraints given to it.

	err = snd_pcm_hw_params_any (handle, hw_params);
	if (err < 0) {
		log_error (backend,
			"Could not initialize hardware params structure (%s)",
			snd_strerror (err));
		return false;
	}

	err = snd_pcm_hw_params_set_periods_integer (handle, hw_params);
	if (err < 0) {
		log_error (backend,
			"Could not restrict period size to integer value");
		return false;
	}

	// This attempts to set the access type for this stream to one of the
	// following configurations, in order.

	found = false;
	for (size_t i = 0; i < (sizeof try_access / sizeof *try_access); i++) {
		err = snd_pcm_hw_params_set_access (handle, hw_params, try_access[i]);
		if (err == 0) {
			found = true;
			break;
		}
	}
	if (!found) {
		log_error (backend,
			"Could not get mmap-based access to %s stream", stream);
		return false;
	}

	// This attemps to set the sampling rate for this stream to one of the
	// following configurations, in order.

	found = false;
	for (size_t i = 0; i < (sizeof try_formats / sizeof *try_formats); i++) {
		err = snd_pcm_hw_params_set_format (handle, hw_params, 
			try_formats[i].pcm_format);
		if (err == 0) {
			log_info (backend,
				"Setting sample format for %s stream to %s",
				stream, try_formats[i].id);
			found = true;
			break;
		}
	}
	if (!found) {
		log_error (backend,
			"Could not find supported sample format for %s stream",
			stream);
		return false;
	}

	err = snd_pcm_hw_params_set_rate_near (handle, hw_params,
		&frames_per_second, NULL);
	if (err < 0) {
		log_error (backend,
			"Could not set sample rate to %u Hz for %s stream",
			backend->frames_per_second, stream);
		return false;
	}
	if (frames_per_second != backend->frames_per_second) {
		log_warn (backend,
			"Setting sample rate to %u Hz instead of %u Hz on %s stream",
			frames_per_second, backend->frames_per_second, stream);
		backend->frames_per_second = frames_per_second;
	} else {
		log_info (backend,
			"Setting sample rate to %u Hz on %s stream", frames_per_second, stream);
	}

	err = snd_pcm_hw_params_get_channels_max (hw_params, &channels_max);
	if (err < 0) {
		log_error (backend,
			"Could not get maximum channel count available for %s stream",
			stream);
		return false;
	}

	err = snd_pcm_hw_params_set_channels (handle, hw_params, channels_max);
	if (err < 0) {
		log_error (backend,
			"Could not set channel count for %s stream", stream);
		return false;
	}

	err = snd_pcm_hw_params_set_period_size (handle, hw_params,
		backend->frames_per_period, 0);
	if (err < 0) {
		log_error (backend,
			"Could not set period size to %u frames for %s stream",
			backend->frames_per_period, stream);
		return false;
	}

	err = snd_pcm_hw_params_set_periods_min (handle, hw_params,
		&periods_per_buffer, NULL);
	if (periods_per_buffer < backend->periods_per_buffer) {
		backend->periods_per_buffer = periods_per_buffer;
	}
	err = snd_pcm_hw_params_set_periods_near (handle, hw_params,
		&periods_per_buffer, NULL);
	if (err < 0) {
		log_error (backend,
			"Could not set number of periods to %u for %s stream",
			periods_per_buffer, stream);
		return false;
	}

	if (periods_per_buffer < backend->periods_per_buffer) {
		log_error (backend,
			"Could only get %u periods instead of %u for %s stream",
			periods_per_buffer, backend->periods_per_buffer, stream);
		return false;
	}
	log_info (backend,
		"Setting periods to %u on %s stream", periods_per_buffer, stream);

	err = snd_pcm_hw_params_set_buffer_size (handle, hw_params,
		frames_per_period * periods_per_buffer);
	if (err < 0) {
		log_error (backend,
			"Could not set buffer length to %u for %s stream",
			frames_per_period * periods_per_buffer, stream);
		return false;
	}

	err = snd_pcm_hw_params (handle, hw_params);
	if (err < 0) {
		log_error (backend,
			"Could not set hardware parameters for %s stream", stream);
		return false;
	}

	snd_pcm_sw_params_current (handle, sw_params);
	
	err = snd_pcm_sw_params_set_start_threshold (handle, sw_params, 0U);
	if (err < 0) {
		log_error (backend,
			"Could not set start mode for %s stream", stream);
		return false;
	}

	stop_threshold = periods_per_buffer * frames_per_period;
	err = snd_pcm_sw_params_set_stop_threshold (handle, sw_params,
		stop_threshold);
	if (err < 0) {
		log_error (backend,
			"Could not set stop mode for %s stream", stream);
		return false;
	}

	err = snd_pcm_sw_params_set_silence_threshold (handle, sw_params, 0);
	if (err < 0) {
		log_error (backend,
			"Could not set silence threshold for %s stream", stream);
		return false;
	}

	err = snd_pcm_sw_params_set_avail_min (handle, sw_params, frames_per_period);
	if (err < 0) {
		log_error (backend,
			"Could not set available minimum for %s stream", stream);
		return false;
	}

	err = snd_pcm_sw_params (handle, sw_params);
	if (err < 0) {
		log_error (backend,
			"Could not set software parameters for %s stream");
		return false;
	}

	*channels = channels_max;

	return true;
}

static bool audio_configure_duplex (lively_audio_backend_t *backend) {
	unsigned int channels;

	// Params structures
	if (snd_pcm_hw_params_malloc (&backend->playback_hw_params) < 0) {
		log_error (backend,
			"Could not allocate playback hardware params structure");
		return false;
	}
	if (snd_pcm_sw_params_malloc (&backend->playback_sw_params) < 0) {
		log_error (backend,
			"Could not allocate playback software params structure");
		return false;
	}
	if (snd_pcm_hw_params_malloc (&backend->capture_hw_params) < 0) {
		log_error (backend,
			"Could not allocate capture hardware params structure");
		return false;
	}
	if (snd_pcm_sw_params_malloc (&backend->capture_sw_params) < 0) {
		log_error (backend,
			"Could not allocate capture software params structure");
		return false;
	}

	if (!audio_configure_stream (backend, "playback", 
		backend->handle_playback,
		backend->playback_hw_params,
		backend->playback_sw_params,
		&channels)) {

		return false;
	}
	backend->playback_channels = channels;

	if (!audio_configure_stream (backend, "capture",
		backend->handle_capture,
		backend->capture_hw_params,
		backend->capture_sw_params,
		&channels)) {

		return false;
	}
	backend->capture_channels = channels;

	return true;
}

