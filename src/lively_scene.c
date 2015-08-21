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
	scene->buffer_length = 512;
	scene->head = NULL;
	scene->name = "scene000";
}

/**
* Destroys a Lively Scene by cleaning up memory
*
* This function cleans up memory by disconnecting all plug_head from each 
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
* Returns the current buffer length that is set for all of the Lively Nodes
* belonging to this Lively Scene.
*
* @param scene The Lively Scene
*
* @return The buffer length, in number of samples
*/
unsigned int
lively_scene_get_buffer_length (lively_scene_t *scene) {
	return scene->buffer_length;
}

/**
* Sets the buffer length for all nodes in the Lively Scene, atomically, and
* returns its success.
*
* The function first sets the buffer length for all nodes, but if we are
* unsuccessful, we will revert back to the previous buffer length.
*
* @param scene The Lively Scene
* @param length The buffer length, in number of samples
*
* @return A success value
*/
bool
lively_scene_set_buffer_length (lively_scene_t *scene, unsigned int length) {
	bool success = true;
	unsigned int previous = scene->buffer_length;

	lively_node_t *node_iterator = scene->head;
	while (node_iterator) {
		if (!node_iterator->set_buffer_length (node_iterator, length)) {
			success = false;
			break;
		}
		node_iterator = node_iterator->next;
	}

	if (success) {
		scene->buffer_length = length;
	} else {
		// Revert back
		node_iterator = scene->head;
		while (node_iterator) {
			node_iterator->set_buffer_length (node_iterator, previous);
			node_iterator = node_iterator->next;
		}
	}

	return success;
}

/**
* Adds a Lively Node to the Lively Scene
*
* @param scene The Lively Scene
* @param node The Lively Node
*
* @return A success value
*/
bool
lively_scene_add_node(lively_scene_t *scene, lively_node_t *node) {
	lively_node_t *head = scene->head;
	scene->head = node;

	node->next = head;

	node->plug_head = NULL;
	node->inputs_ready = 0;
	node->inputs_total = 0;

	return node->set_buffer_length (node, scene->buffer_length);
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
* Disconnects all plug_head to and from the specified Lively Node
*
* @param scene The Lively Scene which contains the Lively Node
* @param node The Lively Node
*/
void
lively_scene_disconnect_node (lively_scene_t *scene, lively_node_t *node) {
	lively_node_t *node_iterator = scene->head;
	while (node_iterator) {
		if (node_iterator == node) {
			lively_node_plug_t *plug_iterator = node_iterator->plug_head;
			while (plug_iterator) {
				lively_node_plug_t *plug_iterator_next = plug_iterator->next;

				// Disconnect, then free plug.
				plug_iterator->target->inputs_total--;
				free (plug_iterator);

				plug_iterator = plug_iterator_next;
			}
			node_iterator->plug_head = NULL;
			break;
		}
		node_iterator = node_iterator->next;
	}
}

static void
lively_scene_process_node (
	lively_scene_t *scene,
	lively_node_t *node,
	unsigned int length) {

	// First we call the node's process() func.
	bool success = node->process (node, length);
	if (!success) {
		// TODO: Safe logging from audio processing thread.
		/*lively_app_log (scene->app, LIVELY_WARN, "scene",
			"Node '%s' in scene '%s' reported processing failure",
			node->name, scene->name);*/
		return;
	}

	lively_node_plug_t *plug_iterator = node->plug_head;
	while (plug_iterator) {
		// #plug = *plug_iterator
		// #target = *plug_iterator->target
		//
		// Given: #plug is written.
		// Goal: All plug_head to #target are written.
		//
		// It follows that to reach the goal, we need to know if there are any
		// other plug_head. If there are no more other plug_head to target, it follows
		// that we have accomplished our goal. Otherwise, we need to wait for
		// the other plug_head before we go on.
		//
		// We can keep track of the number of written plug_head with a temporary
		// lengther variable on #target. This can be reset after the node is
		// processed.
		//
		// #target.inputs_ready += 1.
		// if (#target.inputs_ready == #target.inputs_total) {
		// 		#goal reached
		// } else {
		// 		#skip target, not ready
		// }

		lively_node_plug_t *plug = plug_iterator;
		lively_node_t *target = plug->target;

		float *source_buffer = node->get_read_buffer (node, plug->source_ch);
		float *target_buffer = target->get_write_buffer (target, plug->target_ch);
		for (size_t i = 0; i < length; i++) {
			target_buffer[i] += source_buffer[i];
		}

		target->inputs_ready += 1;
		if (target->inputs_ready == target->inputs_total) {
			target->inputs_ready = 0;
			lively_scene_process_node (scene, target, length);
		}

		plug_iterator = plug_iterator->next;
	}
}

void
lively_scene_process (lively_scene_t *scene, unsigned int length) {
	// We do a depth-first search, starting with the input nodes.
	lively_node_t *node_iterator = scene->head;
	while (node_iterator) {
		if (node_iterator->type == LIVELY_NODE_INPUT) {
			lively_scene_process_node (scene, node_iterator, length);
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

	lively_node_plug_t **plug_iterator = &source->plug_head;
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
* @return true if the plug exists
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
			"Attempted to connect channels that are already connected");
		return;
	}

	// TODO: Check for cyclic connections

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

	lively_node_plug_t *head = source->plug_head;
	source->plug_head = plug;
	plug->next = head;

	target->inputs_total++;
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
			"Attempted to disconnect channels that are not connected");
		return;
	}

	lively_node_plug_t *next = (*plug)->next;
	free (*plug);
	*plug = next;

	target->inputs_total--;
}
