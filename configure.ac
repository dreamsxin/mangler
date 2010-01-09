#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([mangler], [1.1.0], [eric@ypass.net])
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([enable])

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

AC_ARG_ENABLE(gsm,   [  --disable-gsm           disable GSM codec support], [disable_gsm=yes])
AC_ARG_ENABLE(speex, [  --disable-speex         disable Speex codec support], [disable_speex=yes])

if test "$enable_gsm" != "no" ; then
	AC_CHECK_LIB([gsm], [gsm_create], [], [
		echo "error: cannot find libgsm: Is it installed? Also make sure you have the"
		echo "       libgsm development packages installed"
		exit -1
		])
	AC_CHECK_HEADER([gsm.h], [AC_DEFINE(HAVE_GSM_H, 1, [location of gsm.h])], [
		AC_CHECK_HEADER([gsm/gsm.h], AC_DEFINE(HAVE_GSM_GSM_H, 1, [location of gsm.h]),[
			echo "error: found gsm libraries, but headers are missing or corrupt. are you"
			echo "       missing a libgsm development package?"
			exit -1
			])
		])
	AC_DEFINE(HAVE_GSM, 1, [have gsm])
else
	AC_DEFINE(HAVE_GSM, 0, [have gsm])
fi


# Checks for header files.
if test "$enable_speex" != "no" ; then
	PKG_CHECK_MODULES(speex, speex >= 1.2)
	AC_SUBST(speex_CFLAGS)
	AC_SUBST(speex_LIBS)
	AC_DEFINE(HAVE_SPEEX, 1, [have speex])
else
	AC_DEFINE(HAVE_SPEEX, 0, [have speex])
fi

PKG_CHECK_MODULES(gtk, gtk+-2.0 >= 2.16.0)

PKG_CHECK_MODULES(gtkmm, gtkmm-2.4 >= 1.3.3)
AC_SUBST(gtkmm_CFLAGS)
AC_SUBST(gtkmm_LIBS)

AC_ARG_ENABLE(pulseaudio, [  --disable-pulseaudio    disable PulseAudio support], [disable_pulse=yes])
AC_ARG_ENABLE(alsa,       [  --enable-alsa           enable ALSA support], [enable_alsa=yes])

if test -z "$enable_pulseaudio" -a -z "$enable_alsa" ; then
	pulseaudio=yes
	alsa=no
elif  test "$enable_pulseaudio" = "yes" -a "$enable_alsa" = "yes" ; then
	echo "error: you cannot enable support for both ALSA and PulseAudio"
	exit
elif  test "$enable_pulseaudio" = "no" -a "$enable_alsa" = "no" ; then
	echo "**************************************************************************"
	echo "*** WARNING: You have disabled support for both PulseAudio and ALSA.   ***"
	echo "***          You can compile and use Mangler without an audio          ***"
	echo "***          subsystem, but you will not have any sound, which more    ***"
	echo "***          more or less makes the program useless.                   ***"
	echo "**************************************************************************"
	pulseaudio=no
	alsa=no
elif  test "$enable_pulseaudio" = "no" -a "$enable_alsa" != "yes" ; then
	echo "**************************************************************************"
	echo "*** WARNING: You have disabled support for both PulseAudio, but you    ***"
	echo "***          did not enable ALSA support.  Enabling ALSA by default.   ***"
	echo "**************************************************************************"
	pulseaudio=no
	alsa=yes
	exit
elif  test "$enable_pulseaudio" = "yes" ; then
	echo "Enabling PulseAudio support and disabling ALSA support"
	pulseaudio=yes
	alsa=no
elif  test "$enable_alsa" = "yes" ; then
	echo "Enabling ALSA support and disabling PulseAudio support"
	pulseaudio=no
	alsa=yes
fi

AS_IF(
	[test "$pulseaudio" = "yes"],
	[
		PKG_CHECK_MODULES(pulse, libpulse-simple >= 0.9.14, [AC_DEFINE(HAVE_PULSE, 1, [have pulse audio])], [
			echo "**************************************************************************"
			echo "*** ERROR: pulse audio development libraries or headers were not found ***"
			echo "*** on your system.  You can disable PulseAudio support with           ***"
			echo "*** ./configure --without-pulseaudio                                   ***"
			echo "**************************************************************************"
			exit 1
		])
		AC_SUBST(pulse_CFLAGS)
		AC_SUBST(pulse_LIBS)
	],[]
)
AS_IF(
	[test "$alsa" = "yes"],
	[
		PKG_CHECK_MODULES(alsa, alsa >= 1.0.0, [AC_DEFINE(HAVE_ALSA, 1, [have alsa])], [
			echo "**************************************************************************"
			echo "*** ERROR: ALSA audio development libraries or headers were not found  ***"
			echo "*** on your system.  You can disable ALSA support with                 ***"
			echo "*** ./configure --without-alsa                                         ***"
			echo "**************************************************************************"
			exit 1
		])
		AC_SUBST(alsa_CFLAGS)
		AC_SUBST(alsa_LIBS)
	],[]
)

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
AC_CHECK_FUNCS([dup2 gettimeofday inet_ntoa memset select socket strchr strdup strerror gethostbyname_r])

AC_CONFIG_FILES([src/Makefile
                 Makefile
                 libventrilo3/Makefile])
AC_OUTPUT
