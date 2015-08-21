#ifndef LIVELY_SCENE_H
#define LIVELY_SCENE_H

#include <stdbool.h>

struct lively_app;

#include "lively_node.h"

typedef struct lively_scene {
	struct lively_app *app;
	struct lively_node *head;

	unsigned int buffer_length;

	const char *name;
} lively_scene_t;

void lively_scene_init(struct lively_scene *scene, struct lively_app *app);
void lively_scene_destroy(struct lively_scene *scene);

void lively_scene_nodes_foreach(
	struct lively_scene *scene,
	void (*callback) (struct lively_scene *scene, struct lively_node *node, void *data),
	void *data);

bool lively_scene_add_node(struct lively_scene *scene, struct lively_node *node);
void lively_scene_remove_node(struct lively_scene *scene, struct lively_node *node);
void lively_scene_disconnect_node (struct lively_scene *scene, struct lively_node *node);

void lively_scene_process (struct lively_scene *scene, unsigned int count);

bool lively_scene_is_connected (
	struct lively_scene *scene,
	struct lively_node *source,
	enum lively_node_channel source_ch,
	struct lively_node *target,
	enum lively_node_channel target_ch);
void lively_scene_connect(
	struct lively_scene *scene,
	struct lively_node *source,
	enum lively_node_channel source_ch,
	struct lively_node *target,
	enum lively_node_channel target_ch);
void lively_scene_disconnect(
	struct lively_scene *scene,
	struct lively_node *source,
	enum lively_node_channel source_ch,
	struct lively_node *target,
	enum lively_node_channel target_ch);

#endif
