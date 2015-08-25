#ifndef LIVELY_AUDIO_H
#define LIVELY_AUDIO_H

#include <stdbool.h>

#define _POSIX_C_SOURCE
#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>
#undef ALSA_PCM_NEW_HW_PARAMS_API
#undef ALSA_PCM_NEW_SW_PARAMS_API
#undef _POSIX_C_SOURCE

struct lively_app;

typedef struct lively_audio {
	struct lively_app *app;

	snd_pcm_t *handle_playback;
	snd_pcm_t *handle_capture;
	snd_pcm_hw_params_t *playback_hw_params;
	snd_pcm_sw_params_t *playback_sw_params;
	snd_pcm_hw_params_t *capture_hw_params;
	snd_pcm_sw_params_t *capture_sw_params;

	bool started;
	unsigned int playback_channels;
	unsigned int capture_channels;

	unsigned int frames_per_second;
	unsigned int frames_per_period;
	unsigned int periods_per_buffer;
} lively_audio_t;

bool lively_audio_init (struct lively_audio *audio, struct lively_app *app);
void lively_audio_destroy (struct lively_audio *audio);

bool lively_audio_start (struct lively_audio *audio);
bool lively_audio_stop (struct lively_audio *audio);

#endif
