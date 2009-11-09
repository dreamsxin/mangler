#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([mangler], [1.0], [eric@ypass.net])
AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
AC_PROG_LIBTOOL

AX_PTHREAD([
LIBS="$PTHREAD_LIBS $LIBS"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
CC="$PTHREAD_CC"
], [echo "error: cannot determine how to build threaded applications"; exit 1])
AM_PATH_GLIB_2_0([], [], [], [gthread])
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

AC_CHECK_LIB([m], [tan], [], [
	echo "error: cannot find a working math library"
	exit -1
	])
AC_CHECK_HEADER([mpd/client.h],
	[AC_CHECK_LIB([mpdclient], [mpd_run_status])]
)
AC_CHECK_LIB([gsm], [gsm_create], [], [
	echo "error: cannot find libgsm: Is it installed? Also make sure you have the"
	echo "       libgsm development packages installed"
	exit -1
	])
AC_CHECK_HEADER([gsm.h], [AC_DEFINE(HAVE_GSM_H, 1, [location of gsm.h])], [
	AC_CHECK_HEADER([gsm/gsm.h], [], [
		echo "error: found gsm libraries, but headers are missing or corrupt. are you"
		echo "       missing a libgsm development package?"
		exit -1
		])
	])


# Checks for header files.
PKG_CHECK_MODULES(speex, speex >= 1.2)
AC_SUBST(speex_CFLAGS)
AC_SUBST(speex_LIBS)

PKG_CHECK_MODULES(gtk, gtk+-2.0 >= 2.16.0)

PKG_CHECK_MODULES(gtkmm, gtkmm-2.4 >= 1.3.3)
AC_SUBST(gtkmm_CFLAGS)
AC_SUBST(gtkmm_LIBS)

PKG_CHECK_MODULES(pulse, libpulse-simple >= 0.9.14, [AC_DEFINE(HAVE_PULSE, 1, [have pulse audio])], [
	echo "**************************************************************************"
	echo "*** WARNING: compiling without pulse.  you will not have sound support ***"
	echo "*** PulseAudio 0.9.14 or higher is required for audio support          ***"
	echo "**************************************************************************"
])
AC_SUBST(pulse_CFLAGS)
AC_SUBST(pulse_LIBS)

PKG_CHECK_MODULES(gthread, gthread-2.0 >= 2.20.1)
AC_SUBST(gthread_CFLAGS)
AC_SUBST(gthread_LIBS)

PKG_CHECK_MODULES(gtkmm, gtkmm-2.4 >= 1.3.3)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([dup2 gettimeofday inet_ntoa memset select socket strchr strdup strerror])

AC_CONFIG_FILES([src/Makefile
                 Makefile
                 libventrilo3/Makefile])
AC_OUTPUT
