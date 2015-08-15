#ifndef LIVELY_AUDIO_H
#define LIVELY_AUDIO_H

#include <stdbool.h>

#include <jack/jack.h>

#include "lively_app.h"

typedef struct lively_audio {
	lively_app_t *app;
	jack_client_t *client;

	bool started;
	unsigned int sample_rate;
} lively_audio_t;

bool lively_audio_init (lively_audio_t *audio, lively_app_t *app);
void lively_audio_destroy (lively_audio_t *audio);

bool lively_audio_start (lively_audio_t *audio);
bool lively_audio_stop (lively_audio_t *audio);

#endif
