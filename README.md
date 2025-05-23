![EFL](/data/readme/efl.png)
# EFL

-----

*Please report bugs/issues at*
[git.enlightenment.org](https://git.enlightenment.org/enlightenment/efl/issues)

-----

EFL is a collection of libraries for handling many common tasks a
developer may have such as data structures, communication, rendering,
widgets and more.

There are many components inside EFL. They also build various things
like shared libraries, loadable plug-in modules and also binary
executables. Different elements will end up being licensed differently
too. Below is a rundown of the components, what they do and their
licensing.

All library source is in the src/lib/ directory. All binaries are in
src/bin/. All loadable modules are in src/modules/. All data files are
in the data/ directory. Licensing details are listed in COPYING and
the licenses/ directory. The pc/ directory holds all the Package
Config files installed to help developers build against EFL.

For more documentation please see:

[www.enlightenment.org/doc](https://www.enlightenment.org/docs)

## Platforms

EFL is primarily developed on Linux (GNU/Linux) and should work on
most distributions as long as dependencies are provided. It has been
compiled and run also on Windows (using MSYS2 + mingw-w64) - please see
[windows docs](https://www.enlightenment.org/docs/distros/windows-start.md),
Mac OS X, FreeBSD and NetBSD.

## Components

**Ecore:**

*BSD 2-Clause license*

This is the core main-loop, system events and execution layer. This
handles running the main loop, integrating with external data and
timing sources (the system clock, file descriptors, system signals),
and producing an event queue, walking that queue and dispatching
events to appropriate callbacks.

**Ecore Audio:**

*BSD 2-Clause license*

This library provides an API for audio playback and recording. It uses
pulse audio underneath to handle mixing and policies. The API for this
should not be considered stable right now because it relies on EO and
EO is not considered finalized yet.

**Ecore Cocoa:**

*BSD 2-Clause license*

This provides wrappers/abstractions around Max OS-X Cocoa APIs to help
Mac porting.

**Ecore Con:**

*BSD 2-Clause license*

This provides a completely event-based TCP/UDP and Unix domain socket
API that integrates with the main-loop. This means no blocking to send
or receive data, supporting "infinite send buffers" with storage and
spooling being done by Ecore Con. It also supports SSL encryption
transparently turned on or not on the same connection, certificate
verification, CURL wrapping for HTTP connection usage (GETs, POSTs
etc.), asynchronous DNS lookups and provides the ability to also be a
server, not just a client, with the same event-based API.

**Ecore Evas:**

*BSD 2-Clause license*

This acts as glue between the display target (X11, Wayland,
Frame buffer, Cocoa on OSX, Win32 etc.) and Evas. It creates/provides a
target for Evas to render to (a Window or Surface etc.) and feeds
input events (Keyboard, Mouse, Multi-touch) into Evas, which then
selects the target object and calls the callbacks. It also provides
wrappers/glue for manipulating the Window/Surface.

**Ecore Fb:**

*BSD 2-Clause license*

This provides virtual terminal allocation, access and handling,
frame buffer information, raw input handling for keyboard, mouse and
touch (via tslib).

**Ecore File:**

*BSD 2-Clause license*

This provides file access convenience APIs for doing simple file
operations like renames, copies, listing directories and more. It also
supports file change monitoring and URL downloads.

**Ecore IMF:**

*BSD 2-Clause license*

This is an input method abstraction framework to allow EFL to talk to
things like SCIM, IBus, Wayland and XIM. This allows for complex text
entry in languages such as Chinese, Japanese and Korean.

**Ecore IMF Evas:**

*BSD 2-Clause license*

This library glues Input method support from Ecore IMF and Evas
together.

**Ecore Input:**

*BSD 2-Clause license*

This acts as a generic input layer where multiple display systems can
post events in the same format to the event queue.

**Ecore Input Evas:**

*BSD 2-Clause license*

This Routes events from Ecore Input into a given Evas canvas which
will then route the event further to the destination object and
callbacks.

**Ecore IPC:**

*BSD 2-Clause license*

This acts as a layer on top of Ecore Con which handles entire IPC
message packets, dealing with header delta compression and
portability, as well as ensuring an entire message is received in one
go as a single IPC message, regardless of payload data size. The
entire API is event based almost exactly like Ecore Con and thus it
supports all the transport layers Ecore Con supports.

**Ecore SDL:**

*BSD 2-Clause license*

This acts as a wrapper/glue around SDL to handle SDL Windows as well
as input events from SDL and tie them to the Ecore main-loop and event
queue.

**Ecore Wayland:**

*BSD 2-Clause license*

This is a glue/wrapper library to interface EFL to Wayland libraries
to tie them into the Ecore main-loop and event queue.

**Ecore Win32:**

*BSD 2-Clause license*

This acts as glue/wrapper around Windows Win32 APIs to tie them into
the Ecore main-loop and event queue.

**Ecore X:**

*BSD 2-Clause license*

This is a library to wrap/deal with Xlib make dealing with X11 less painful
and less footwork as well as being glue to tie these into the Ecore main-loop
and event queue.

**Edje:**

*BSD 2-Clause license* (except the epp binary which is GPLv2)

This is a graphics event, input, theme, animation and theme
abstraction library used to place many UI/UX elements into data files
(called edj files) that define how to react to incoming
events/signals, and define animation time lines, states, relative
scalable layout and much much more. It comes with a compiler that
compiles source "edc" files into "edj" files (which are actually just
data files managed by Eet). These "edj" files are fully portable and
can work on any OS/Architecture just like a JPEG or PNG might be
portable.

**Eet:**

*BSD 2-Clause license*

This library is a data storage, encoding and decoding library
designed to be extremely compact, fast and easy to use. It can take
data structures directly from memory and serialize them portably to
disk, then de-serialize them right back to the same data structures in
memory, handling allocation and population of memory all for you in 1
call. It handles indirections such as pointers, linked lists, arrays
and hash tables too, so almost any level of data structure complexity
can be easily saved and loaded back. It is used for "edj" files by
Edje as well as all configuration in Enlightenment, Elementary and
Terminology. It supports encryption of data too via SSL, signing of
files, as well as various compression techniques. It also supports
encoding and decoding of image data in lossless or lossy form.

**Eeze:**

*BSD 2-Clause license*

This library acts as an abstraction to discovering hardware interfaces
for sensors as well as removable media and much more.

**EFL:**

*BSD 2-Clause license*

This is just some core common header data like a common version number
for EFL and how to expose the EO API.

**Efreet:**

*BSD 2-Clause license*

This library provides code for handling Freedesktop.org standards such
as .desktop files, XDG Menus, Icon search paths and more. It provides
a central daemon to handle monitoring for changes that the library
talks to, and the daemon handles updating local cache files the
library reads.

**Eina:**

*LGPL v2 license*

This library provides low-level routines for common things like linked
lists, hash tables, growable arrays, basic string buffers, shared
string tokens, mmaped() file access, thread abstraction and locking,
memory pools, copy-on-write segments, iterators, matrices, general
data models, red/black trees, quad-trees, a simple SAX XML parser and
more.

**EIO:**

*LGPL v2 license*

This is an asynchronous I/O library for doing disk I/O without blocking.

**ElDBus:**

*LGPL v2 license*

This is a DBus access library to allow you to create DBus services as
well as clients. This glues in DBus into the main-loop so all access is
asynchronous.

**Embryo:**

*Small license (same as ZLib license)*

This is a small C-like language compiler and byte-code interpreter
library. This is used for scripting in Edje. The code is based on
original source from the Pawn/Small Language but has been made
portable (endianness issues fixed) and 64bit issues fixed, with the
runtime library being refactored to be extremely small.

**Emotion:**

*BSD 2-Clause license*

This is a wrapper around Gstreamer 1.x pluggable decoder libraries
This glues in the decoder library, and its output into a smart Evas object
that will display the playback for you as the video plays, as well as
providing higher level controls to seek, play, pause and query the stream
regardless of the back-end used.

**EO:**

*BSD 2-Clause license*

This is a core object system API that EFL 1.8 and on depend on. The
API is not finalized, so do not depend on it yet in EFL 1.8, but
future EFL versions will lock it down.

This object system does simple and multiple inheritance, refcounting,
strong and weak references, auto-deletion of child objects, unifies
callback handling with a single path, and also abstracts object
pointers to be indirect table lookups for increased safety at runtime.

**EPhysics:**

*BSD 2-Clause license*

This library provides a wrapper around the Bullet physics library,
allowing for it to be linked directly with Evas objects and control
their behavior as if they were real physical objects. This is now
disabled by default as it's rarely if ever used by anything.

**Ethumb:**

*LGPL v2 license*

This library provides core API for a thumbnailing daemon as well as
the thumbnail daemon itself. Ethumb Client talks with Ethumb to pass
off thumbnail generation to a central location to be done
asynchronously.

**Ethumb Client:**

*LGPL v2 license*

This is the client-side part of Ethumb that provides an API for
clients to request the Ethumb thumbnailer to generate or find cached
thumbnails of files.

**Evas:**

*BSD 2-Clause license*

This is the core rendering and scene graph abstraction library for
EFL. It manages a stateful 2D scene graph that defines the entire
content of any canvas. This supplies rendering back-ends for many
display systems like X11, Windows, Wayland, Frame-buffer etc. and via
many rendering APIs like OpenGL, OpenGL-ES 2, and pure software
implementations that are fast and accurate.

**Evil:**

*BSD 2-Clause license*

This library acts as a porting library for Windows to provide missing
libc calls not in Mingw32 that EFL needs. It is used internally and
no symbol is public.

-----

## Requirements

EFL requires a C and C++ compiler by default. C++ exists mostly to interface
to C++ libraries like Bullet and our C++ bindings.

Required by default:

* libpng
* libjpeg
* openjpeg2
* gstreamer (Ensure all codecs you want are installed.)
* zlib
* lua (lua 5.1, luajit or lua 5.2 support optional)
* libtiff
* openssl
* curl
* dbus
* libc
* fontconfig
* freetype2
* fribidi
* harfbuzz
* libpulse
* libsndfile
* libx11
* libxau
* libxcomposite
* libxdamage
* libxdmcp
* libxext
* libxfixes
* libxinerama
* libxrandr
* libxrender
* libxss
* libxtst
* libxcursor
* libxi (2.2 or newer)
* opengl(mesa etc.) (opengl/glx/full or opengl-es2/egl. full opengl only on osx - must be explicitly specified to be full to have support)
* giflib/libgif
* util-linux (limbount + libblkid)
* systemd / libudev
* poppler / poppler-cpp
* libraw
* libspectre
* librsvg
* openmp (clang needs libomp, while gcc uses libgomp)
* libwebp


### Wayland support

You may also want wayland support when on Linux. This enables support
for EFL to target wayland support for client applications. To do this
supply:
``` sh
-Dwl=true
```

### Framebuffer support

For more modern framebuffer support you may want drm/kms rendering
support so enable this. This is what you also want for wayland
compositor support in enlightenment as it will want to be able to
render to a modern framebuffer target with atomic buffer swapping. To
do this provide:
``` sh
-Ddrm=true
```

Legacy fbcon support also exists, but you probably no longer want to
use this as it is not maintained anymore. This supports basic frame-buffers
like /dev/fb as well as input via /dev/input for keyboards and mice in a
basic way. Enable this with:
``` sh
-Dfb=true
```

You may want to change the install prefix for EFL with:
``` sh
--prefix=/path/to/prefix
```

The default prefix if not given is "/usr/local". Many people like to
use prefixes like /opt/e or /opt/efl or /home/USERNAME/software etc.

### Compiler flags

You can affect compilation optimization, debugging and other factors
by setting your `CFLAGS` environment variable (and `CXXFLAGS`). Be aware
that to ensure ABI stability you should use the exact same `CFLAGS` /
`CXXFLAGS` for all the build of EFL and any applications/libraries that
depend on them.

There are many other configure options that can be used, but in
general it is not a good idea to go enabling or disabling things
unless you wish to break things. The defaults are well tested, with
the above recommended options also being well tested. Go much further
and your mileage may vary wildly. Disabling features is a good way of
breaking EFL functionality, so it is not recommended to mess with
these without understanding the implications. The defaults
have been carefully considered to provide full functionality so users
will not be missing anything.

-----

## Compiling and Installing

Meson is the build system used for this project. For more information
please see [mesonbuild.com](https://mesonbuild.com)

You will need normal build tooling installed such as a compiler (gcc
or clang for example), pkg-config, ninja, any relevant package-dev or
package-devel packages if your distribution splits out development
headers (e.g. libc6-dev) etc.

Depending on where dependencies,  you might have to set your
`PKG_CONFIG_PATH` environment variable like:
```sh
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/lib/pkgconfig
```

Also note that some distributions like to add extra arch directories
to your library locations so you might have to have more like:
```sh
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:/usr/local/lib/x86_64-linux-gnu/pkgconfig:/usr/lib/pkgconfig:/usr/lib64/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig
```

You will need to ensure that the destination library directory (e.g.
`/usr/local/lib` is in your /etc/ld.so.conf or /etc/ld.so.conf.d/
files and after installing anything that installs libraries you
re-run `ldconfig`. Please see relevant documentation on ldconfig and
ld.so.conf for your distribution.

You might also want to add the destination bin dir to your environment
variable PATH (see documentation on your shell PATH variable) such as:
```sh
export PATH=/usr/local/bin:/usr/bin:/bin
```

Normal compilation in /usr/local:
```sh
meson . build
ninja -C build
sudo ninja -C build install
```

For meson build generic options:
```sh
meson --help
```

For a list of project specific options supported:
```sh
cat meson_options.txt
```

To set 1 or more project specific options:
```sh
meson --prefix=/path/to -Doption=value [-Dother=value2] [...] . build
```

To display current configuration:
```sh
meson configure build
```

The above will only work after at least the following is done:
```sh
meson . build
```

### Quick build help

How to clean out the build and config and start fresh:
```sh
rm -rf build
```

How to make a dist tarball and check its build:
(must do it from git tree clone and commit all changes to git first)
```sh
ninja -C build dist
```

How to change prefix:
```sh
meson --prefix=/path/to/prefix . build
```

How to install in a specific destination directory for packaging:
```sh
DESTDIR=/path/to/destdir ninja -C build install
```

How to build with verbose output (full commands run):
```sh
ninja -C build -v
```

-----

**NOTE:** Nvidia OpenGL-ES/EGL seems to have bugs and performance issues.
All other drivers work just fine except Nvidia with OpenGL-ES/EGL. Users have
reported stuttering and graphical glitches and sometimes windows not
updating. If you want the best results and are stuck on Nvidia and unable to
change that, then:

  * Make sure you do not enable **Wayland** i.e. Meson option `-Dwl=false` (this is default anyway if not provided)
  * Make sure you do not enable **DRM** i.e. Meson option `-Ddrm=false` (this is default anyway if not provided)
  * Make sure **OpenGL** is **full** i.e. Meson option `-Dopengl=full`
