# Note that this is NOT a relocatable package
%define ver      0.0.1
%define rel      1
%define prefix   /usr/local

Summary: eet
Name: eet
Version: %ver
Release: %rel
Copyright: BSD
Group: Base/Group
Source: ftp://ftp.enlightenment.org/pub/eet/eet-%{ver}.tar.gz
BuildRoot: /var/tmp/eet-root
Packager: The Rasterman <raster@rasterman.com>
URL: http://www.enlightenment.org/
Requires: libjpeg
Requires: zlib

Docdir: %{prefix}/doc

%description
Eet

%prep
%setup

%build
./configure --prefix=%prefix

if [ "$SMP" != "" ]; then
  (make "MAKE=make -k -j $SMP"; exit 0)
  make
else
  make
fi
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

