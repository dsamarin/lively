AM_CFLAGS = -std=c11 -pedantic -pedantic-errors -Wall -Werror -Os
AM_LDFLAGS = 

SUBDIRS = $(PLATFORM)
DIST_SUBDIRS = $(PLATFORM_LIST)

bin_PROGRAMS = lively

lively_SOURCES = main.c \
	lively_app.c \
	lively_app.h \
	lively_audio.c \
	lively_audio.h \
	lively_scene.c \
	lively_scene.h \
	lively_scene_node.h

lively_LDADD = $(PLATFORM)/libplatform.a
lively_DEPENDENCIES = $(PLATFORM)/libplatform.a