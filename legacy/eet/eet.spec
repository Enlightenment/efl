# Note that this is NOT a relocatable package
%define ver      0.0.1
%define rel      1
%define prefix   /usr/local

Summary: eet
Name: eet
Version: %ver
Release: %rel
Copyright: BSD
Group: System Environment/Libraries
Source: ftp://ftp.enlightenment.org/pub/eet/eet-%{ver}.tar.gz
BuildRoot: /var/tmp/eet-root
Packager: The Rasterman <raster@rasterman.com>
URL: http://www.enlightenment.org/
Requires: libjpeg
Requires: zlib

Docdir: %{prefix}/doc

%description

Eet is a tiny library designed to write an arbitary set of chunks of data
to a file and optionally compress each chunk (very much like a zip file)
and allow fast random-access reading of the file later on. It does not
do zip as a zip itself has more complexity than is needed, and it was much
simpler to impliment this once here.

It also can encode and decode data structures in memory, as well as image
data for saving to eet files or sending across the network to other
machines, or just writing to arbitary files on the system. All data is
encoded ina platform independant way and can be written and read by any
architecture.

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
%doc README COPYING ChangeLog eet_docs.tar.gz
%attr(755,root,root) %{prefix}/bin/*
%attr(755,root,root) %{prefix}/lib/*
%{prefix}/share/*

%doc AUTHORS
%doc COPYING
%doc README
%doc eet_docs.tar.gz

%changelog
* Sat Jun 23 2001 The Rasterman <raster@rasterman.com>
- Created spec file

