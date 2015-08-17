#ifndef LIVELY_SCENE_NODE_H
#define LIVELY_SCENE_NODE_H

#include <stdbool.h>

struct lively_scene_node_plug;

typedef struct lively_scene_node {
	struct lively_scene_node *next;
	struct lively_scene_node_plug *plugs;
} lively_scene_node_t;

typedef enum { LIVELY_LEFT, LIVELY_RIGHT } lively_scene_node_port_t;

#define LIVELY_SCENE_NODE_CONNECTIONS_LENGTH 4

typedef struct lively_scene_node_plug {
	struct lively_scene_node_plug *next;
	lively_scene_node_t *source;
	lively_scene_node_t *target;
	lively_scene_node_port_t source_port;
	lively_scene_node_port_t target_port;
	bool filled;
} lively_scene_node_plug_t;

#endif
