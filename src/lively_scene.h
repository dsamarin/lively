#ifndef LIVELY_SCENE_H
#define LIVELY_SCENE_H

#include <stdbool.h>

#include "lively_app.h"
#include "lively_node.h"

typedef struct lively_scene {
	lively_app_t *app;
	lively_node_t *head;

	const char *name;
} lively_scene_t;

void lively_scene_init(lively_scene_t *scene, lively_app_t *app);

void lively_scene_nodes_foreach(lively_scene_t *scene, void (*callback) (lively_scene_t *scene, lively_node_t *node, void *data), void *data);

void lively_scene_add_node(lively_scene_t *scene, lively_node_t *node);
void lively_scene_remove_node(lively_scene_t *scene, lively_node_t *node);
void lively_scene_disconnect_node (lively_scene_t *scene, lively_node_t *node);

void lively_scene_connect(lively_scene_t *scene, lively_node_t *source, unsigned int source_port, lively_node_t *target, unsigned int target_port);
void lively_scene_disconnect(lively_scene_t *scene, lively_node_t *source, unsigned int source_port, lively_node_t *target, unsigned int target_port);

#endif
