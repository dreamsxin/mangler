
ACLOCAL_AMFLAGS = -I m4

manglerui:
	echo -n "char ManglerUI[] = " > manglerui.h
	cat mangler.ui  | sed 's/"/\\"/g' | sed 's/^/"/' | sed 's/$$/\\n"/' >> manglerui.h
	echo ";" >> manglerui.h


bin_PROGRAMS = mangler

mangler_SOURCES = manglerui.h mangler.cpp channeltree.cpp  mangleraudio.cpp  manglernetwork.cpp  manglerserverlist.cpp  manglersettings.cpp manglerconfig.cpp

mangler_LDADD = @top_srcdir@/libventrilo3/libventrilo3.la $(gtkmm_LIBS) $(gthread_LIBS) $(pulse_LIBS) $(speex_LIBS)

INCLUDES = -I@top_srcdir@/libventrilo3/ -I@top_srcdir@/icons/ -I@top_srcdir@/sounds/ $(gtkmm_CFLAGS) $(gthread_CFLAGS) $(pulse_CFLAGS) $(speex_CFLAGS)

dev:
	cd ../libventrilo3 && make dev
	gcc -g -Wall -o mangler -I../ -I../libventrilo3/ -I../icons/ -I ../sounds/ -pthread \
		channeltree.cpp  mangleraudio.cpp  manglerconfig.cpp  mangler.cpp \
		manglernetwork.cpp  manglerserverlist.cpp  manglersettings.cpp \
		../libventrilo3/libventrilo3.a \
		-lpulse-simple -lpulse -lspeex -lgsm \
		`pkg-config --cflags --libs gthread-2.0` \
		`pkg-config --cflags --libs gtkmm-2.4`
