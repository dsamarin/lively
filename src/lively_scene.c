/**
 * @file lively_scene.c
 * Lively Scene: A graph structure for Lively Nodes.
 */

#include <stdlib.h>

#include "lively_app.h"
#include "lively_scene.h"
#include "lively_node.h"

/**
* Initializes a new Lively Scene
*
* @param scene The pointer to the scene memory to initialize
* @param app Reference to the currently running Lively App
*/
void
lively_scene_init(lively_scene_t *scene, lively_app_t *app) {
	scene->app = app;
	scene->head = NULL;
	scene->name = "scene000";
}

/**
* Destroys a Lively Scene by cleaning up memory
*
* This function cleans up memory by disconnecting all plugs from each 
* Lively Node. The caller is responsible for cleaning up Lively Nodes.
*
* @param scene
*/
void
lively_scene_destroy(lively_scene_t *scene) {
	lively_node_t *node_iterator = scene->head;
	while (node_iterator) {
		lively_scene_disconnect_node (scene, node_iterator);
		node_iterator = node_iterator->next;
	}
}

/**
* Calls the callback specified for each Lively Node in the Lively Scene
*
* @param scene The Lively Scene
* @param callback The callback to call
* @param data User-supplied data
*/
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

/**
* Adds a Lively Node to the Lively Scene
*
* @param scene The Lively Scene
* @param node The Lively Node
*/
void
lively_scene_add_node(lively_scene_t *scene, lively_node_t *node) {
	lively_node_t *head = scene->head;
	scene->head = node;
	node->next = head;
}

/**
* Removes a Lively Node from the Lively Scene
*
* This function will produce a warning if the Lively Node doesn’t belong
* to the Lively Scene.
*
* @param scene The Lively Scene
* @param node The Lively Node
*/
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

/**
* Disconnects all plugs to and from the specified Lively Node
*
* @param scene The Lively Scene which contains the Lively Node
* @param node The Lively Node
*/
void
lively_scene_disconnect_node (lively_scene_t *scene, lively_node_t *node) {
	lively_node_t *node_iterator = scene->head;
	while (node_iterator) {
		if (node_iterator == node) {
			lively_node_plug_t *plug_iterator = node_iterator->plugs;
			while (plug_iterator) {
				lively_node_plug_t *plug_iterator_next = plug_iterator->next;
				free (plug_iterator);
				plug_iterator = plug_iterator_next;
			}
			node_iterator->plugs = NULL;
			break;
		}
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

/**
* Returns true if the plug specified exists in the Lively Scene
*
* @param scene The Lively Scene
* @param source The source node
* @param source_ch The source channel
* @param target The target node
* @param target_ch The target channel
*
* @return 
*/
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

/**
* Creates a plug from the source to the target
*
* This function will produce #LIVELY_WARN if the plug already exists,
* or a #LIVELY_FATAL if memory could not be allocated for the plug.
*
* @param scene The Lively Scene
* @param source The source node
* @param source_ch The source channel
* @param target The target node
* @param target_ch The target channel
*/
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

	// TODO: Check for cyclic ports

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

/**
* Removes a plug from the source to the target
*
* This function will produce #LIVELY_WARN if the plug doesn’t already exist.
*
* @param scene The Lively Scene
* @param source The source node
* @param source_ch The source channel
* @param target The target node
* @param target_ch The target channel
*/

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
	free (*plug);
	*plug = next;
}
