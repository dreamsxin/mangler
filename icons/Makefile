
all: mangler-icons.h

mangler-icons.h: black_circle.png blue_circle.png cyan_circle.png green_circle.png \
		 grey_circle.png purple_circle.png red_circle.png yellow_circle.png \
		 mangler_logo.svg \
		 user_icon_red.png  user_icon_yellow.png  user_icon_green.png  user_icon_orange.png \
		 tray_icon_blue.svg tray_icon_red.svg tray_icon_yellow.svg tray_icon_green.svg tray_icon_grey.svg tray_icon_purple.svg
	list=""
	for f in *.png *.svg ; do \
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
