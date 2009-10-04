
all: mangler

mangler.o: manglerui.h mangler.cpp
	g++ -Wall -I. -I../icons -I../libventrilo3 -g -c -o mangler.o mangler.cpp `pkg-config --cflags gtkmm-2.4`
	
channeltree.o: channeltree.cpp
	g++ -Wall -I. -I../libventrilo3 -g -c -o channeltree.o channeltree.cpp `pkg-config --cflags gtkmm-2.4`
	
manglernetwork.o: manglernetwork.cpp
	g++ -Wall -I. -I../libventrilo3 -g -c -o manglernetwork.o manglernetwork.cpp `pkg-config --cflags gtkmm-2.4`

manglerui.h: mangler.ui
	echo -n "char ManglerUI[] = " > manglerui.h
	cat mangler.ui  | sed 's/"/\\"/g' | sed 's/^/"/' | sed 's/$$/\\n"/' >> manglerui.h
	echo ";" >> manglerui.h

mangler: mangler.o channeltree.o manglernetwork.o
	g++ -Wall -L../libventrilo3/ -g -o mangler mangler.o channeltree.o manglernetwork.o -export-dynamic -lventrilo3 `pkg-config --libs gtkmm-2.4` `pkg-config --libs gthread-2.0` -lspeex

clean:
	rm -f tags
	rm -f *.o
	rm -f mangler
	rm -f manglerui.h
	rm -f mangler-icons.h
