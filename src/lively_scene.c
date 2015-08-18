#include <stdlib.h>

#include "lively_app.h"
#include "lively_scene.h"
#include "lively_node.h"

void
lively_scene_init(lively_scene_t *scene, lively_app_t *app) {
	scene->app = app;
	scene->head = NULL;
	scene->name = "scene000";
}

void
lively_scene_nodes_foreach(
	lively_scene_t *scene,
	void (*callback) (lively_scene_t *scene, lively_node_t *node, void *data),
	void *data) {

	lively_node_t *node = scene->head;
	while (node) {
		callback (scene, node, data);
		node = node->next;
	}
}

void
lively_scene_add_node(lively_scene_t *scene, lively_node_t *node) {
	lively_node_t *head = scene->head;
	scene->head = node;
	node->next = head;
}

void
lively_scene_remove_node(lively_scene_t *scene, lively_node_t *node) {
	lively_node_t **iterator = &scene->head;
	while (*iterator) {
		if (*iterator == node) {
			lively_scene_disconnect_node (scene, node);

			lively_node_t *next = (*iterator)->next;
			*iterator = next;
			return;
		}
		iterator = &(*iterator)->next;
	}

	lively_app_log (
		scene->app,
		LIVELY_WARN,
		"scene",
		"Attempt to remove non-existant node '%s' from scene '%s'",
		node->name,
		scene->name);
}

void
lively_scene_disconnect_node (lively_scene_t *scene, lively_node_t *node) {
	lively_node_t *node_iterator = scene->head;
	while (node_iterator) {
		lively_node_plug_t *plug_iterator = node_iterator->plugs;
		while (plug_iterator) {
			lively_node_plug_t *plug_iterator_next = plug_iterator->next;
			free (plug_iterator);
			plug_iterator = plug_iterator_next;
		}
		node_iterator->plugs = NULL;
		node_iterator = node_iterator->next;
	}
}

static lively_node_plug_t **
scene_find_plug (
	lively_scene_t *scene,
	lively_node_t *source,
	lively_node_channel_t source_ch,
	lively_node_t *target,
	lively_node_channel_t target_ch) {

	lively_node_plug_t **plug_iterator = &source->plugs;
	while (*plug_iterator) {
		bool target_match = (*plug_iterator)->target == target;
		bool channel_match = (*plug_iterator)->target_ch == target_ch
			&& (*plug_iterator)->source_ch == source_ch;

		if (target_match && channel_match) {
			return plug_iterator;
		}
		plug_iterator = &(*plug_iterator)->next;
	}

	return NULL;
}

bool
lively_scene_is_connected (
	lively_scene_t *scene,
	lively_node_t *source,
	lively_node_channel_t source_ch,
	lively_node_t *target,
	lively_node_channel_t target_ch) {

	return scene_find_plug (
		scene, source, source_ch, target, target_ch) != NULL;
}

void
lively_scene_connect(
	lively_scene_t *scene,
	lively_node_t *source,
	lively_node_channel_t source_ch,
	lively_node_t *target,
	lively_node_channel_t target_ch) {

	lively_node_plug_t *plug;

	if (lively_scene_is_connected (
		scene, source, source_ch, target, target_ch)) {

		lively_app_log (
			scene->app,
			LIVELY_WARN,
			"scene",
			"Attempted to connect ports that are already connected");
		return;
	}

	plug = malloc (sizeof *plug);
	if (!plug) {
		lively_app_log (
			scene->app,
			LIVELY_FATAL,
			"scene",
			"Could not allocate memory");
		return;
	}

	plug->next = NULL;
	plug->target = target;
	plug->source_ch = source_ch;
	plug->target_ch = target_ch;

	lively_node_plug_t *head = source->plugs;
	source->plugs = plug;
	plug->next = head;
}

void
lively_scene_disconnect(
	lively_scene_t *scene,
	lively_node_t *source,
	lively_node_channel_t source_ch,
	lively_node_t *target,
	lively_node_channel_t target_ch) {

	lively_node_plug_t **plug;

	plug = scene_find_plug (scene, source, source_ch, target, target_ch);

	if (!plug) {
		lively_app_log (
			scene->app,
			LIVELY_WARN,
			"scene",
			"Attempted to disconnect ports that are not connected");
		return;
	}

	lively_node_plug_t *next = (*plug)->next;
	*plug = next;

	free (plug);
}
