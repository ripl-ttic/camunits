## Downloading and Installing CamUnits ##

To stay on the bleeding edge of Camunits development, you'll want to work from our Git repositories hosted on Google Code.

You will want at least the following:

 * libjpeg62 development files
 * OpenGL development files
 * GLib/GTK+ development files
 * autotools (autoconf, automake, libtool, m4)
 * autoconf macro archive
 * gtk-doc
 * docbook

1. Checkout the repository
~~~~
    $ svn checkout http://camunits.googlecode.com/svn/trunk/ camunits-read-only
~~~~
2. Setup the build scripts
~~~~
    $ cd camunits-read-only
    $ gtkdocize --copy
    $ autoreconf -i
    $ ./configure
~~~~
3. (Optionally) mess with the source code

4. Build
~~~~
    make
~~~~
5. Install (requires superuser privileges)
~~~~
    make install
~~~~


### GNU/Linux ###

After installation, you may need to run `ldconfig` as root to refresh the dynamic linker cache.

~~~~
ldconfig
~~~~

### Mac OS X ###

Camunits has been built and run successfully on Mac OS 10.5 (Leopard).  To build, you will need glib and gtk+ installed, which we suggest getting from [http://darwinports.com/ Darwinports].  After installing darwinports, run:
~~~~
sudo port install glib2
sudo port install gtk2
~~~~
to install the glib and gtk+.

IIDC camera support works if you have libdc1394 >= 2.0.0 installed.  After installing libdc1394, you may need to run:
~~~~
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
~~~~
in order for the Camunits ./configure script to detect libdc1394.