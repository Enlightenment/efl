Summary: embryo
Name: embryo
Version: 0.0.1
Release: 1
Copyright: BSD
Group: System Environment/Libraries
URL: http://www.enlightenment.org/
Source: ftp://ftp.enlightenment.org/pub/embryo/%{name}-%{version}.tar.gz
Packager: Michael Jennings <mej@eterm.org
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Embryo is a tiny library designed as a virtual machine to interpret a
limited set of small compiled programs.

%package devel
Summary: Embryo headers, static libraries, documentation and test programs
Group: System Environment/Libraries
Requires: %{name} = %{version}

%description devel
Headers, static libraries, test programs and documentation for Embryo

%prep
%setup -q

%build
%{configure} --prefix=%{_prefix}
%{__make} %{?_smp_mflags} %{?mflags}

%install
%{__make} %{?mflags_install} DESTDIR=$RPM_BUILD_ROOT install
test -x `which doxygen` && sh gendoc || :

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%clean
test "x$RPM_BUILD_ROOT" != "x/" && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc AUTHORS COPYING* README
%{_libdir}/libembryo.so.*
%{_libdir}/libembryo.la
%attr(755,root,root) %{_bindir}/embryo_cc
%attr(755,root,root) %{_bindir}/embryo

%files devel
%defattr(-, root, root)
%doc doc/html
%{_libdir}/libembryo.so
%{_libdir}/libembryo.a
%{_bindir}/embryo-config
%{_datadir}/embryo/examples
%{_datadir}/embryo/include
%{_libdir}/pkgconfig/embryo.pc
%{_includedir}/Embryo*

%changelog
