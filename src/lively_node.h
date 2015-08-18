#ifndef lively_node_H
#define lively_node_H

#include "lively_scene.h"

struct lively_node;

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

typedef struct lively_node {
	struct lively_node *next;
	struct lively_node_plug *plugs;

	char *name;
} lively_node_t;

#endif
