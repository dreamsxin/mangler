
ACLOCAL_AMFLAGS = -I m4

icondir = $(datadir)/pixmaps
desktopdir = $(datadir)/applications

manglerui:
	echo -n "char ManglerUI[] = " > manglerui.h
	cat mangler.ui  | sed 's/"/\\"/g' | sed 's/^/"/' | sed 's/$$/\\n"/' >> manglerui.h
	echo ";" >> manglerui.h


bin_PROGRAMS = mangler
icon_DATA = ../icons/mangler_logo.svg 
desktop_DATA = ../mangler.desktop

mangler_SOURCES = manglerui.h mangler.cpp channeltree.cpp  mangleraudio.cpp  manglernetwork.cpp  manglerserverlist.cpp  manglersettings.cpp manglerconfig.cpp manglerchat.cpp manglerprivchat.cpp manglercharset.cpp manglerintegration.cpp

mangler_LDADD = @top_srcdir@/libventrilo3/libventrilo3.la $(gtkmm_LIBS) $(gthread_LIBS) $(pulse_LIBS) $(alsa_LIBS) $(speex_LIBS) ${dbus_LIBS}

INCLUDES = -I@top_srcdir@/libventrilo3/ -I@top_srcdir@/icons/ -I@top_srcdir@/sounds/ $(gtkmm_CFLAGS) $(gthread_CFLAGS) $(pulse_CFLAGS) $(alsa_CFLAGS) $(speex_CFLAGS) ${dbus_CFLAGS}

dev:
	cd ../libventrilo3 && make dev
	gcc -g -Wall -o mangler -I../ -I../libventrilo3/ -I../icons/ -I ../sounds/ -pthread \
		channeltree.cpp  mangleraudio.cpp  manglerconfig.cpp  mangler.cpp \
		manglernetwork.cpp  manglerserverlist.cpp  manglersettings.cpp \
		manglerchat.cpp manglercharset.cpp manglerintegration.cpp \
		../libventrilo3/libventrilo3.a \
		-lpulse-simple -lpulse -lspeex -lgsm \
		`pkg-config --cflags --libs gthread-2.0` \
		`pkg-config --cflags --libs gtkmm-2.4`
