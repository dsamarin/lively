#include <stdio.h>

#include "lively_app.h"

#include "platform.h"

void lively_app_init (lively_app_t *app) {
	printf ("Starting lively.\n");
}
void lively_app_run (lively_app_t *app) {
	printf ("Running lively.\n");
	platform_sleep (3);
}
void lively_app_destroy (lively_app_t *app) {
	printf ("Stopping lively.\n");
}
