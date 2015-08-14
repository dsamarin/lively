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

AC_PROG_CC
AC_PROG_RANLIB

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

AC_CONFIG_FILES([Makefile src/Makefile src/linux/Makefile src/windows/Makefile])
AC_OUTPUT
