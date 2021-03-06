AM_CFLAGS = -std=c11 -pedantic -pedantic-errors -Wall -Werror -flto -Ofast -pthread

common_sources = \
	main.c \
	platform.h \
	lively_audio.c \
	lively_audio.h \
	lively_audio_backend.h \
	lively_audio_config.c \
	lively_audio_config.h \
	lively_app.c \
	lively_app.h \
	lively_node.c \
	lively_node.h \
	lively_scene.c \
	lively_scene.h \
	lively_thread.c \
	lively_thread.h

linux_sources = \
	platform/linux/signals.c \
	platform/linux/utils.c

windows_sources = \
	platform/windows/signals.c \
	platform/windows/utils.c

audio_sources = \
	audio/lively_audio_block.c

alsa_sources = \
	$(audio_sources) \
	audio/alsa/audio_format.c \
	audio/alsa/audio_format.h \
	audio/alsa/lively_audio_backend.c

jack_sources = \
	$(audio_sources) \
	audio/jack/lively_audio_backend.c

asio_sources = \
	$(audio_sources) \
	audio/asio/lively_audio_backend.c

if USE_ALSA
lively_alsa = lively_alsa
else
lively_alsa =
endif

if USE_JACK
lively_jack = lively_jack
else
lively_jack =
endif

if USE_ASIO
lively_asio = lively_asio
else
lively_asio =
endif

if OS_WINDOWS
platform_sources = $(windows_sources)
else
platform_sources = $(linux_sources)
endif

bin_PROGRAMS = $(lively_alsa) $(lively_jack) $(lively_asio)

lively_alsa_SOURCES = $(common_sources) $(platform_sources) $(alsa_sources)
lively_alsa_CFLAGS = $(AM_CFLAGS) $(ALSA_CFLAGS)
lively_alsa_LDADD = $(ALSA_LIBS)

lively_jack_SOURCES = $(common_sources) $(platform_sources) $(jack_sources)
lively_jack_CFLAGS = $(AM_CFLAGS) $(JACK_CFLAGS)
lively_jack_LDADD = $(JACK_LIBS)

lively_asio_SOURCES = $(common_sources) $(platform_sources) $(asio_sources)
lively_asio_CFLAGS = $(AM_CFLAGS) $(ASIO_CFLAGS)
lively_asio_LDADD = $(ASIO_LIBS)
