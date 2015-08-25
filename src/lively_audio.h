#ifndef LIVELY_AUDIO_H
#define LIVELY_AUDIO_H

#include <stdbool.h>

struct lively_app;
struct lively_audio_backend;

typedef struct lively_audio {
	struct lively_app *app;
	struct lively_audio_backend *backend;
} lively_audio_t;

bool lively_audio_init (struct lively_audio *audio, struct lively_app *app);
void lively_audio_destroy (struct lively_audio *audio);

bool lively_audio_start (struct lively_audio *audio);
bool lively_audio_stop (struct lively_audio *audio);

#endif
