# Note that this is NOT a relocatable package
%define ver      0.0.1
%define rel      1
%define prefix   /usr

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
BuildRequires: libjpeg-devel
BuildRequires: zlib-devel
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

%package devel
Summary: Eet headers, static libraries, documentation and test programs
Group: System Environment/Libraries
Requires: %{name} = %{version}

%description devel
Headers, static libraries, test programs and documentation for Eet

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q

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
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-,root,root)
%attr(755,root,root) %{prefix}/lib/libeet.so*
%attr(755,root,root) %{prefix}/lib/libeet.la

%files devel
%attr(755,root,root) %{prefix}/lib/libeet.a
%attr(755,root,root) %{prefix}/bin/eet*
%{prefix}/include/Eet*
%doc AUTHORS
%doc COPYING
%doc README
%doc eet_docs.tar.gz

%changelog
* Sat Jun 23 2001 The Rasterman <raster@rasterman.com>
- Created spec file
