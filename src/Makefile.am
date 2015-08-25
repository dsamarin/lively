AM_CFLAGS = -std=c11 -pedantic -pedantic-errors -Wall -Werror -Ofast
AM_LDFLAGS =

bin_PROGRAMS = lively

common_sources = \
	main.c \
	platform.h \
	lively_audio.h \
	lively_app.c \
	lively_app.h \
	lively_node.c \
	lively_node.h \
	lively_scene.c \
	lively_scene.h

os_linux_sources = \
	platform/linux/signals.c \
	platform/linux/utils.c

os_windows_sources = \
	platform/windows/signals.c \
	platform/windows/utils.c

audio_alsa_sources = \
	audio/alsa/lively_audio.c

audio_jack_sources = \
	audio/jack/lively_audio.c

audio_asio_sources = \
	audio/asio/lively_audio.c

lively_SOURCES = $(common_sources)
lively_CFLAGS = $(AM_CFLAGS)

#
# Platform configuration
#
if OS_LINUX
lively_SOURCES += $(os_linux_sources)
endif
if OS_WINDOWS
lively_SOURCES += $(os_windows_sources)
endif

#
# Audio API configuration
#
if USE_ALSA
lively_SOURCES += $(audio_alsa_sources)
lively_CFLAGS += $(ALSA_CFLAGS)
lively_LDADD = $(ALSA_LIBS)
endif
if USE_JACK
lively_SOURCES += $(audio_jack_sources)
endif
if USE_ASIO
lively_SOURCES += $(audio_asio_sources)
endif
