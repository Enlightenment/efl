# this is NOT relocatable, unless you alter the patch!
%define	name	ecore
%define	ver	0.0.2
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
if [ -e ./configure ]
then
  ./configure --prefix=%{prefix}
else
  ./autogen.sh --prefix=%{prefix}
fi
make

%install
make prefix=$RPM_BUILD_ROOT%{prefix} install

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{prefix}/lib/libecore.so.*

%files devel
%defattr(-,root,root)
%{prefix}/lib/libecore.so
%{prefix}/lib/libecore.*a
%{prefix}/include/Ecore.h
%{prefix}/bin/ecore-config
