# this is NOT relocatable, unless you alter the patch!
%define	name	ecore
%define	ver	1.0.0_pre4
%define	rel	1
%define prefix  /usr

Summary: Enlightened Core X interface library
Name: %{name}
Version: %{ver}
Release: %{rel}
Copyright: BSD
Group: User Interface/X
URL: http://www.enlightenment.org/efm.html
Packager: The Rasterman <raster@rasterman.com> Term <term@twistedpath.org>
Vendor: The Enlightenment Development Team <e-develop@enlightenment.org>
Source: ftp://ftp.enlightenment.org/enlightenment/%{name}-%{ver}.tar.gz
BuildRoot: /var/tmp/%{name}-root

%description
Ecore is brub.

%package devel
Summary: Ecore headers and development libraries.
Group: Development/Libraries
Requires: %{name} = %{ver}

%description devel
Ecore development files

%prep
%setup -q

%build
./configure \
--prefix=%{prefix} \
--enable-ecore-x \
--enable-ecore-evas \
--enable-ecore-job \
--enable-ecore-con \
--enable-ecore-ipc

make

%install
make DESTDIR=$RPM_BUILD_ROOT install

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%attr(755,root,root) %{prefix}/lib/libecore*.so.*
%{prefix}/lib/libecore*.la
%attr(755,root,root) %{prefix}/bin/ecore_*
%{prefix}/share/ecore

%files devel
%defattr(-,root,root)
%attr(755,root,root) %{prefix}/lib/libecore*.so
%attr(755,root,root) %{prefix}/lib/libecore*.a
%attr(755,root,root) %{prefix}/bin/ecore-config
%{prefix}/include/Ecore*.h
%doc AUTHORS
%doc COPYING
%doc README
%doc ecore_docs.tar.gz
