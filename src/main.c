#include <stdlib.h>

#include "platform.h"
#include "lively_app.h"

static lively_app_t lively;

static void shutdown (void) {
	lively_app_destroy (&lively);
	exit (0);
}

int main(void) {
	platform_register_exit (&shutdown);

	lively_app_init (&lively);
	lively_app_run (&lively);
	lively_app_destroy (&lively);
	return 0;
}
