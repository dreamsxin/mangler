
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = lv3_test

lib_LTLIBRARIES = libventrilo3.la

libventrilo3_la_SOURCES = libventrilo3.c  libventrilo3_message.c  ventrilo3_handshake.c

lv3_test_SOURCES = lv3_test.c

INCLUDES=$(gtkmm_CFLAGS) $(gthread_CFLAGS) $(pulse_CFLAGS) $(speex_CFLAGS)

lv3_test_LDADD = libventrilo3.la $(gtkmm_LIBS) $(gthread_LIBS) $(pulse_LIBS) $(speex_LIBS)


dev:
	gcc -Wall -g -o lv3_test -I../ -pthread lv3_test.c libventrilo3.c libventrilo3_message.c  ventrilo3_handshake.c -lpulse-simple -lpulse -lspeex -lgsm

