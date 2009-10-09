
ACLOCAL_AMFLAGS = -I m4

manglerui.h: mangler.ui
	echo -n "char ManglerUI[] = " > manglerui.h
	cat mangler.ui  | sed 's/"/\\"/g' | sed 's/^/"/' | sed 's/$$/\\n"/' >> manglerui.h
	echo ";" >> manglerui.h

bin_PROGRAMS = mangler

mangler_SOURCES = mangler.cpp channeltree.cpp  mangleraudio.cpp  manglernetwork.cpp  manglerserverlist.cpp  manglersettings.cpp manglerui.h

mangler_LDADD = @top_srcdir@/libventrilo3/libventrilo3.la $(GTKMM_LIBS) $(GTHREAD_LIBS) $(PULSEAUDIO_LIBS)

INCLUDES = -I@top_srcdir@/libventrilo3/ -I@top_srcdir@/icons/ $(GTKMM_CFLAGS) $(GTHREAD_CFLAGS) $(PULSEAUDIO_CFLAGS)


