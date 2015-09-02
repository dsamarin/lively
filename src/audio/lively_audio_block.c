#include <stdlib.h>

#include "../lively_audio_backend.h"

bool
lively_audio_block_init (
	lively_audio_block_t *block,
	lively_audio_config_t *config) {

	unsigned int i;

	block->frames = config->frames_per_period;
	block->avail_in = 0;
	block->avail_out = 0;
	block->num_in = config->channels_in;
	block->num_out = config->channels_out;

	block->in = malloc (block->num_in * (sizeof *block->in));
	if (!block->in) {
		return false;
	}

	block->out = malloc (block->num_out * (sizeof *block->out));
	if (!block->out) {
		free (block->in);
		block->in = NULL;
		return false;
	}

	for (i = 0; i < block->num_in; i++) {
		block->in[i].ready = false;
		block->in[i].data = NULL;
	}

	for (i = 0; i < block->num_out; i++) {
		block->out[i].ready = false;
		block->out[i].data = NULL;
	}

	for (i = 0; i < block->num_in; i++) {
		block->in[i].data = malloc (block->frames * sizeof (float));
		if (!block->in[i].data) {
			return false;
		}
	}
	for (i = 0; i < block->num_out; i++) {
		block->out[i].data = malloc (block->frames * sizeof (float));
		if (!block->out[i].data) {
			return false;
		}
	}

	return true;
}

void
lively_audio_block_silence_output (lively_audio_block_t *block) {
	unsigned int i, j;

	for (i = 0; i < block->num_out; i++) {
		float *buffer = block->out[i].data;
		for (j = 0; j < block->frames; j++) {
			buffer[j] = 0.0;
		}
		block->out[i].ready = true;
	}
}

void
lively_audio_block_destroy (lively_audio_block_t *block) {
	unsigned int i;

	for (i = 0; i < block->num_in; i++) {
		free (block->in[i].data);
		block->in[i].data = NULL;
	}
	for (i = 0; i < block->num_out; i++) {
		free (block->out[i].data);
		block->out[i].data = NULL;
	}

	if (!block->in) {
		free (block->in);
		block->in = NULL;
	}
	if (!block->out) {
		free (block->out);
		block->out = NULL;
	}
}
