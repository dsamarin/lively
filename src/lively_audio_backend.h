#ifndef LIVELY_AUDIO_BACKEND_H
#define LIVELY_AUDIO_BACKEND_H

#include <stdbool.h>

#include "lively_app.h"
#include "lively_audio_config.h"

typedef struct lively_audio_channel {
	bool ready;
	float *data;
} lively_audio_channel_t;

typedef struct lively_audio_block {
	unsigned int frames;
	unsigned int avail_in;
	unsigned int avail_out;

	float *silence;

	unsigned int num_in;
	unsigned int num_out;

	lively_audio_channel_t *in;
	lively_audio_channel_t *out;
} lively_audio_block_t;

bool lively_audio_block_init (lively_audio_block_t *, lively_audio_config_t *);
void lively_audio_block_destroy (lively_audio_block_t *);
void lively_audio_block_silence_output (lively_audio_block_t *);

typedef struct lively_audio_backend lively_audio_backend_t;
typedef void (*lively_audio_backend_logger_callback_t) (
	void *, enum lively_log_level, const char *, ...);

const char *
lively_audio_backend_name (lively_audio_backend_t *);

lively_audio_backend_t *
lively_audio_backend_new (lively_audio_config_t *);

void
lively_audio_backend_delete (lively_audio_backend_t **);

void
lively_audio_backend_set_logger (
	lively_audio_backend_t *,
	lively_audio_backend_logger_callback_t,
	void *);

bool lively_audio_backend_connect (lively_audio_backend_t *);
bool lively_audio_backend_disconnect (lively_audio_backend_t *);

bool lively_audio_backend_start (lively_audio_backend_t *, lively_audio_block_t *);
bool lively_audio_backend_stop (lively_audio_backend_t *);

bool lively_audio_backend_wait (lively_audio_backend_t *);

bool lively_audio_backend_read (lively_audio_backend_t *, lively_audio_block_t *);
bool lively_audio_backend_write (lively_audio_backend_t *, lively_audio_block_t *);

#endif
