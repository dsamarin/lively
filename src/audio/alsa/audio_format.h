#ifndef ALSA_AUDIO_FORMAT_H
#define ALSA_AUDIO_FORMAT_H

#include <stddef.h>

typedef void (*sample_write_func_t) (char *, float *, unsigned int, size_t);
typedef void (*sample_read_func_t) (float *, char *, unsigned int, size_t);

void sample_read_float_le (float *, char *, unsigned int, size_t);
void sample_write_float_le (char *, float *, unsigned int, size_t);

void sample_read_s32_le (float *, char *, unsigned int, size_t);
void sample_write_s32_le (char *, float *, unsigned int, size_t);

void sample_read_s16_le (float *, char *, unsigned int, size_t);
void sample_write_s16_le (char *, float *, unsigned int, size_t);

#endif
