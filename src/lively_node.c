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

bool
lively_node_io_set_buffer_length(lively_node_t *node, unsigned int length) {
	lively_node_io_t *node_io = (lively_node_io_t *) node;
	if (node_io->buffer) {
		if (node->buffer_length < length) {
			free (node_io->buffer);
		} else {
			return true;
		}
	}
	node_io->buffer = malloc (length * sizeof *node_io->buffer);
	return node_io->buffer != NULL;
}

float*
lively_node_io_get_buffer(lively_node_t *node, lively_node_channel_t channel) {
	return ((lively_node_io_t *) node)->buffer;
}


