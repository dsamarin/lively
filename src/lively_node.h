#ifndef LIVELY_NODE_H
#define LIVELY_NODE_H

#include <stdbool.h>

/**
 * Specifies a type of Lively Node.
 *
 * Lively Scenes are mapped to the Lively Audio interface through the use of
 * input and output nodes.
 */
typedef enum lively_node_type {
	LIVELY_NODE_PROCESS = 1 << 0, /**< Process nodes have functions which filter audio */
	LIVELY_NODE_INPUT = 1 << 1, /**< Input nodes map virtual input devices into the Lively Scene */
	LIVELY_NODE_OUTPUT = 1 << 2 /**< Output nodes map virtual output devices from the Lively Scene */
} lively_node_type_t;

/**
 * Specifies a named channel on a Lively Node
 */
typedef enum lively_node_channel {
	LIVELY_MONO, /**< The main mono channel */
	LIVELY_LEFT, /**< The main stereo left channel */
	LIVELY_RIGHT /**< The main stereo right channel */
} lively_node_channel_t;

typedef struct lively_node_plug {
	struct lively_node_plug *next;

	struct lively_node *target;
	enum lively_node_channel source_ch;
	enum lively_node_channel target_ch;
} lively_node_plug_t;

// TODO: Figure out what access needs to be atomized.
typedef struct lively_node {
	struct lively_node *next;

	struct lively_node_plug *plug_head;
	unsigned int inputs_ready;
	unsigned int inputs_total;

	enum lively_node_type type;
	char *name;

	unsigned int buffer_length;
	bool (*process)(struct lively_node *, unsigned int size);
	bool (*set_buffer_length)(struct lively_node *, unsigned int count);
	float *(*get_read_buffer)(struct lively_node *, lively_node_channel_t);
	float *(*get_write_buffer)(struct lively_node *, lively_node_channel_t);
} lively_node_t;

typedef struct lively_node_io {
	struct lively_node node;
	
	float *buffer;
} lively_node_io_t;

void lively_node_io_init (lively_node_io_t *, lively_node_type_t);
bool lively_node_io_process (lively_node_t *, unsigned int);
bool lively_node_io_set_buffer_length(lively_node_t *, unsigned int);
float* lively_node_io_get_buffer(lively_node_t *, lively_node_channel_t);


#endif
