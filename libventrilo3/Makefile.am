
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libventrilo3.la

libventrilo3_la_SOURCES = libventrilo3.c  libventrilo3_message.c  ventrilo3_handshake.c

libventrilo3_la_LIBADD = $(pulse_LIBS) $(speex_LIBS) $(gsm_LIBS)

lv3_test_SOURCES = lv3_test.c

INCLUDES=$(pulse_CFLAGS) $(speex_CFLAGS)

lv3_test_LDADD = libventrilo3.la

noinst_PROGRAMS = lv3_test


dev:
	gcc -pthread -Wall -g -c -I../ -o libventrilo3.o libventrilo3.c
	gcc -pthread -Wall -g -c -I../ -o ventrilo3_handshake.o ventrilo3_handshake.c
	gcc -pthread -Wall -g -c -I../ -o libventrilo3_message.o libventrilo3_message.c
	ar rvs libventrilo3.a libventrilo3.o ventrilo3_handshake.o libventrilo3_message.o
	gcc -Wall -g -o lv3_test -I../ -pthread lv3_test.c -L. libventrilo3.a -lpulse-simple -lpulse -lspeex -lgsm



