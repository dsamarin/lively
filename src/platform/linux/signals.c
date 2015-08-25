#include <stdlib.h>
#include <signal.h>

#include "../../platform.h"

static void (*callback_exit)(void) = NULL;

static void signal_handler(int signal) {
	if (callback_exit) {
		callback_exit();
	}
}

void platform_register_exit(void (*callback)(void)) {
	callback_exit = callback;

	signal (SIGQUIT, signal_handler);
	signal (SIGTERM, signal_handler);
	signal (SIGHUP, signal_handler);
	signal (SIGINT, signal_handler);
}
