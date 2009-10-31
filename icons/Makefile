
all: mangler-icons.h

mangler-icons.h: black_circle.png blue_circle.png cyan_circle.png green_circle.png \
		 grey_circle.png purple_circle.png red_circle.png yellow_circle.png \
		 logo1.png logo2.jpg logo3.png \
		 user_icon_noxmit_otherroom.png  user_icon_noxmit.png  user_icon_xmit_otherroom.png  user_icon_xmit.png \
		 tray_icon_blue.svg tray_icon_red.svg tray_icon_yellow.svg tray_icon_green.svg tray_icon_grey.svg tray_icon_purple.svg
	list=""
	for f in *.png *.jpg *.svg ; do \
		id=`echo $$f | perl -pe 's/^(.*)\..*$$/$$1/'`;  \
		list="$$list $$id $$f";  \
	done ; \
	gdk-pixbuf-csource --raw --build-list $$list > mangler-icons.h

clean:
	rm -f mangler-icons.h

distclean:
	rm -f mangler-icons.h

install: mangler-icons.h

uninstall:
