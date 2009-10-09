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

ACX_PTHREAD()
AM_PATH_GLIB_2_0([], [], [], [gthread])

# Checks for libraries.
AC_CHECK_LIB([speex], [speex_encoder_init], [], [
	echo "error: cannot find libspeex: Is it installed? Also make sure you have the"
	echo "       libspeex development packages installed"
	exit -1
	])
AC_CHECK_LIB([gsm], [gsm_create], [], [
	echo "error: cannot find libgsm: Is it installed? Also make sure you have the"
	echo "       libgsm development packages installed"
	exit -1
	])


# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])
AC_CHECK_HEADER([speex/speex.h], [], [
	echo "error: found speex libraries, but headers are missing or corrupt. are you"
	echo "       missing a libspeex development package?"
	exit -1
	])
AC_CHECK_HEADER([gsm.h], [], [
	echo "error: found gsm libraries, but headers are missing or corrupt. are you"
	echo "       missing a libgsm development package?"
	exit -1
	])
AC_CHECK_HEADERS([pulse/pulse-simple.h pulse/error.h pulse/gccmacro.h])
AC_CHECK_HEADERS([gtkmm.h])

GTKMM_CFLAGS=`pkg-config --cflags gtkmm-2.4`
GTKMM_LIBS=`pkg-config --libs gtkmm-2.4`
AC_SUBST(GTKMM_CFLAGS)
AC_SUBST(GTKMM_LIBS)
LIBS="$LIBS $GTKMM_LIBS"
CFLAGS="$LIBS $GTKMM_CFLAGS"

GTHREAD_CFLAGS=`pkg-config --cflags gthread-2.0`
GTHREAD_LIBS=`pkg-config --libs gthread-2.0`
AC_SUBST(GTHREADMM_CFLAGS)
AC_SUBST(GTHREADMM_LIBS)
LIBS="$LIBS $GTHREAD_LIBS"
CFLAGS="$LIBS $GTHREAD_CFLAGS"

PULSEAUDIO_CFLAGS=`pkg-config --cflags libpulse-simple`
PULSEAUDIO_LIBS=`pkg-config --libs libpulse-simple`
AC_SUBST(PULSEAUDIOMM_CFLAGS)
AC_SUBST(PULSEAUDIOMM_LIBS)
LIBS="$LIBS $PULSEAUDIO_LIBS"
CFLAGS="$LIBS $PULSEAUDIO_CFLAGS"

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
