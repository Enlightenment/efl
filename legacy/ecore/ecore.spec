Summary: Enlightened Core X interface library
Name: ecore
Version: 1.0.0
Release: 0.pre7
Copyright: BSD
Group: User Interface/X
Source: ftp://ftp.enlightenment.org/enlightenment/%{name}-%{version}_pre7.tar.gz
URL: http://www.enlightenment.org/efm.html
Packager: Michael Jennings <mej@eterm.org>
Vendor: The Enlightenment Development Team <e-develop@enlightenment.org>
#BuildSuggests: openssl-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Ecore is the event/X abstraction layer that makes doing selections,
Xdnd, general X stuff, event loops, timeouts and idle handlers fast,
optimized, and convenient. It's a separate library so anyone can make
use of the work put into Ecore to make this job easy for applications.

%package devel
Summary: Ecore headers and development libraries.
Group: Development/Libraries
Requires: %{name} = %{version}

%description devel
Ecore development files

%prep
%setup -q -n %{name}-%{version}_pre7

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
%doc AUTHORS COPYING* README*
%{_libdir}/libecore*.so.*
%{_libdir}/libecore*.la
%{_bindir}/ecore_*
%{_datadir}/ecore

%files devel
%defattr(-, root, root)
%doc doc/html
%{_libdir}/libecore*.so
%{_libdir}/libecore*.a
%{_libdir}/libecore*.la
%{_libdir}/ecore_config_ipc_*.so
%{_libdir}/ecore_config_ipc_*.a
%{_libdir}/ecore_config_ipc_*.la
%{_bindir}/ecore-config
%{_datadir}/aclocal/ecore.m4
%{_includedir}/Ecore*.h
