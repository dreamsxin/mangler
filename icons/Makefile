
all: mangler-icons.h

mangler-icons.h: mangler_headset_blue.png mangler_headset_blue_red.png mangler_headset_green.png mangler_headset_green_red.png mangler_headset_grey.png mangler_headset_grey_red.png mangler_headset_red.png mangler_headset_red_red.png user_silent_muted.png user_silent.png user_xmit_elsewhere_muted.png user_xmit_elsewhere.png user_xmit_muted.png user_xmit.png
	list=""
	for f in ../icons/*.png ; do \
		id=`echo $$f | perl -pe 's/^\.\.\/icons\/(.*)\..*$$/$$1/'`;  \
		list="$$list $$id $$f";  \
	done ; \
	gdk-pixbuf-csource --raw --build-list $$list > mangler-icons.h

clean:
	rm mangler-icons.h
