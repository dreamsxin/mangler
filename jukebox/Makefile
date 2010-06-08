
all:
	cd ../libventrilo3 && make dev
	gcc -Wall -ggdb -o mangler-jukebox -I../libventrilo3/ -I../ -pthread jukebox.c -L. ../libventrilo3/libventrilo3.a \
		-lgsm \
		-lspeex \
		`if grep "HAVE_CELT 1" ../config.h >/dev/null; then pkg-config --cflags --libs celt; fi` \
		-lmpg123 \
		-lspeexdsp -lm

