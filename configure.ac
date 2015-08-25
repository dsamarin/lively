# Initialization
AC_PREREQ([2.69])
AC_INIT(
	[Lively],
	[0.1],
	[https://github.com/dsamarin/lively/issues],
	[lively],
	[https://dsamar.in/lively/])
AC_SUBST([PACKAGE_BRIEF], ['Lively is an open-source application for live mixing and effects processing.'])
AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([src/main.c])

AC_CANONICAL_HOST

# Checks for programs
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries
PKG_CHECK_MODULES([ALSA], alsa,
	AC_CHECK_LIB([asound], [snd_pcm_open], [
			AC_SUBST(ALSA_CFLAGS)
			AC_SUBST(ALSA_LIBS)
		],
		AC_MSG_ERROR([alsa is not available on this system; install libasound2-dev])),
	AC_MSG_ERROR([pkg-config could not find alsa; install libasound2-dev]))

# Doxygen setup
AC_CHECK_PROGS([DOXYGEN], [doxygen])
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([doc/Makefile doc/Doxyfile])])

# Platform setup
PLATFORM=
PLATFORM_LIST="windows linux"
AC_MSG_CHECKING([platform])
case "$host" in
	*-*-mingw*|*-*-cygwin*)
		PLATFORM="windows"
		;;
	*)
		PLATFORM="linux"
		;;
esac
AC_MSG_RESULT([$PLATFORM])
AC_SUBST([PLATFORM])
AC_SUBST([PLATFORM_LIST])

# Checks for header files
AC_CHECK_HEADERS([stdlib.h alsa/asoundlib.h])
AC_CHECK_HEADER_STDBOOL

#
AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES([Makefile src/Makefile src/linux/Makefile src/windows/Makefile])
AC_OUTPUT
