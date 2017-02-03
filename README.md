# Downloading and Installing CamUnits #



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
    $ 
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


## GNU/Linux ##

After installation, you may need to run `ldconfig` as root to refresh the dynamic linker cache.

~~~~
ldconfig
~~~~

## Mac OS X ##

Camunits has been built and run successfully on Mac OS 10.5 (Leopard).  To build, you will need glib and gtk+ installed, which we suggest getting from [MacPorts](https://www.macports.org).  After installing MacPorts, run:
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



* * *



#Frequently Asked Questions#

** How does Camunits compare with OpenCV? **

Camunits differs from OpenCV in two major aspects.  The first is that Camunits is a framework for developing image acquisition and processing algorithms and applications, whereas OpenCV provides a collection of already developed algorithms.  Camunits is intended to simplify both the process of exposing and experimenting with parameters for machine vision algorithms, and the process of visualizing and debugging the result of such algorithms.

Secondly, within this framework, Camunits places great importance on a simple way to acquire images and control image acquisition and processing elements.  In contrast, while OpenCV does provide some image acquisition support, this support is fairly rudimentary and does not expose much of the functionality that most cameras provide.

In short, Camunits is designed to streamline the process of developing both image processing __algorithms__ and __applications__, and provides some core functionality for doing so;  OpenCV is primarily a collection of pre-developed algorithms available for use.  It will often be the case that an application developed using Camunits will have great use for a specific algorithm provided by OpenCV.



** Why is Camunits written in C and not C++? **

Simplicity and portability.

 * C++ is a complex language with many features that we do not need in Camunits.  We do not need templates, smart pointers, operator and function overloading, multiple inheritance, etc.  Camunits uses a straightforward single-inheritance class hierarchy, which is not too difficult to achieve in C, especially when using GLib.  At the end of the day, we simply do not need C++.

 * The complexities of C++ make it less portable than C.  Specifically, namespace mangling often makes it difficult to link against a C++ library compiled with a different compiler than the one you'd like to use.

 * There are many C programmers out there who don't like C++.  Writing Camunits in C allows both C and C++ programmers to use Camunits.

** Can I do something like ** `{{{while(1) { img = grab_image(); } }}}`?

In short, no.  Camunits always delivers images via function callbacks.  To receive an image, your program must connect to the `frame-ready` signal of a CamUnit or a CamUnitChain.

More specifically, Camunits is designed for [event-driven programming](http://en.wikipedia.org/wiki/Event_driven_programming), which means that it works best when used in a proper event loop. This allows Camunits to interact nicely with graphical applications and other event-driven libraries.

We realize that in many machine vision applications the sole purpose of the application is to grab images and process them, in which case a proper event loop is not completely necessary.  In these situations, we suggest using a barebones GLib event loop, as shown in the [tutorial](http://camunits.googlecode.com/svn/www/tutorial/ch-acquiring.html).  We suspect that the overhead of using an event loop will be trivial compared to the complexity of the rest of your application.