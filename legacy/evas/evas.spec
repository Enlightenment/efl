# this is NOT relocatable, unless you alter the patch!
%define	name	evas
%define	ver	0.0.1
%define	rel	1
%define prefix  /usr

# NB: Raster and I talked about it, and heh says evas_test shouldn't be in
# the rpms, since it's not geared at all towards end users. I'm commenting
# out and withholding the patch to honor that..

Summary: Enlightened Canvas Library
Name: %{name}
Version: %{ver}
Release: %{rel}
Copyright: BSD
Group: User Interface/X
URL: http://www.enlightenment.org/efm.html
Packager: Term <kempler@utdallas.edu>
Vendor: The Enlightenment Development Team <e-develop@enlightenment.org>
Source: ftp://ftp.enlightenment.org/enlightenment/%{name}-%{ver}.tar.gz
BuildRoot: /var/tmp/%{name}-root
Requires: imlib2 >= 1.0.0

#Patch1: evas_test-fix.patch

%description
Evas is an advanced canvas library, providing three backends for
rendering: X11 (without some features like alpha-blending), imlib2, or
OpenGL (hardware accelerated). Due to its simple API, evas can be
developed with rapidly, and cleanly.

Install evas if you want to develop applications against the only
hardware-accelerated canvas library, or if you want to try out the
applications under development.

%package devel
Summary: Enlightened Canvas Library headers and development libraries.
Group: Development/Libraries
Requires: %{name} = %{ver}

%description devel
Evas development headers and libraries.

%prep
%setup -q

#%patch1 -p1 -b .evas_test-fix

%build
./autogen.sh --prefix=%{prefix}
make

%install
make prefix=$RPM_BUILD_ROOT%{prefix} install
#cp -f test/evas_test $RPM_BUILD_ROOT%{prefix}/bin
mkdir -p $RPM_BUILD_ROOT%{prefix}/share/evas
cp -rf test/img $RPM_BUILD_ROOT%{prefix}/share/evas
cp -rf test/fnt $RPM_BUILD_ROOT%{prefix}/share/evas

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{prefix}/lib/libevas.so.*
%{prefix}/bin/evas_*
%{prefix}/share/evas/*

%files devel
%defattr(-,root,root)
%{prefix}/lib/libevas.so
%{prefix}/lib/libevas.*a
%{prefix}/include/Evas.h
%{prefix}/bin/evas-config
