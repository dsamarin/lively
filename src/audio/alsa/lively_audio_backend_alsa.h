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
	lively_audio_config_t *config;

	snd_pcm_t *playback;
	snd_pcm_t *capture;
	snd_pcm_hw_params_t *playback_hw_params;
	snd_pcm_sw_params_t *playback_sw_params;
	snd_pcm_hw_params_t *capture_hw_params;
	snd_pcm_sw_params_t *capture_sw_params;

	int poll_timeout;
	struct pollfd* poll_fds;
	unsigned int poll_fds_count_playback;
	unsigned int poll_fds_count_capture;

	bool linked;
	bool connected;

	lively_audio_backend_logger_callback_t logger;
	void *logger_data;
} lively_audio_backend_t;

typedef struct audio_mmap {
	const snd_pcm_channel_area_t *areas;
	snd_pcm_uframes_t offset;
	snd_pcm_uframes_t frames;
} audio_mmap_t;

#endif
