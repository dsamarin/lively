#include <unistd.h>

#include "../platform.h"

void platform_sleep(unsigned int seconds) {
	sleep (seconds);
}
