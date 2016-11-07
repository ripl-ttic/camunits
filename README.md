## Downloading and Installing CamUnits ##

### From Source ###

To install from source, replace _x.y.z_ with the latest version and:
{{{
wget http://camunits.googlecode.com/files/camunits-x.y.z.tar.gz
tar xzvf camunits-x.y.z.tar.gz
cd camunits-x.y.z
./configure
make
make install 
}}}

### GNU/Linux ###

After installation, you may need to run `ldconfig` as root to refresh the dynamic linker cache.

{{{
ldconfig
}}}

### Mac OS X ###

Camunits has been built and run successfully on Mac OS 10.5 (Leopard).  To build, you will need glib and gtk+ installed, which we suggest getting from [http://darwinports.com/ Darwinports].  After installing darwinports, run:
{{{
sudo port install glib2
sudo port install gtk2
}}}
to install the glib and gtk+.

IIDC camera support works if you have libdc1394 >= 2.0.0 installed.  After installing libdc1394, you may need to run:
{{{
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
}}}
in order for the Camunits ./configure script to detect libdc1394.

### Git ###

To stay on the bleeding edge of Camunits development, you'll want to work from our subversion repositories hosted on Google Code.

You will want at least the following:
 * libjpeg62 development files
 * OpenGL development files
 * GLib/GTK+ development files
 * autotools (autoconf, automake, libtool, m4)
 * autoconf macro archive
 * gtk-doc
 * docbook

 # Checkout the repository
{{{
$ svn checkout http://camunits.googlecode.com/svn/trunk/ camunits-read-only
}}}
 # setup the build scripts
{{{
$ cd camunits-read-only
$ gtkdocize --copy
$ autoreconf -i
$ ./configure
}}}
 # (Optionally) mess with the source code
 # Build
{{{
make
}}}
 # Install (requires superuser privileges)
{{{
make install
}}}




