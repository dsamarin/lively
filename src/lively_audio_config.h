#ifndef LIVELY_AUDIO_CONFIG_H
#define LIVELY_AUDIO_CONFIG_H

enum lively_audio_stream {
	AUDIO_NONE = 0,
	AUDIO_PLAYBACK = 1 << 0,
	AUDIO_CAPTURE = 1 << 1
};

typedef struct lively_audio_config {
	unsigned int frames_per_second;
	unsigned int frames_per_period;
	unsigned int periods_per_buffer;
	unsigned int periods_per_buffer_in;
	unsigned int periods_per_buffer_out;

	enum lively_audio_stream stream;

	unsigned int channels_in;
	unsigned int channels_out;
} lively_audio_config_t;

void lively_audio_config_init (lively_audio_config_t *config);
float lively_audio_config_get_latency (lively_audio_config_t *config);

#endif
