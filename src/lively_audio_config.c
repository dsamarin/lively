#include "lively_audio_config.h"

void lively_audio_config_init (lively_audio_config_t *config) {
	config->frames_per_second = 48000;
	config->frames_per_period = 512;
	config->periods_per_buffer = 2;
	config->periods_per_buffer_in = 2;
	config->periods_per_buffer_out = 2;

	config->stream = AUDIO_CAPTURE | AUDIO_PLAYBACK;

	config->channels_in = 0;
	config->channels_out = 0;
}

float lively_audio_config_get_latency (lively_audio_config_t *config) {
	if (config->frames_per_second == 0) {
		return 0.0;
	}
	return (float) (config->frames_per_period * config->periods_per_buffer)
		/ config->frames_per_second;
}

