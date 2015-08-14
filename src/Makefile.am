CFLAGS = -std=c11 -pedantic -pedantic-errors -Wall -Werror -Os
LDFLAGS = 

MODULES = lively_app.c
MODULES_HEADER = $(MODULES:.c=.h)

if PLATFORM_WINDOWS
	MAYBE_WINDOWS = windows
	lively_LDADD += windows/platform.o
	lively_DEPENDENCIES = windows/platform.o
endif
if PLATFORM_LINUX
	MAYBE_LINUX = linux
	lively_LD += linux/platform.o
	lively_DEPENDENCIES = linux/platform.o
endif

SUBDIRS = $(MAYBE_LINUX) $(MAYBE_WINDOWS)

bin_PROGRAMS = lively

lively_SOURCES = main.c platform.h $(MODULES) $(MODULES_HEADER)
