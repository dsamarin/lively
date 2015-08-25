AM_CFLAGS = -std=c11 -pedantic -pedantic-errors -Wall -Werror -Ofast
AM_LDFLAGS =

SUBDIRS = $(PLATFORM)
DIST_SUBDIRS = $(PLATFORM_LIST)

bin_PROGRAMS = lively

lively_SOURCES = main.c \
	lively_app.c \
	lively_app.h \
	lively_audio.c \
	lively_audio.h \
	lively_node.c \
	lively_node.h \
	lively_scene.c \
	lively_scene.h

lively_CFLAGS = $(AM_CFLAGS) $(ALSA_CFLAGS)
lively_LDADD = $(PLATFORM)/libplatform.a $(ALSA_LIBS)
lively_DEPENDENCIES = $(PLATFORM)/libplatform.a
