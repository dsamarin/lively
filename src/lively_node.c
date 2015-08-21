#include <stdlib.h>

#include "lively_node.h"

void
lively_node_io_init (lively_node_io_t *node_io, lively_node_type_t type) {
	lively_node_t *node = (lively_node_t *) node_io;

	node->type = type;
	node->process = lively_node_io_process;
	node->get_read_buffer = lively_node_io_get_buffer;
	node->get_write_buffer = lively_node_io_get_buffer;
	node->set_buffer_length = lively_node_io_set_buffer_length;

	node_io->buffer = NULL;
}

bool
lively_node_io_process(lively_node_t *node, unsigned int length) {
	return true;
}

/**
* Sets the buffer length for the specified Lively Node.
*
* This function is atomic, meaning no values will change if we will be
* unsuccessful, since we will allocate memory if it is needed.
*
* @param node The Lively Node
* @param length The new buffer length, in number of samples.
*
* @return A success value
*/
bool
lively_node_io_set_buffer_length(lively_node_t *node, unsigned int length) {
	unsigned int previous = node->buffer_length;
	lively_node_io_t *node_io = (lively_node_io_t *) node;

	if (length <= previous) {
		// Our buffer is already big enough.
		node->buffer_length = length;
		return true;
	}

	float *buffer = malloc (length * sizeof *buffer);
	if (!buffer) {
		// Memory allocation failed.
		return false;
	}

	if (node_io->buffer) {
		// We should free the old buffer first.
		free (node_io->buffer);
	}

	// Set the buffer to the new allocation
	node_io->buffer = buffer;
	node->buffer_length = length;

	return true;
}

float*
lively_node_io_get_buffer(lively_node_t *node, lively_node_channel_t channel) {
	return ((lively_node_io_t *) node)->buffer;
}


