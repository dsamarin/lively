AC_PREREQ([2.69])
AC_INIT(
	[Lively],
	[0.1],
	[https://github.com/dsamarin/lively/issues],
	[lively],
	[https://dsamar.in/lively/])

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

AC_CANONICAL_HOST

AC_PROG_CC

AC_MSG_CHECKING([for some Windows platform])
case "$host" in
	*-*-mingw*|*-*-cygwin*)
		platform_windows=yes
		platform_linux=no
		;;
	*)
		platform_windows=no
		platform_linux=yes
		;;
esac

AM_CONDITIONAL(PLATFORM_LINUX, test "$platform_linux" = "yes")
AM_CONDITIONAL(PLATFORM_WINDOWS, test "$platform_windows" = "yes")

AC_CONFIG_FILES([Makefile, src/Makefile, src/linux/Makefile, src/windows/Makefile])
AC_OUTPUT
