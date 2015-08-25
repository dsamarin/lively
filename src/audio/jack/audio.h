#ifndef JACK_AUDIO_H
#define JACK_AUDIO_H

#include <stdbool.h>

#include <jack/jack.h>

#include "../../lively_audio.h"

typedef struct lively_audio_backend {
	jack_client_t *client;

	bool started;
	unsigned int sample_rate;
	unsigned int buffer_length;
} lively_audio_backend_t;

#endif
