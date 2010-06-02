These instructions are only valid from Android NDK Revision 4 and up.

To run Mangler for Android, you will need to have libventrilo built with the Android NDK.
Below are instructions on how to do so with Windows & Cygwin. However, this process should be very similar on GNU/Linux or Mac machines.

- Install subversion and checkout the mangler repository:
$ cd C:\
$ svn co http://svn.mangler.org/mangler/ mangler
You can use a seperate directory, but for this guide I will assume it is checked out in C:\mangler.

- Install cygwin and make sure you install the 'gcc', 'make', 'wget' and 'unzip' packages.

- Run cygwin and download the android-ndk and unzip it:
$ wget http://dl.google.com/android/ndk/android-ndk-r4-windows.zip
$ unzip android-ndk-r4-windows.zip

- Now, from the NDK's root directory, build the native library:
$ cd android-ndk-r4-windows
$ ./ndk-build -C /cygdrive/c/mangler/branches/android/project LIBPATH=/cygdrive/c/mangler/trunk/libventrilo3

You're done!

Note:
- If you already imported the Java project into eclipse make sure to refresh the project.
- If you don't see libs/armeabi/ventrilo_interface.so, you did not properly build the native library.
