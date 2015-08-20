#ifndef LIVELY_AUDIO_H
#define LIVELY_AUDIO_H

#include <stdbool.h>

#include <jack/jack.h>

struct lively_app;

typedef struct lively_audio {
	struct lively_app *app;
	jack_client_t *client;

	bool started;
	unsigned int sample_rate;
	unsigned int buffer_length;
} lively_audio_t;

bool lively_audio_init (struct lively_audio *audio, struct lively_app *app);
void lively_audio_destroy (struct lively_audio *audio);

bool lively_audio_start (struct lively_audio *audio);
bool lively_audio_stop (struct lively_audio *audio);

#endif
