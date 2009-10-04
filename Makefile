
all:
	cd icons && make
	cd libventrilo3 && make
	cd src && make
	cp src/mangler .

clean:
	cd icons && make clean clean
	cd libventrilo3 && make clean
	cd src && make clean
