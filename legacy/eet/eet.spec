%define _missing_doc_files_terminate_build 0

Summary: Library for speedy data storage, retrieval, and compression.
Name: eet
Version: 0.9.9
Release: 2.%(date '+%Y%m%d')
License: BSD
Group: System Environment/Libraries
Source: ftp://ftp.enlightenment.org/pub/eet/eet-%{version}.tar.gz
Packager: %{?_packager:%{_packager}}%{!?_packager:Michael Jennings <mej@eterm.org>}
Vendor: %{?_vendorinfo:%{_vendorinfo}}%{!?_vendorinfo:The Enlightenment Project (http://www.enlightenment.org/)}
Distribution: %{?_distribution:%{_distribution}}%{!?_distribution:%{_vendor}}
URL: http://www.enlightenment.org/
#BuildSuggests: xorg-x11-devel doxygen
BuildRequires: libjpeg-devel XFree86-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Eet is a tiny library designed to write an arbitary set of chunks of
data to a file and optionally compress each chunk (very much like a
zip file) and allow fast random-access reading of the file later
on. It does not do zip as a zip itself has more complexity than is
needed, and it was much simpler to implement this once here.

It also can encode and decode data structures in memory, as well as
image data for saving to eet files or sending across the network to
other machines, or just writing to arbitary files on the system. All
data is encoded in a platform independant way and can be written and
read by any architecture.

%package devel
Summary: Eet headers, static libraries, documentation and test programs
Group: System Environment/Libraries
Requires: %{name} = %{version}

%description devel
Headers, static libraries, test programs and documentation for Eet

%prep
%setup -q

%build
%{configure} --prefix=%{_prefix}
%{__make} %{?_smp_mflags} %{?mflags}
test -x `which doxygen` && sh gendoc || :

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
%doc AUTHORS COPYING README
%{_libdir}/libeet.so.*
%{_libdir}/libeet.la
%{_bindir}/eet

%files devel
%defattr(-, root, root)
%doc html
%{_libdir}/libeet.so
%{_libdir}/libeet.a
%{_bindir}/eet-config
%{_libdir}/pkgconfig/eet.pc
%{_includedir}/Eet*

%changelog
