# Note that this is NOT a relocatable package
%define ver      0.0.1
%define rel      1
%define prefix   /usr

Summary: embryo
Name: embryo
Version: %ver
Release: %rel
Copyright: BSD
Group: System Environment/Libraries
Source: ftp://ftp.enlightenment.org/pub/embryo/embryo-%{ver}.tar.gz
BuildRoot: /var/tmp/embryo-root
Packager: The Rasterman <raster@rasterman.com>
URL: http://www.enlightenment.org/

Docdir: %{prefix}/doc

%description

Embryo is a tiny library designed as a virtual machine to interpret a limited
set of small compiled programs.

%package devel
Summary: Embryo headers, static libraries, documentation and test programs
Group: System Environment/Libraries
Requires: %{name} = %{version}

%description devel
Headers, static libraries, test programs and documentation for Embryo

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
%attr(755,root,root) %{prefix}/lib/libembryo.so.*
%{prefix}/lib/libembryo.la
%attr(755,root,root) %{prefix}/bin/embryo_cc
%attr(755,root,root) %{prefix}/bin/embryo

%files devel
%attr(755,root,root) %{prefix}/lib/libembryo.so
%attr(755,root,root) %{prefix}/lib/libembryo.a
%attr(755,root,root) %{prefix}/bin/embryo-config
%{prefix}/lib/pkgconfig/embryo.pc
%{prefix}/include/Embryo*
%doc AUTHORS
%doc COPYING
%doc README
%doc embryo_docs.tar.gz

%changelog
* Sat Jun 23 2001 The Rasterman <raster@rasterman.com>
- Created spec file
