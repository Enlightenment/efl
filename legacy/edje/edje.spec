# Note that this is NOT a relocatable package
%define ver      0.0.1
%define rel      1
%define prefix   /usr

Summary: edje
Name: edje
Version: %ver
Release: %rel
Copyright: BSD
Group: System Environment/Libraries
Source: ftp://ftp.enlightenment.org/pub/evoak/edje-%{ver}.tar.gz
BuildRoot: /var/tmp/edje-root
Packager: The Rasterman <raster@rasterman.com>
URL: http://www.enlightenment.org/
BuildRequires: libjpeg-devel
BuildRequires: zlib-devel
Requires: libjpeg
Requires: zlib

Docdir: %{prefix}/doc

%description

Edje is a Graphical Design Library

%package devel
Summary: Edje headers, static libraries, documentation and test programs
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
%attr(755,root,root) %{prefix}/lib/libedje.so*
%attr(755,root,root) %{prefix}/lib/libedje.la
%attr(755,root,root) %{prefix}/bin/edje
%attr(755,root,root) %{prefix}/bin/edje_cc
%attr(755,root,root) %{prefix}/share/edje

%files devel
%attr(755,root,root) %{prefix}/lib/libedje.a
%attr(755,root,root) %{prefix}/bin/edje-config
%{prefix}/include/Edje*
%doc AUTHORS
%doc COPYING
%doc README
%doc edje_docs.tar.gz

%changelog
* Sat Jun 23 2001 The Rasterman <raster@rasterman.com>
- Created spec file
