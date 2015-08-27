#ifndef ALSA_AUDIO_H
#define ALSA_AUDIO_H

#define _POSIX_C_SOURCE
#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>
#undef ALSA_PCM_NEW_HW_PARAMS_API
#undef ALSA_PCM_NEW_SW_PARAMS_API
#undef _POSIX_C_SOURCE

#include "../../lively_audio_backend.h"

typedef struct lively_audio_backend {
	snd_pcm_t *handle_playback;
	snd_pcm_t *handle_capture;
	snd_pcm_hw_params_t *playback_hw_params;
	snd_pcm_sw_params_t *playback_sw_params;
	snd_pcm_hw_params_t *capture_hw_params;
	snd_pcm_sw_params_t *capture_sw_params;

	bool connected;
	unsigned int playback_channels;
	unsigned int capture_channels;

	unsigned int frames_per_second;
	unsigned int frames_per_period;
	unsigned int periods_per_buffer;

	lively_audio_backend_logger_callback_t logger;
	void *logger_data;
} lively_audio_backend_t;

#endif
