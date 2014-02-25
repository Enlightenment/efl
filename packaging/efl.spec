%bcond_with wayland
%bcond_with x

Name:           efl
Version:        1.9.0
Release:        0
License:        LGPL-2.1
Summary:        Enlightenment Foundation Libraries - set of libraries used (not only) by E17
Url:            http://enlightenment.org/
Group:          Graphics & UI Framework/API
Source:         %{name}-%{version}.tar.bz2
Source1001:     efl.manifest

BuildRequires:  pkgconfig(check)
BuildRequires:  zlib-devel
BuildRequires:  gettext-tools

%if %{with wayland}
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  pkgconfig(wayland-cursor)
BuildRequires:  pkgconfig(wayland-egl)
BuildRequires:  pkgconfig(glesv2)
%endif

%if %{with x}
BuildRequires:  pkgconfig(libdri2)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xcursor)
BuildRequires:  pkgconfig(xinerama)
BuildRequires:  pkgconfig(xpm)
BuildRequires:  pkgconfig(xrandr)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xcomposite)
BuildRequires:  pkgconfig(xdamage)
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(xrender)
BuildRequires:  pkgconfig(xscrnsaver)
BuildRequires:  libXtst-devel
BuildRequires:  pkgconfig(xi)
BuildRequires:  pkgconfig(gles20)
BuildRequires:  pkgconfig(ice)
BuildRequires:  pkgconfig(sm)
%endif

BuildRequires:  glib2-devel
BuildRequires:  pkgconfig(openssl)
BuildRequires:  gnutls-devel
BuildRequires:  curl-devel
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(xkbcommon)
BuildRequires:  systemd-devel

#eldbus
BuildRequires:  dbus-devel

#edje
BuildRequires:  pkgconfig(lua)
BuildRequires:  pkgconfig(sndfile)
BuildRequires:  pkgconfig(libpulse)

#emotion
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  pkgconfig(gstreamer-plugins-base-1.0)

#evas
BuildRequires:  libexif-devel
BuildRequires:  giflib-devel
BuildRequires:  libtiff-devel
BuildRequires:  pkgconfig(libpng)
BuildRequires:  libjpeg-turbo-devel
BuildRequires:  pkgconfig(pixman-1)
BuildRequires:  pkgconfig(freetype2)
BuildRequires:  pkgconfig(fribidi)
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  pkgconfig(harfbuzz)
BuildRequires:  pkgconfig(libtbm)

#eeze
BuildRequires:  libudev-devel
BuildRequires:  pkgconfig(capi-system-sensor)
BuildRequires:  libmount-devel
BuildRequires:  pkgconfig(dlog)

%description
EFL is a library collection providing various functionality used (not only) by
Enlightenment, Terminology, Tizen and more.

############ efl-data
%package data
Summary: Data files for EFL package

%description data
This package includes data files common to all packages.

############ Eina
%package -n eina
Summary: Data type library
Requires: %{name}-data = %{version}-%{release}

%description -n eina
Eina is a data type library.

%package -n eina-examples
Summary:  Examples for the eina package
Group:    Graphics & UI Framework/Testing
Requires: eina = %{version}-%{release}

%description -n eina-examples
Example files for eina

%package -n eina-devel
Summary:  Development components for the eina package
Group:    Graphics & UI Framework/Development
Requires: eina = %{version}-%{release}

%description -n eina-devel
Development files for eina.

############ Eet
%package -n eet
Summary: Library for speedy data storage, retrieval, and compression
Requires: %{name}-data = %{version}-%{release}

%description -n eet
Eet is a tiny library designed to write an arbitrary set of chunks of
data to a file and optionally compress each chunk (very much like a
zip file) and allow fast random-access reading of the file later
on. It does not do zip as a zip itself has more complexity than is
needed, and it was much simpler to implement this once here.

It also can encode and decode data structures in memory, as well as
image data for saving to eet files or sending across the network to
other machines, or just writing to arbitrary files on the system. All
data is encoded in a platform independent way and can be written and
read by any architecture.

%package -n eet-tools
Summary:   Eet Tools
Group:     Graphics & UI Framework/Utilities
Requires:  eet = %{version}-%{release}
Provides:  eet-bin

%description -n eet-tools
The eet tools package the eet binary utility.

%package -n eet-examples
Summary:  Examples for the eet package
Group:    Graphics & UI Framework/Testing
Requires: eet = %{version}-%{release}

%description -n eet-examples
Example files for eet

%package -n eet-devel
Summary:  Development components for the eet package
Group:    Graphics & UI Framework/Development
Requires: eet = %{version}-%{release}

%description -n eet-devel
Development files for eet

############ Eo
%package -n eo
Summary: EFL generic object system library
Requires: %{name}-data = %{version}-%{release}

%description -n eo
The Eo generic object system. It was designed to be the base object
system for the EFL.

%package -n eo-examples
Summary:  Examples for the eo package
Group:    Graphics & UI Framework/Testing
Requires: eo = %{version}-%{release}

%description -n eo-examples
Example files for eo

%package -n eo-devel
Summary:  Development components for the eo package
Group:    Graphics & UI Framework/Development
Requires: eo = %{version}-%{release}

%description -n eo-devel
Development files for eo

############ Evas
%package -n evas
Summary: Multi-platform canvas library
Requires: %{name}-data = %{version}-%{release}

%description -n evas
Evas is a clean display canvas API for several target display systems
that can draw anti-aliased text, smooth super and sub-sampled scaled
images, alpha-blend objects much and more.

%package -n evas-examples
Summary:  Examples for the evas package
Group:    Graphics & UI Framework/Testing
Requires: evas = %{version}-%{release}

%description -n evas-examples
Example files for evas

%package -n evas-devel
Summary:  Development components for the evas package
Group:    Graphics & UI Framework/Development
Requires: evas = %{version}-%{release}

%description -n evas-devel
Development files for evas

############ Ecore
%package -n ecore
Summary: Enlightened Core X interface library
Requires: %{name}-data = %{version}-%{release}

%description -n ecore
Ecore is the event/X abstraction layer that makes doing selections,
Xdnd, general X stuff, event loops, timeouts and idle handlers fast,
optimized, and convenient.

%package -n ecore-examples
Summary:  Examples for the ecore package
Group:    Graphics & UI Framework/Testing
Requires: ecore = %{version}-%{release}

%description -n ecore-examples
Example files for ecore

%package -n ecore-devel
Summary:  Development components for the ecore package
Group:    Graphics & UI Framework/Development
Requires: ecore = %{version}-%{release}

%description -n ecore-devel
Development files for ecore

############ Eldbus
%package -n eldbus
Summary: D-Bus bindings for EFL
Requires: %{name}-data = %{version}-%{release}

%description -n eldbus
Eldbus allows connecting to both system and session buses acting as
both client and service roles.

This is a replacement for old library "edbus". Its main purpose is to
fix some core bugs and completely encapsulating D-Bus without exposing
libdbus to end-user. One day libdbus may be completely removed as a
dependency.

%package -n eldbus-examples
Summary:  Examples for the eldbus package
Group:    Graphics & UI Framework/Testing
Requires: eldbus = %{version}-%{release}

%description -n eldbus-examples
Example files for eldbus

%package -n eldbus-devel
Summary:  Development components for the eldbus package
Group:    Graphics & UI Framework/Development
Requires: eldbus = %{version}-%{release}

%description -n eldbus-devel
Development files for eldbus

############ Embryo
%package -n embryo
Summary: A small virtual machine engine (in a library) and bytecode compiler
Requires: %{name}-data = %{version}-%{release}
Provides: embryo-bin

%description -n embryo
Embryo is a tiny library designed as a virtual machine to interpret a
limited set of small compiled programs.

%package -n embryo-devel
Summary:  Development components for the embryo package
Group:    Graphics & UI Framework/Development
Requires: embryo = %{version}-%{release}

%description -n embryo-devel
Development files for embryo

############ Eio
%package -n eio
Summary: Enlightenment Input/Output Library
Requires: %{name}-data = %{version}-%{release}

%description -n eio
Enlightenment Input/Output Library

%package -n eio-examples
Summary:  Examples for the eio package
Group:    Graphics & UI Framework/Testing
Requires: eio = %{version}-%{release}

%description -n eio-examples
Example files for eio

%package -n eio-devel
Summary:  Development components for the eio package
Group:    Graphics & UI Framework/Development
Requires: eio = %{version}-%{release}

%description -n eio-devel
Development files for eio

############ Ephysics
# %package -n ephysics
# Summary: EFL wrapper for the Bullet Physics library
#
# %description -n ephysics
# EPhysics is a library that makes it easy to use Ecore, Evas and Bullet
# Physics together. It's a kind of wrapper, a glue, between these libraries.
# It's not intended to be a physics library (we already have many out there).
#
# Enlightenment Input/Output Library
#
# %package -n ephysics-examples
# Summary:  Examples for the ephysics package
# Group:    Graphics & UI Framework/Testing
# Requires: ephysics = %{version}-%{release}
#
# %description -n ephysics-examples
# Example files for ephysics
#
# %package -n ephysics-devel
# Summary:  Development components for the ephysics package
# Group:    Graphics & UI Framework/Development
# Requires: ephysics = %{version}-%{release}
#
# %description -n ephysics-devel
# Development files for ephysics
#

############ Edje
%package -n edje
Summary: Complex Graphical Design/Layout Engine
Requires: %{name}-data = %{version}-%{release}

%description -n edje
Edje is a complex graphical design and layout engine. It provides a
mechanism for allowing configuration data to define visual elements in
terms of layout, behavior, and appearance.  Edje allows for multiple
collections of layouts in one file, allowing a complete set of images,
animations, and controls to exist as a unified whole.

Edje separates the arrangement, appearance, and behavior logic into
distinct independent entities.  This allows visual objects to share
image data and configuration information without requiring them to do
so.  This separation and simplistic event driven style of programming
can produce almost any look and feel one could want for basic visual
elements. Anything more complex is likely the domain of an application
or widget set that may use Edje as a conveneient way of being able to
configure parts of the display.

%package -n edje-tools
Summary:   Edje tools
Group:     Graphics & UI Framework/Utilities
Requires:  edje = %{version}-%{release}
Provides:  edje-bin

%description -n edje-tools
The edje tools package include the compiler, decompiler, visualizer, code
generator and more.

%package -n edje-examples
Summary:  Examples for the edje package
Group:    Graphics & UI Framework/Testing
Requires: edje = %{version}-%{release}

%description -n edje-examples
Example files for edje

%package -n edje-devel
Summary:  Development components for the edje package
Group:    Graphics & UI Framework/Development
Requires: edje-tools = %{version}-%{release}

%description -n edje-devel
Development files for edje

############ Eeze
%package -n eeze
Summary: Device convenience library
Requires: %{name}-data = %{version}-%{release}

%description -n eeze
Eeze is a library for manipulating devices through udev with a simple
and fast api. It interfaces directly with libudev, avoiding such
middleman daemons as udisks/upower or hal, to immediately gather
device information the instant it becomes known to the system.  This
can be used to determine such things as:
  * If a cdrom has a disk inserted
  * The temperature of a cpu core
  * The remaining power left in a battery
  * The current power consumption of various parts
  * Monitor in realtime the status of peripheral devices

Each of the above examples can be performed by using only a single
eeze function, as one of the primary focuses of the library is to
reduce the complexity of managing devices.

%package -n eeze-devel
Summary:  Development components for the eeze package
Group:    Graphics & UI Framework/Development
Requires: eeze = %{version}-%{release}

%description -n eeze-devel
Development files for eeze

############ Efreet
%package -n efreet
Summary: FreeDesktop.Org standards implementation
Requires: %{name}-data = %{version}-%{release}

%description -n efreet
Efreet implements the FreeDesktop.Org application and MIME-handling
standards.

%package -n efreet-devel
Summary:  Development components for the efreet package
Group:    Graphics & UI Framework/Development
Requires: efreet = %{version}-%{release}

%description -n efreet-devel
Development files for efreet

############ Emotion
%package -n emotion
Summary: EFL Media Library
Requires: %{name}-data = %{version}-%{release}

%description -n emotion
Emotion is a media library with support for different backends as plug-ins.

%package -n emotion-examples
Summary:  Examples for the emotion package
Group:    Graphics & UI Framework/Testing
Requires: emotion = %{version}-%{release}

%description -n emotion-examples
Example files for emotion

%package -n emotion-devel
Summary:  Development components for the emotion package
Group:    Graphics & UI Framework/Development
Requires: emotion = %{version}-%{release}

%description -n emotion-devel
Development files for emotion

############ Ethumb
%package -n ethumb
Summary: EFL thumbnail generator library
Requires: %{name}-data = %{version}-%{release}

%description -n ethumb
Enlightenment thumbnailing library

%package -n ethumb-examples
Summary:  Examples for the ethumb package
Group:    Graphics & UI Framework/Testing
Requires: ethumb = %{version}-%{release}

%description -n ethumb-examples
Example files for ethumb

%package -n ethumb-devel
Summary:  Development components for the ethumb package
Group:    Graphics & UI Framework/Development
Requires: ethumb = %{version}-%{release}

%description -n ethumb-devel
Development files for emotion


%prep
%setup -q
cp %{SOURCE1001} .


%build

%if ! %{with x}
CFLAGS+=" -DMESA_EGL_NO_X11_HEADERS "
%endif

%reconfigure \
    --disable-physics \
    --enable-tizen \
    --enable-g-main-loop \
    --disable-xim \
    --disable-scim \
    --disable-gesture \
    --with-tests=regular \
    --with-opengl=es \
%if %{with wayland}
    --enable-wayland \
    --enable-egl \
    --enable-tile-rotate \
    --disable-rpath \
%endif
%if %{with x}
    --with-x11=xlib \
    --disable-gesture \
%else
    --with-x11=none \
    --enable-tile-rotate \
    --disable-rpath \
%endif
    --enable-always-build-examples \
    --enable-lua-old \
    --enable-i-really-know-what-i-am-doing-and-that-this-will-probably-break-things-and-i-will-fix-them-myself-and-send-patches-aaa

make %{?_smp_mflags}


%install
%make_install
make datadir=%{buildroot}%{_datadir} install-examples
rm -rf %{buildroot}%{_libdir}/ecore/system/upower


%post -n eina -p /sbin/ldconfig
%postun -n eina -p /sbin/ldconfig

%post -n eet -p /sbin/ldconfig
%postun -n eet -p /sbin/ldconfig

%post -n eo -p /sbin/ldconfig
%postun -n eo -p /sbin/ldconfig

%post -n evas -p /sbin/ldconfig
%postun -n evas -p /sbin/ldconfig

%post -n ecore -p /sbin/ldconfig
%postun -n ecore -p /sbin/ldconfig

%post -n eldbus -p /sbin/ldconfig
%postun -n eldbus -p /sbin/ldconfig

%post -n embryo -p /sbin/ldconfig
%postun -n embryo -p /sbin/ldconfig

%post -n eio -p /sbin/ldconfig
%postun -n eio -p /sbin/ldconfig

%post -n edje -p /sbin/ldconfig
%postun -n edje -p /sbin/ldconfig

%post -n eeze -p /sbin/ldconfig
%postun -n eeze -p /sbin/ldconfig

%post -n efreet -p /sbin/ldconfig
%postun -n efreet -p /sbin/ldconfig

%post -n emotion -p /sbin/ldconfig
%postun -n emotion -p /sbin/ldconfig

%post -n ethumb -p /sbin/ldconfig
%postun -n ethumb -p /sbin/ldconfig


%files data
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_datadir}/locale/*/*/*.mo
%{_includedir}/efl-1/Efl_Config.h

%files -n eina
%manifest %{name}.manifest
%defattr(-,root,root,-)
%license COPYING
%{_libdir}/libeina.so.*
%{_bindir}/eina-bench-cmp

%files -n eina-examples
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_datadir}/eina/examples/*
%{_libdir}/eina/examples/*

%files -n eina-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/eina-1/*.h
%{_includedir}/eina-1/eina/*
%{_libdir}/libeina.so
%{_libdir}/pkgconfig/eina*.pc
%{_libdir}/cmake/Eina/*.cmake

%files -n eet
%manifest %{name}.manifest
%defattr(-,root,root,-)
%license COPYING
%{_libdir}/libeet*.so.*

%files -n eet-tools
%manifest %{name}.manifest
%{_bindir}/eet

%files -n eet-examples
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_datadir}/eet/examples/*
%{_libdir}/eet/examples/*

%files -n eet-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/eet-1/*.h
%{_libdir}/libeet.so
%{_libdir}/pkgconfig/eet*.pc
%{_libdir}/cmake/Eet/*.cmake

%files -n eo
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/libeo.so.*

%files -n eo-examples
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_datadir}/eo/examples/*
%{_libdir}/eo/examples/*

%files -n eo-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/eo-1/*.h
%{_libdir}/libeo.so
%{_libdir}/pkgconfig/eo*.pc
%{_datadir}/eo/gdb/eo_gdb.py
%{_datadir}/gdb/auto-load/usr/lib*/*
%{_libdir}/cmake/Eo/*.cmake

%files -n evas
%manifest %{name}.manifest
%defattr(-,root,root,-)
%license COPYING
%{_bindir}/evas_cserve2_client
%{_bindir}/evas_cserve2_debug
%{_bindir}/evas_cserve2_shm_debug
%{_bindir}/evas_cserve2_usage
%{_libdir}/evas/modules/*/*/*/module.so
%{_libdir}/libevas.so.*
%{_libdir}/evas/cserve2
%{_datadir}/evas/checkme

%files -n evas-examples
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_datadir}/evas/examples/*
%{_libdir}/evas/examples/*

%files -n evas-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/evas-1/*.h
%{_libdir}/libevas.so
%{_libdir}/pkgconfig/evas*.pc
%{_libdir}/cmake/Evas/*.cmake

%files -n ecore
%manifest %{name}.manifest
%defattr(-,root,root,-)
%license COPYING
%{_libdir}/libecore.so.*
%{_libdir}/libecore_audio.so.*
%{_libdir}/libecore_avahi.so.*
%{_libdir}/libecore_con.so.*
%{_libdir}/libecore_evas.so.*
%{_libdir}/libecore_file.so.*
%{_libdir}/libecore_imf.so.*
%{_libdir}/libecore_imf_evas.so.*
%{_libdir}/libecore_input.so.*
%{_libdir}/libecore_input_evas.so.*
%{_libdir}/libecore_ipc.so.*
%if %{with wayland}
%{_libdir}/libecore_wayland.so.*
%endif
%if %{with x}
%{_libdir}/libecore_x.so.*
%endif
%{_libdir}/ecore_evas/engines/*/*/module.so
%{_libdir}/ecore_imf/modules/*/*/module.so
%{_libdir}/ecore/system/tizen/*/module.so
%{_datadir}/ecore/checkme
%{_datadir}/ecore_imf/checkme

%files -n ecore-examples
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_datadir}/ecore*/examples/*
%{_libdir}/ecore*/examples/*

%files -n ecore-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/ecore*/*.h
%{_libdir}/libecore.so
%{_libdir}/libecore_audio.so
%{_libdir}/libecore_avahi.so
%{_libdir}/libecore_con.so
%{_libdir}/libecore_evas.so
%{_libdir}/libecore_file.so
%{_libdir}/libecore_imf.so
%{_libdir}/libecore_imf_evas.so
%{_libdir}/libecore_input.so
%{_libdir}/libecore_input_evas.so
%{_libdir}/libecore_ipc.so
%if %{with wayland}
%{_libdir}/libecore_wayland.so
%endif
%if %{with x}
%{_libdir}/libecore_x.so
%endif
%{_libdir}/pkgconfig/ecore*.pc
%{_libdir}/cmake/Ecore/*.cmake

%files -n eldbus
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/libeldbus.so.*
%{_bindir}/eldbus*
%license COPYING

%files -n eldbus-examples
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_datadir}/eldbus/examples/*
%{_libdir}/eldbus/examples/*

%files -n eldbus-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/libeldbus.so
%{_libdir}/pkgconfig/eldbus*.pc
%{_includedir}/eldbus*/*
%{_libdir}/cmake/Eldbus/*.cmake

%files -n embryo
%manifest %{name}.manifest
%defattr(-,root,root,-)
%license COPYING
%{_bindir}/embryo_cc
%{_libdir}/libembryo.so.*
%{_datadir}/embryo/include/default.inc

%files -n embryo-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/embryo-1/*.h
%{_libdir}/libembryo.so
%{_libdir}/pkgconfig/embryo*.pc

%files -n eio
%manifest %{name}.manifest
%defattr(-,root,root,-)
%license COPYING
%{_libdir}/libeio.so.*

%files -n eio-examples
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_datadir}/eio/examples/*
%{_libdir}/eio/examples/*

%files -n eio-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/eio-1/*
%{_libdir}/libeio.so
%{_libdir}/pkgconfig/eio*.pc

%files -n edje
%manifest %{name}.manifest
%defattr(-,root,root,-)
%license COPYING
%{_libdir}/libedje.so.*
%{_libdir}/edje/utils/*/*
%{_datadir}/edje/include/edje.inc
%{_datadir}/mime/packages/edje.xml

%files -n edje-tools
%manifest %{name}.manifest
%{_bindir}/edje*
%{_datadir}/edje/data/*

%files -n edje-examples
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_datadir}/edje/examples/*
%{_libdir}/edje/examples/*

%files -n edje-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/edje-1/*.h
%{_libdir}/libedje.so
%{_libdir}/pkgconfig/edje*.pc
%{_libdir}/cmake/Edje/*.cmake

%files -n eeze
%manifest %{name}.manifest
%defattr(-,root,root,-)
%license COPYING
%{_libdir}/libeeze.so.*
%{_bindir}/eeze_disk_ls
%{_bindir}/eeze_mount
%{_bindir}/eeze_scanner
%{_bindir}/eeze_umount
%{_libdir}/eeze/modules/sensor/*/*/module.so
%{_datadir}/eeze/checkme

%files -n eeze-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/eeze-1/*.h
%{_libdir}/libeeze.so
%{_libdir}/pkgconfig/eeze*.pc
%{_libdir}/cmake/Eeze/*.cmake

%files -n efreet
%manifest %{name}.manifest
%defattr(-,root,root,-)
%license COPYING
%{_datadir}/dbus-1/services/org.enlightenment.Efreet.service
%{_bindir}/efreetd
%{_libdir}/efreet/*/efreet_desktop_cache_create
%{_libdir}/efreet/*/efreet_icon_cache_create
%{_libdir}/libefreet.so.*
%{_libdir}/libefreet_mime.so.*
%{_libdir}/libefreet_trash.so.*
%{_datadir}/efreet/*

%files -n efreet-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/efreet-1/*.h
%{_libdir}/libefreet.so
%{_libdir}/libefreet_mime.so
%{_libdir}/libefreet_trash.so
%{_libdir}/pkgconfig/efreet*.pc
%{_libdir}/cmake/Efreet/*.cmake

%files -n emotion
%manifest %{name}.manifest
%defattr(-,root,root,-)
%license COPYING
%{_libdir}/libemotion.so.*
%{_libdir}/edje/modules/emotion/*/module.so
%{_libdir}/emotion/modules/*/*/module.so
%{_datadir}/emotion/*
%{_bindir}/emotion_test

%files -n emotion-examples
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_datadir}/emotion/examples/*
%{_libdir}/emotion/examples/*

%files -n emotion-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/emotion-1/*
%{_libdir}/libemotion.so
%{_libdir}/pkgconfig/emotion*.pc
%{_libdir}/cmake/Emotion/*.cmake

%files -n ethumb
%manifest %{name}.manifest
%defattr(-,root,root,-)
%license COPYING
%{_libdir}/libethumb.so.*
%{_libdir}/libethumb_client.so.*
%{_datadir}/dbus-1/services/org.enlightenment.Ethumb.service
%{_datadir}/ethumb/*
%{_datadir}/ethumb_client/*
#%{_datadir}/ethumb_client/*/*
%{_libdir}/ethumb/modules/*/*/module.so
%{_libdir}/ethumb/modules/*/*/template.edj
%{_libdir}/ethumb_client/utils/*/ethumbd_slave
%{_bindir}/ethumb
%{_bindir}/ethumbd
%{_bindir}/ethumbd_client

%files -n ethumb-examples
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_datadir}/ethumb_client/examples/*
%{_libdir}/ethumb_client/examples/*

%files -n ethumb-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/ethumb-1/*.h
%{_includedir}/ethumb-client-1/*.h
%{_libdir}/libethumb.so
%{_libdir}/libethumb_client.so
%{_libdir}/pkgconfig/ethumb*.pc
%{_libdir}/cmake/Ethumb/*.cmake
%{_libdir}/cmake/EthumbClient/*.cmake
