# Note that this is NOT a relocatable package
%define ver      1.0.0-pre4
%define rel      1
%define prefix   /usr

Summary: evas
Name: evas
Version: %ver
Release: %rel
Copyright: BSD
Group: Base/Group
Source: ftp://ftp.enlightenment.org/pub/evas/evas-%{ver}.tar.gz
BuildRoot: /var/tmp/evas-root
Packager: The Rasterman <raster@rasterman.com>
URL: http://www.enlightenment.org/
Requires: freetype >= 2.0.0
Requires: libpng >= 1.0.0
Requires: libjpeg

Docdir: %{prefix}/doc

%description
Evas - the library

%prep
%setup

%build
./configure \
--prefix=%prefix \
--enable-software-x11 \
--enable-image-loader-png \
--enable-image-loader-jpeg \
--enable-cpu-p2-only \
--enable-cpu-mmx \
--enable-cpu-sse \
--enable-cpu-c \
--enable-scale-smooth \
--enable-scale-sample \
--enable-convert-16-rgb-565 \
--enable-convert-16-rgb-555 \
--enable-convert-16-rgb-rot-0 \
--enable-convert-32-rgb-8888 \
--enable-convert-32-bgr-8888 \
--enable-convert-32-rgb-rot-0

make CFLAGS="-O9 -mpentiumpro -march=pentiumpro -mcpu=pentiumpro"

###########################################################################

%install
rm -rf $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT%{prefix} install

%clean
rm -rf $RPM_BUILD_ROOT

%post

%postun

%files
%defattr(-,root,root)
%doc README COPYING ChangeLog
%attr(755,root,root) %{prefix}/bin/*
%attr(755,root,root) %{prefix}/lib/*
%{prefix}/share/*

%doc AUTHORS
%doc COPYING
%doc README

%changelog
* Sat Jun 23 2001 The Rasterman <raster@rasterman.com>
- Created spec file

