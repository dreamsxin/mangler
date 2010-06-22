
all:
	cd ../libventrilo3 && make dev
	gcc -pthread -Wall -g -o mangler-jukebox -I../libventrilo3/ -I.. \
		`if pkg-config flac; then echo -n "-DHAVE_FLAC"; fi` \
		jukebox.c \
		../libventrilo3/libventrilo3.a \
		-lgsm \
		-lspeex \
		`if grep "HAVE_CELT 1" ../config.h >/dev/null; then pkg-config --cflags --libs celt; fi` \
		-lmpg123 \
		-lspeexdsp \
		`if pkg-config flac; then echo -n "-lFLAC"; fi` \
		-lm

