#include <windows.h>

#include "../platform.h"

void platform_sleep(unsigned int seconds) {
	Sleep (1000 * seconds);
}
