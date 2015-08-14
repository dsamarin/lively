#ifndef LIVELY_APP_H
#define LIVELY_APP_H

typedef struct lively_app {
	void *nada;
} lively_app_t;

void lively_app_init (lively_app_t *);
void lively_app_run (lively_app_t *);
void lively_app_destroy (lively_app_t *);

#endif
