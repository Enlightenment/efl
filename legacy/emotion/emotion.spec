# Note that this is NOT a relocatable package
%define ver      0.0.1
%define rel      1
%define prefix   /usr

Summary: emotion
Name: emotion
Version: %ver
Release: %rel
Copyright: BSD
Group: System Environment/Libraries
Source: ftp://ftp.enlightenment.org/pub/emotion/emotion-%{ver}.tar.gz
BuildRoot: /var/tmp/emotion-root
Packager: The Rasterman <raster@rasterman.com>
URL: http://www.enlightenment.org/
BuildRequires: libjpeg-devel
BuildRequires: zlib-devel
Requires: libjpeg
Requires: zlib

Docdir: %{prefix}/doc

%description

Emotion is a Media Library

%package devel
Summary: Emotion headers, static libraries, documentation and test programs
Group: System Environment/Libraries
Requires: %{name} = %{version}

%description devel
Headers, static libraries, test programs and documentation for Eet

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
%attr(755,root,root) %{prefix}/lib/libemotion.a
%attr(755,root,root) %{prefix}/lib/libemotion.so.*
%attr(755,root,root) %{prefix}/bin/emotion_*
%attr(755,root,root) %{prefix}/lib/emotion/emotion_decoder_xine.a
%attr(755,root,root) %{prefix}/lib/xine/plugins/1.0.0/xineplug_vo_out_emotion.a
%{_datadir}/emotion
%doc AUTHORS
%doc COPYING
%doc README

%files devel
%attr(755,root,root) %{prefix}/bin/emotion-config
%attr(755,root,root) %{prefix}/lib/pkgconfig/emotion.pc
%attr(755,root,root) %{prefix}/lib/libemotion.so
%attr(755,root,root) %{prefix}/lib/libemotion.la
%attr(755,root,root) %{prefix}/lib/emotion/emotion_decoder_xine.la
%attr(755,root,root) %{prefix}/lib/emotion/emotion_decoder_xine.so
%attr(755,root,root) %{prefix}/lib/xine/plugins/1.0.0/xineplug_vo_out_emotion.so
%attr(755,root,root) %{prefix}/lib/xine/plugins/1.0.0/xineplug_vo_out_emotion.la
%{prefix}/include/Emotion*
%doc emotion_docs.tar.gz

%changelog
* Fri Jun 18 2004 Azundris <edevel@azundris.com>
- Added missing files
* Sat Jun 23 2001 The Rasterman <raster@rasterman.com>
- Created spec file                                            
