/**
 * @file main.c
 * Contains the main entry point of lively.
 **/

#include <stdlib.h>

#include "platform.h"
#include "lively_app.h"

static lively_app_t lively;

static void shutdown (void) {
	lively_app_destroy (&lively);
	exit (0);
}

/**
* The main entry point of lively.
*
* Initializes and runs a new Lively Application into a static
* variable with file scope. This way, upon receiving the
* signal to quit by the user, we can clean up.
*
* @return Success value 
*/
int main(void) {
	platform_register_exit (&shutdown);

	if (lively_app_init (&lively)) {
		lively_app_run (&lively);
		lively_app_destroy (&lively);
	}

	return 0;
}
