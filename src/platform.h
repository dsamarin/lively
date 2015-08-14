#ifndef PLATFORM_H
#define PLATFORM_H

void platform_register_exit(void (*callback)(void));
void platform_sleep(unsigned int seconds);

#endif
