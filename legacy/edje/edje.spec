%define _missing_doc_files_terminate_build 0

Summary: Complex Graphical Design/Layout Engine
Name: edje
Version: 0.5.0
Release: 1.%(date '+%Y%m%d')
Copyright: BSD
Group: System Environment/Libraries
Source: ftp://ftp.enlightenment.org/pub/evoak/%{name}-%{version}.tar.gz
Packager: The Rasterman <raster@rasterman.com>
URL: http://www.enlightenment.org/
#BuildSuggests: libjpeg-devel zlib-devel eet-devel xorg-x11-devel
Requires: libjpeg zlib ecore evas eet imlib2
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Edje is a complex graphical design and layout engine. It provides a
mechanism for allowing configuration data to define visual elements in
terms of layout, behavior, and appearance.  Edje allows for multiple
collections of layouts in one file, allowing a complete set of images,
animations, and controls to exist as a unified whole.

Edje separates the arrangement, appearance, and behavior logic into
distinct independent entities.  This allows visual objects to share
image data and configuration information without requiring them to do
so.  This separation and simplistic event driven style of programming
can produce almost any look and feel one could want for basic visual
elements. Anything more complex is likely the domain of an application
or widget set that may use Edje as a conveneient way of being able to
configure parts of the display.

%package devel
Summary: Edje headers, static libraries, documentation and test programs
Group: System Environment/Libraries
Requires: %{name} = %{version}

%description devel
Headers, static libraries, test programs and documentation for Eet

%prep
%setup -q

%build
%{configure} --prefix=%{_prefix}
%{__make} %{?_smp_mflags} %{?mflags}

%install
%{__make} %{?mflags_install} DESTDIR=$RPM_BUILD_ROOT install
test -x `which doxygen` && sh gendoc || :

%post
/sbin/ldconfig || :

%postun
/sbin/ldconfig || :

%clean
test "x$RPM_BUILD_ROOT" != "x/" && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc AUTHORS COPYING README
%{_libdir}/libedje.so*
%{_bindir}/edje
%{_bindir}/edje_cc
%{_bindir}/edje_ls
%{_bindir}/edje_decc
%{_bindir}/edje_recc
%{_datadir}/edje

%files devel
%defattr(-, root, root)
%doc doc/html
%{_libdir}/libedje.a
%{_libdir}/libedje.la
%{_libdir}/pkgconfig/edje.pc
%{_bindir}/edje-config
%{_includedir}/Edje*

%changelog
