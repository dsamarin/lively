#include <math.h>
#include <stdint.h>

#include "audio_format.h"

// TODO: Support big-endian
// (max - min) * (x - (-1)) / (1 - (-1)) + min
// (max - min) * (x + 1) / 2 + min

/* 32-bit float little-endian */
void sample_write_float_le (char *dst, float *src, unsigned int length, size_t skip) {
	while (length) {
		length--;
		
		*((float *) dst) = *src;
		dst += skip;
		src += 1;
	}
}

void sample_read_float_le (float *dst, char *src, unsigned int length, size_t skip) {
	while (length) {
		length--;

		*dst = *((float *) src);
		dst += 1;
		src += skip;
	}
}

/* 32-bit integer little-endian */

// max =  0x7fffffff
// min = -0x80000000
// (0x7fffffff - (-0x80000000) * (x + 1) / 2 + (-0x80000000)
// 0xffffffff * (x + 1) / 2 - 0x80000000
// (0xffffffff * x + 0xffffffff) / 2 - 0x80000000
// 0xffffffff/2 * x + 0xffffffff/2 - 0x80000000
// 0xffffffff/2 * x - 0.5
// 2147483647.5 x - 0.5

void sample_write_s32_le (char *dst, float *src, unsigned int length, size_t skip) {
	while (length) {
		length--;

		int32_t sample;
		if (*src < -1.0f) {
			sample = INT32_MIN;
		} else if (*src > 1.0f) {
			sample = INT32_MAX;
		} else {
			sample = lrintf ((*src) * INT32_MAX);
		}

		*((int32_t *) dst) = sample;
		dst += skip;
		src += 1;
	}
}
void sample_read_s32_le (float *dst, char *src, unsigned int length, size_t skip) {
	while (length) {
		length--;

		float sample = ((float) *((int32_t *) src)) / INT32_MAX;
		*dst = sample;
		src += skip;
		dst += 1;
	}
}

/* 16-bit little-endian */

void sample_write_s16_le (char *dst, float *src, unsigned int length, size_t skip) {
	while (length) {
		length--;

		int16_t sample;
		if (*src < -1.0f) {
			sample = INT16_MIN;
		} else if (*src > 1.0f) {
			sample = INT16_MAX;
		} else {
			sample = lrintf ((*src) * INT16_MAX);
		}

		*((int16_t *) dst) = sample;
		dst += skip;
		src += 1;
	}
}

void sample_read_s16_le (float *dst, char *src, unsigned int length, size_t skip) {
	while (length) {
		length--;

		float sample = ((float) *((int16_t *) src)) / INT16_MAX;
		*dst = sample;
		src += skip;
		dst += 1;
	}
}
