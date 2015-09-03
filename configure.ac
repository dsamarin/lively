# Initialization
AC_PREREQ([2.69])
AC_INIT(
	[Lively],
	[0.1],
	[https://github.com/dsamarin/lively/issues],
	[lively],
	[https://dsamar.in/lively/])
AC_SUBST([PACKAGE_BRIEF], ['Lively is an open-source application for live mixing and effects processing.'])
AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_SRCDIR([src/main.c])

CFLAGS="$CFLAGS"

# Specify options
AC_ARG_WITH([alsa],
	AS_HELP_STRING([--with-alsa], [Enable support for ALSA]),
	[with_alsa=$withval])
AC_ARG_WITH([jack],
	AS_HELP_STRING([--with-jack], [Enable support for JACK]),
	[with_jack=$withval])
AC_ARG_WITH([asio],
	AS_HELP_STRING([--with-asio], [Enable support for ASIO]),
	[with_asio=$withval])

os_windows=no
os_linux=no
case "$host" in
	*mingw*) os_windows=yes ;;
	*) os_linux=yes ;;
esac

# Sanity check
if test "$with_asio" != "yes" \
	&& test "$with_alsa" != "yes" \
	&& test "$with_jack" != "yes"; then

	AC_MSG_NOTICE([*** No audio backend was specified. Enabling --with-alsa ***])
	with_alsa=yes
fi

AC_CANONICAL_HOST
PKG_PROG_PKG_CONFIG

# Checks for ALSA support
have_alsa=no
if test "$with_alsa" = "yes"; then
	PKG_CHECK_MODULES([ALSA], alsa, [
			have_alsa=yes
			AC_SUBST(ALSA_CFLAGS)
			AC_SUBST(ALSA_LIBS)
		])
fi

# Checks for JACK support
have_jack=no
if test "$with_jack" = "yes"; then
	PKG_CHECK_MODULES([JACK], jack, [
			have_jack=yes
			AC_SUBST(JACK_CFLAGS)
			AC_SUBST(JACK_LIBS)
		])
fi

# Checks for ASIO support
have_asio=no
if test "$with_asio" = "yes"; then
	AC_MSG_ERROR([*** ASIO support is not implemented yet. ***])
fi

# Checks for programs
AC_PROG_CC
AC_PROG_RANLIB

AC_C_BIGENDIAN

# Doxygen setup
AC_CHECK_PROGS([DOXYGEN], [doxygen])
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([doc/Makefile doc/Doxyfile])])

# Checks for header files
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADER_STDBOOL

AM_CONDITIONAL([OS_LINUX], [test "$os_linux" = "yes"])
AM_CONDITIONAL([OS_WINDOWS], [test "$os_windows" = "yes"])
AM_CONDITIONAL([USE_ALSA], [test "$have_alsa" = "yes"])
AM_CONDITIONAL([USE_JACK], [test "$have_jack" = "yes"])
AM_CONDITIONAL([USE_ASIO], [test "$have_asio" = "yes"])

AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
