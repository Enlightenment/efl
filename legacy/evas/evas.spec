%define _missing_doc_files_terminate_build 0

Summary: Multi-platform Canvas Library
Name: evas
Version: 1.0.0
Release: 0.pre13.%(date '+%Y%m%d')
License: BSD
Group: System Environment/Libraries
URL: http://www.enlightenment.org/
Source: ftp://ftp.enlightenment.org/pub/evas/evas-%{version}_pre13.tar.gz
Packager: %{?_packager:%{_packager}}%{!?_packager:Michael Jennings <mej@eterm.org>}
Vendor: %{?_vendorinfo:%{_vendorinfo}}%{!?_vendorinfo:The Enlightenment Project (http://www.enlightenment.org/)}
Distribution: %{?_distribution:%{_distribution}}%{!?_distribution:%{_vendor}}
#BuildSuggests: xorg-x11-devel
BuildRequires: libpng-devel eet-devel edb-devel XFree86-devel freetype2-devel libjpeg-devel
Provides: evas evas_software_x11 evas_loader_png evas_loader_jpeg evas_loader_eet evas_loader_edb
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Evas is a clean display canvas API for several target display systems
that can draw anti-aliased text, smooth super and sub-sampled scaled
images, alpha-blend objects much and more.

%package devel
Summary: Evas headers, static libraries, documentation and test programs
Group: System Environment/Libraries
Requires: %{name} = %{version}
Requires: libpng-devel eet-devel edb-devel XFree86-devel freetype-devel libjpeg-devel

%description devel
Headers, static libraries, test programs and documentation for Evas.

%prep
%setup -q -n %{name}-%{version}_pre13

%build
%{configure} --prefix=%{_prefix}
%{__make} %{?_smp_mflags} %{?mflags}
test -x `which doxygen` && /bin/sh gendoc || :

%install
%{__make} %{?mflags_install} DESTDIR=$RPM_BUILD_ROOT install

%clean
test "x$RPM_BUILD_ROOT" != "x/" && rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-, root, root)
%doc AUTHORS README COPYING
%{_libdir}/libevas.so.*
%{_libdir}/libevas.la
%{_bindir}/evas_*
%{_datadir}/%{name}

%files devel
%defattr(-, root, root)
%doc doc/html
%{_libdir}/libevas.so
%{_libdir}/libevas.a
%{_bindir}/evas-config
%{_libdir}/pkgconfig/evas.pc
%{_includedir}/Evas*

%changelog
