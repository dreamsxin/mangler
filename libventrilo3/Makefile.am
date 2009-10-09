
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = lv3_test

lib_LTLIBRARIES = libventrilo3.la

libventrilo3_la_SOURCES = libventrilo3.c  libventrilo3_message.c  ventrilo3_handshake.c

lv3_test_SOURCES = lv3_test.c

lv3_test_LDADD = libventrilo3.la

