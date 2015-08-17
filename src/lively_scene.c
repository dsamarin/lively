#include <stdlib.h>

#include "lively_app.h"
#include "lively_scene.h"
#include "lively_scene_node.h"

void
lively_scene_init(lively_scene_t *scene, lively_app_t *app) {
	scene->nodes = NULL;
}

lively_scene_node_t *
lively_scene_list_nodes(lively_scene_t *scene) {
	return scene->nodes;
}

void
lively_scene_add_node(lively_scene_t *scene, lively_scene_node_t *node) {
	// We push the node onto the nodes list as a doubly-linked list
	lively_scene_node_t *first = scene->nodes;
	scene->nodes = node;
	node->next = first;
}

bool
lively_scene_remove_node(lively_scene_t *scene, lively_scene_node_t *node) {
	lively_scene_node_t *current = scene->nodes;

	if (current == node) {
		scene->nodes = node->next;
		return true;
	}

	while (current) {
		if (current->next == node) {
			current->next = node->next;
			return true;
		}
		current = current->next;
	}

	lively_app_log (
		scene->app,
		LIVELY_WARN,
		"scene",
		"Attempted to remove node from scene that is not part of the scene.");

	return false;
}

static inline bool
is_matching_scene_node_plug (
	lively_scene_node_plug_t *plug,
	lively_scene_node_t *source,
	lively_scene_node_port_t source_port,
	lively_scene_node_t *target,
	lively_scene_node_port_t target_port) {

	return (plug->source == source &&
		plug->target == target &&
		plug->source_port == source_port &&
		plug->target_port == target_port);
}

void
lively_scene_connect(
	lively_scene_t *scene,
	lively_scene_node_t *source,
	lively_scene_node_port_t source_port,
	lively_scene_node_t *target,
	lively_scene_node_port_t target_port) {

	lively_scene_node_plug_t *plug;

	// 1. Check if nodes are already connected there.
	plug = source->plugs;
	while (plug) {
		if (is_matching_scene_node_plug (plug, source, source_port, target, target_port)) {
			lively_app_log (
				scene->app,
				LIVELY_WARN,
				"scene",
				"Attempted to connect ports that are already connected");
			return;
		}
		plug = plug->next;
	}

	// 2. Allocate new connection
	plug = malloc (sizeof *plug);
	if (!plug) {
		lively_app_log (
			scene->app,
			LIVELY_FATAL,
			"scene",
			"Could not allocate memory");
		return;
	}

	plug->source = source;
	plug->target = target;
	plug->source_port = source_port;
	plug->target_port = target_port;
	plug->filled = false;
	plug->next = NULL;

	// 3. Insert connection
	lively_scene_node_plug_t *first = source->plugs;
	source->plugs = plug;
	plug->next = first;
}

void
lively_scene_disconnect(
	lively_scene_t *scene,
	lively_scene_node_t *source,
	lively_scene_node_port_t source_port,
	lively_scene_node_t *target,
	lively_scene_node_port_t target_port) {

	lively_scene_node_plug_t *plug = source->plugs;
	lively_scene_node_plug_t *prev = NULL;

	while (plug) {
		if (is_matching_scene_node_plug (plug, source, source_port, target, target_port)) {
			if (prev) {
				prev->next = plug->next;
			} else {
				source->plugs = plug->next;
			}
			free (plug);
			return;
		}
		prev = plug;
		plug = plug->next;
	}

	lively_app_log (
		scene->app,
		LIVELY_WARN,
		"scene",
		"Attempted to disconnect port that was non-existant");
}
