
all: mangler

mangler.o: manglerui.h mangler.cpp mangler.h
	g++ -Wall -I. -I../icons -I../libventrilo3 -g -c -o mangler.o mangler.cpp `pkg-config --cflags gtkmm-2.4`
	
channeltree.o: channeltree.cpp mangler.h channeltree.h
	g++ -Wall -I. -I../libventrilo3 -g -c -o channeltree.o channeltree.cpp `pkg-config --cflags gtkmm-2.4`
	
manglernetwork.o: manglernetwork.cpp mangler.h manglernetwork.h
	g++ -Wall -I. -I../libventrilo3 -g -c -o manglernetwork.o manglernetwork.cpp `pkg-config --cflags gtkmm-2.4`

mangleraudio.o: mangleraudio.cpp mangler.h mangleraudio.h
	g++ -Wall -I. -I../libventrilo3 -g -c -o mangleraudio.o mangleraudio.cpp `pkg-config --cflags gtkmm-2.4` `pkg-config --cflags libpulse-simple`

manglersettings.o: manglersettings.cpp mangler.h manglersettings.h
	g++ -Wall -I. -I../libventrilo3 -g -c -o manglersettings.o manglersettings.cpp `pkg-config --cflags gtkmm-2.4`

manglerui.h: mangler.ui
	echo -n "char ManglerUI[] = " > manglerui.h
	cat mangler.ui  | sed 's/"/\\"/g' | sed 's/^/"/' | sed 's/$$/\\n"/' >> manglerui.h
	echo ";" >> manglerui.h

mangler: mangler.o channeltree.o manglernetwork.o mangleraudio.o manglersettings.o mangler.h ../libventrilo3/libventrilo.a
	g++ -Wall -L../libventrilo3/ -g -o mangler mangler.o channeltree.o manglernetwork.o mangleraudio.o manglersettings.o -export-dynamic -lventrilo3 `pkg-config --libs gtkmm-2.4` `pkg-config --libs gthread-2.0` `pkg-config --libs libpulse-simple` -lgsm -lspeex

../libventrilo3/libventrilo.a:
	cd ../libventrilo3/ && make

clean:
	rm -f tags
	rm -f *.o
	rm -f mangler
	rm -f manglerui.h
	rm -f mangler-icons.h
