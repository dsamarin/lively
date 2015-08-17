#ifndef LIVELY_SCENE_H
#define LIVELY_SCENE_H

#include <stdbool.h>

#include "lively_app.h"
#include "lively_scene_node.h"

typedef struct lively_scene {
	lively_app_t *app;
	lively_scene_node_t *nodes;
} lively_scene_t;

void lively_scene_init(lively_scene_t *scene, lively_app_t *app);

lively_scene_node_t * lively_scene_list_nodes(lively_scene_t *scene);
void lively_scene_add_node(lively_scene_t *scene, lively_scene_node_t *node);
bool lively_scene_remove_node(lively_scene_t *scene, lively_scene_node_t *node);

void lively_scene_connect(lively_scene_t *scene, lively_scene_node_t *source, unsigned int source_port, lively_scene_node_t *target, unsigned int target_port);
void lively_scene_disconnect(lively_scene_t *scene, lively_scene_node_t *source, unsigned int source_port, lively_scene_node_t *target, unsigned int target_port);

#endif
