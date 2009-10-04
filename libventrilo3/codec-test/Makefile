
all:
	gcc -o pacat -g pacat.c  `pkg-config --libs libpulse-simple`
	gcc -o parec -g parec.c  `pkg-config --libs libpulse-simple`
	gcc -o speexdec -g speexdec.c -lspeex
	gcc -o speexenc -g speexenc.c -lspeex

