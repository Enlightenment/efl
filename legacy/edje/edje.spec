Summary: edje
Name: edje
Version: 0.5.0
Release: 1
Copyright: BSD
Group: System Environment/Libraries
Source: ftp://ftp.enlightenment.org/pub/evoak/%{name}-%{version}.tar.gz
Packager: The Rasterman <raster@rasterman.com>
URL: http://www.enlightenment.org/
BuildRequires: libjpeg-devel zlib-devel
Requires: libjpeg zlib ecore evas eet imlib2
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Edje is a Graphical Design Library

%package devel
Summary: Edje headers, static libraries, documentation and test programs
Group: System Environment/Libraries
Requires: %{name} = %{version}

%description devel
Headers, static libraries, test programs and documentation for Eet

%prep
%setup -q

%build
%{configure}

if [ "$SMP" != "" ]; then
  (%{__make} "MAKE=make -k -j $SMP"; exit 0)
  %{__make}
else
  %{__make}
fi

%install
%{__make} DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig || :

%postun
/sbin/ldconfig || :

%files
%defattr(-, root, root)
%doc AUTHORS COPYING README
%{_libdir}/libedje.so*
%{_libdir}/libedje_edit.so*
%{_bindir}/edje
%{_bindir}/edje_cc
%{_bindir}/edje_ls
%{_datadir}/edje

%files devel
%defattr(-, root, root)
%doc edje_docs.tar.gz
%{_libdir}/libedje.a
%{_libdir}/libedje.la
%{_libdir}/libedje_edit.a
%{_libdir}/libedje_edit.la
%{_bindir}/edje-config
%{_includedir}/Edje*

%changelog
* Sat Jun 23 2001 The Rasterman <raster@rasterman.com>
- Created spec file
