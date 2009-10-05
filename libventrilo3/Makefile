
all: lv3_test

lv3_test: libventrilo3.a lv3_test.c ventrilo3.h
	gcc -pthread -Wall -g -o lv3_test lv3_test.c -L. -lventrilo3 `pkg-config --libs libpulse-simple` -lspeex

libventrilo3.o: libventrilo3.c ventrilo3.h libventrilo3.h
	gcc -pthread -Wall -g -c -o libventrilo3.o libventrilo3.c

ventrilo3_handshake.o: ventrilo3_handshake.c ventrilo_algo.h ventrilo3.h
	gcc -pthread -Wall -g -c -o ventrilo3_handshake.o ventrilo3_handshake.c

libventrilo3_message.o: libventrilo3_message.c libventrilo3_message.h ventrilo3.h
	gcc -pthread -Wall -g -c -o libventrilo3_message.o libventrilo3_message.c

libventrilo3.a: libventrilo3.o ventrilo3_handshake.o libventrilo3_message.o ventrilo3.h
	ar rvs libventrilo3.a libventrilo3.o ventrilo3_handshake.o libventrilo3_message.o

all32:
	gcc -m32 -Wall -g -c -o libventrilo3.o libventrilo3.c
	gcc -m32 -Wall -g -c -o ventrilo3_handshake.o ventrilo3_handshake.c
	gcc -m32 -Wall -g -c -o libventrilo3_message.o libventrilo3_message.c
	ar rvs libventrilo3.a libventrilo3.o ventrilo3_handshake.o libventrilo3_message.o
	gcc -m32 -Wall -g -o lv3_test lv3_test.c -L. -lventrilo3

all-dmalloc:
	gcc -Wall -g -c -DDMALLOC -DDMALLOC_FUNC_CHECK -o libventrilo3.o libventrilo3.c
	gcc -Wall -g -c -DDMALLOC -DDMALLOC_FUNC_CHECK -o ventrilo3_handshake.o ventrilo3_handshake.c
	gcc -Wall -g -c -DDMALLOC -DDMALLOC_FUNC_CHECK -o libventrilo3_message.o libventrilo3_message.c
	ar rvs libventrilo3.a libventrilo3.o ventrilo3_handshake.o libventrilo3_message.o
	gcc -Wall -g -DDMALLOC -DDMALLOC_FUNC_CHECK -o lv3_test lv3_test.c -L. -lventrilo3  -ldmalloc

all-dmalloc-32:
	gcc -m32 -Wall -g -c -DDMALLOC -DDMALLOC_FUNC_CHECK -o libventrilo3.o libventrilo3.c
	gcc -m32 -Wall -g -c -DDMALLOC -DDMALLOC_FUNC_CHECK -o ventrilo3_handshake.o ventrilo3_handshake.c
	gcc -m32 -Wall -g -c -DDMALLOC -DDMALLOC_FUNC_CHECK -o libventrilo3_message.o libventrilo3_message.c
	ar rvs libventrilo3.a libventrilo3.o ventrilo3_handshake.o libventrilo3_message.o
	gcc -m32 -Wall -g -DDMALLOC -DDMALLOC_FUNC_CHECK -o lv3_test lv3_test.c -L/usr/lib32 -Wl,-R/usr/lib32 -L. -lventrilo3  -ldmalloc

clean:
	rm -f tags
	rm -f *.o
	rm -f *.a
	rm -f lv3_test
