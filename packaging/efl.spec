Name:           efl
Version:        1.8.alpha
Release:        1
License:        LGPL-2.1
Summary:        Enlightenment Foundation Libraries - set of libraries used (not only) by E17
Url:            http://enlightenment.org/
Group:          System/Libraries
Source:         %{name}-%{version}.tar.bz2
Provides:       ecore = %{version}
Provides:       edje = %{version}
Provides:       eina = %{version}
Provides:       eio = %{version}
Provides:       eet = %{version}
Provides:       eeze = %{version}
Provides:       efreet = %{version}
Provides:       embryo = %{version}
Provides:       emotion = %{version}
Provides:       ephysics = %{version}
Provides:       eo = %{version}
Provides:       ethumb = %{version}
Provides:       eldbus = %{version}
Provides:       evas = %{version}
Obsoletes:      ecore < %{version}
Obsoletes:      edje < %{version}
Obsoletes:      eina < %{version}
Obsoletes:      eio < %{version}
Obsoletes:      eet < %{version}
Obsoletes:      eeze < %{version}
Obsoletes:      efreet < %{version}
Obsoletes:      embryo < %{version}
Obsoletes:      emotion < %{version}
Obsoletes:      eo < %{version}
Obsoletes:      ephysics < %{version}
Obsoletes:      ethumb < %{version}
Obsoletes:      evas < %{version}

BuildRequires:  pkgconfig(check)
BuildRequires:  zlib-devel
BuildRequires:  gettext-tools

#ecore
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xcursor)
BuildRequires:  pkgconfig(xinerama)
BuildRequires:  pkgconfig(xrandr)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xcomposite)
BuildRequires:  pkgconfig(xdamage)
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(xrender)
BuildRequires:  pkgconfig(xscrnsaver)
BuildRequires:  libXtst-devel
BuildRequires:  pkgconfig(xi)
BuildRequires:  pkgconfig(xgesture)
BuildRequires:  glib2-devel
BuildRequires:  pkgconfig(openssl)
BuildRequires:  gnutls-devel
BuildRequires:  curl-devel
BuildRequires:  pkgconfig(vconf)

#eldbus
BuildRequires:  dbus-devel

#edje
BuildRequires:  pkgconfig(lua)
BuildRequires:  pkgconfig(sndfile)
BuildRequires:  pkgconfig(libpulse)

#emotion
BuildRequires:  pkgconfig(gstreamer-0.10)
BuildRequires:  pkgconfig(gstreamer-plugins-base-0.10)

#evas
BuildRequires:  libexif-devel
BuildRequires:  giflib-devel
BuildRequires:  libtiff-devel
BuildRequires:  pkgconfig(libpng)
BuildRequires:  pkgconfig(xpm)
BuildRequires:  libjpeg-turbo-devel
BuildRequires:  pkgconfig(pixman-1)
BuildRequires:  pkgconfig(freetype2)
BuildRequires:  pkgconfig(fribidi)
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  pkgconfig(harfbuzz)
BuildRequires:  pkgconfig(libdri2)
BuildRequires:  opengl-es-devel

#eeze
BuildRequires:  libudev-devel
BuildRequires:  pkgconfig(capi-system-sensor)
BuildRequires:  libmount-devel

BuildRequires:  pkgconfig(ice)
BuildRequires:  pkgconfig(sm)
BuildRequires:  pkgconfig(dlog)

#ephysics
#BuildRequires:  libbullet-devel

%description
EFL is library collection providing various functionality used (not only) by Enlightenment 17, Terminology, Tizen mobile platform and much more.

%package devel
Summary:        Headers, pkgconfig files and other files for development with EFL
Group:          System/Libraries
Provides:       ecore-devel = %{version}
Provides:       edje-devel = %{version}
Provides:       eina-devel = %{version}
Provides:       eio-devel = %{version}
Provides:       eet-devel = %{version}
Provides:       eeze-devel = %{version}
Provides:       efreet-devel = %{version}
Provides:       embryo-devel = %{version}
Provides:       emotion-devel = %{version}
Provides:       ephysics-devel = %{version}
Provides:       eo-devel = %{version}
Provides:       ethumb-devel = %{version}
Provides:       eldbus-devel = %{version}
Provides:       evas-devel = %{version}
Obsoletes:      ecore-devel < %{version}
Obsoletes:      edje-devel < %{version}
Obsoletes:      eldbus-devel < %{version}
Obsoletes:      eina-devel < %{version}
Obsoletes:      eio-devel < %{version}
Obsoletes:      eet-devel < %{version}
Obsoletes:      eeze-devel < %{version}
Obsoletes:      efreet-devel < %{version}
Obsoletes:      embryo-devel < %{version}
Obsoletes:      emotion-devel < %{version}
Obsoletes:      eo-devel < %{version}
Obsoletes:      ephysics-devel < %{version}
Obsoletes:      ethumb-devel < %{version}
Obsoletes:      evas-devel < %{version}

%description devel
Headers, pkgconfig files and other files needed for development with EFL.

%prep
%setup -q

%build
NOCONFIGURE=1 ./autogen.sh
%configure --disable-physics --enable-tizen --enable-g-main-loop \
		--disable-xim --disable-scim --enable-gesture \
		--enable-tile-rotate --disable-rpath --with-x11=xlib --with-opengl=none

make

%install
%make_install

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%post -n efl-devel -p /sbin/ldconfig
%postun -n efl-devel -p /sbin/ldconfig

%files
%defattr(-, root, root)
%doc AUTHORS COPYING README
%{_libdir}/libe*.so.*
%{_libdir}/evas/modules/engines/*/*/module.so
%{_libdir}/evas/modules/loaders/*/*/module.so
%{_libdir}/evas/modules/savers/*/*/module.so
%{_libdir}/evas/cserve2/*
%{_libdir}/cmake/*/*.cmake
%{_libdir}/edje/modules/*/*/*.so
%{_libdir}/edje/utils/*/*
%{_libdir}/eeze/modules/sensor/*/*/*.so
%{_libdir}/efreet/*/*
%{_libdir}/emotion/modules/*/*/*.so
%{_libdir}/ethumb/modules/*/*/*
%{_libdir}/ethumb_client/utils/*/*
%{_libdir}/ecore_evas/engines/*/*/*.so
%{_libdir}/ecore/*/*/*/*.so

%{_bindir}/edje_cc
%{_bindir}/edje_codegen
%{_bindir}/edje_decc
%{_bindir}/edje_external_inspector
%{_bindir}/edje_inspector
%{_bindir}/edje_pick
%{_bindir}/edje_player
%{_bindir}/edje_recc
%{_bindir}/edje_watch
%{_bindir}/eet
%{_bindir}/eeze_disk_ls
%{_bindir}/eeze_mount
%{_bindir}/eeze_scanner
%{_bindir}/eeze_umount
%{_bindir}/efreetd
%{_bindir}/eina-bench-cmp
%{_bindir}/eldbus-codegen
%{_bindir}/embryo_cc
%{_bindir}/emotion_test
%{_bindir}/ethumb
%{_bindir}/ethumbd
%{_bindir}/ethumbd_client
%{_bindir}/evas_cserve2_client
%{_bindir}/evas_cserve2_debug
%{_bindir}/evas_cserve2_usage

%{_datadir}/*
/usr/share/locale/*/LC_MESSAGES/efl.mo
%manifest %{name}.manifest

%files devel
%defattr(-, root, root)
%{_includedir}/*
%{_libdir}/*.so
%{_libdir}/pkgconfig/*.pc
