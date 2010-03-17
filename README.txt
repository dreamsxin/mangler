To run this Android application, you will need to have libventrilo built with the Android NDK.
Below are instructions on how to do so with Windows & Cygwin, this process should be VERY similar
on GNU/Linux or Mac machines.

- Install subversion and checkout the mangler repository:
$ cd C:\
$ svn co http://svn.mangler.org/mangler/ mangler
You can use a seperate directory, but for this guide I will assume it is checked out in C:\mangler.

- Install cygwin and make sure you install the 'gcc', 'make', 'wget' and 'unzip' packages.

- Run cygwin and download the android-ndk and unzip it:
$ wget http://dl.google.com/android/ndk/android-ndk-r3-windows.zip
$ unzip android-ndk-r3-windows.zip

- Go into the NDK folder and execute host setup script:
$ cd android-ndk-r3
$ ./build/host-setup.sh

- Symlink the android branch into our NDK's app folder:
$ ln -s /cygdrive/c/mangler/branches/android/ ~/android-ndk-r3/apps/mangler

- Now, from the NDK's root directory, build the native library:
$ make APP=mangler LIBPATH=/cygdrive/c/mangler/trunk/libventrilo3

Done! If you already imported the Java project into eclipse make sure to refresh the project.
If you don't see libs/armeabi/ventrilo_interface.so, the application will not run!
