
all:
	cd ../libventrilo3 && make dev
	gcc -Wall -g -o mangler-jukebox -I../libventrilo3/ -I../ -pthread jukebox.c -L. ../libventrilo3/libventrilo3.a -lgsm -lspeex -lmpg123 -lspeexdsp

