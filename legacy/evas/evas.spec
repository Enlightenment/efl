# Note that this is NOT a relocatable package
%define ver      1.0.0_pre10
%define rel      1
%define prefix   /usr

Summary: evas
Name: evas
Version: %ver
Release: %rel
Copyright: BSD
Group: System Environment/Libraries
Source: ftp://ftp.enlightenment.org/pub/evas/evas-%{ver}.tar.gz
BuildRoot: /var/tmp/evas-root
Packager: The Rasterman <raster@rasterman.com>
URL: http://www.enlightenment.org/
BuildRequires: XFree86-devel
BuildRequires: freetype-devel
BuildRequires: libjpeg-devel
BuildRequires: libpng-devel
BuildRequires: eet-devel
BuildRequires: edb-devel
Requires: XFree86
Requires: freetype >= 2.0.0
Requires: libpng >= 1.0.0
Requires: libjpeg
Requires: eet
Requires: edb
Provides: evas evas_software_x11 evas_loader_png evas_loader_jpeg evas_loader_eet evas_loader_edb

Docdir: %{prefix}/doc

%description
Evas is a clean display canvas API for several target display systems that
can draw anti-aliased text, smooth super and sub-sampled scaled images,
alpha-blend objects much and more.

%package devel
Summary: Evas headers, static libraries, documentation and test programs
Group: System Environment/Libraries
Requires: %{name} = %{version}

%description devel
Headers, static libraries, test programs and documentation for Evas.

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q

%build
./configure \
--prefix=%{prefix} \
--enable-software-x11 \
--enable-image-loader-png \
--enable-image-loader-jpeg \
--enable-image-loader-eet \
--enable-image-loader-edb \
--enable-cpu-p2-only \
--enable-cpu-mmx \
--enable-cpu-sse \
--enable-cpu-c \
--enable-scale-smooth \
--enable-scale-sample \
--enable-convert-8-rgb-332 \
--enable-convert-8-rgb-666 \
--enable-convert-8-rgb-232 \
--enable-convert-8-rgb-222 \
--enable-convert-8-rgb-221 \
--enable-convert-8-rgb-121 \
--enable-convert-8-rgb-111 \
--enable-convert-16-rgb-565 \
--enable-convert-16-rgb-555 \
--enable-convert-16-rgb-rot-0 \
--enable-convert-16-rgb-rot-90 \
--enable-convert-16-rgb-rot-270 \
--enable-convert-32-rgb-8888 \
--enable-convert-32-rgbx-8888 \
--enable-convert-32-bgr-8888 \
--enable-convert-32-bgrx-8888 \
--enable-convert-32-rgb-rot-0 \
--enable-convert-32-rgb-rot-90 \
--enable-convert-32-rgb-rot-270

make CFLAGS="-O2 -mpentiumpro -march=pentiumpro -mcpu=pentiumpro"

###########################################################################

%install
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-,root,root)
%attr(755,root,root) %{prefix}/lib/libevas.so*
%attr(755,root,root) %{prefix}/lib/libevas.la
%attr(755,root,root) %{prefix}/bin/evas_*
%attr(755,root,root) %{prefix}/share/evas

%files devel
%attr(755,root,root) %{prefix}/lib/libevas.a
%attr(755,root,root) %{prefix}/bin/evas-config
%{prefix}/include/Evas*
%{prefix}/share/*
%doc AUTHORS
%doc COPYING
%doc README
%doc evas_docs.tar.gz

%changelog
* Sat Jun 23 2001 The Rasterman <raster@rasterman.com>
- Created spec file

