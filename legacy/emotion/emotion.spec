%define _missing_doc_files_terminate_build 0

Summary: emotion
Name: emotion
Version: 0.0.1
Release: 1.%(date '+%Y%m%d')
Copyright: BSD
Group: System Environment/Libraries
URL: http://www.enlightenment.org/
Source: ftp://ftp.enlightenment.org/pub/emotion/%{name}-%{version}.tar.gz
Packager: Michael Jennings <mej@eterm.org>
#BuildSuggests: xorg-x11-devel xine-lib-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Emotion is a Media Library

%package devel
Summary: Emotion headers, static libraries, documentation and test programs
Group: System Environment/Libraries
Requires: %{name} = %{version}

%description devel
Headers, static libraries, test programs and documentation for Emotion

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
%doc AUTHORS COPYING* README
%{_libdir}/libemotion.a
%{_libdir}/libemotion.so.*
%{_bindir}/emotion_*
%{_libdir}/emotion/emotion_decoder_xine.a
%{_libdir}/xine/plugins/*/xineplug_vo_out_emotion.a
%{_datadir}/emotion

%files devel
%defattr(-, root, root)
%doc doc/html
%{_bindir}/emotion-config
%{_libdir}/pkgconfig/emotion.pc
%{_libdir}/libemotion.so
%{_libdir}/libemotion.la
%{_libdir}/emotion/emotion_decoder_xine.la
%{_libdir}/emotion/emotion_decoder_xine.so
%{_libdir}/xine/plugins/*/xineplug_vo_out_emotion.so
%{_libdir}/xine/plugins/*/xineplug_vo_out_emotion.la
%{_includedir}/Emotion*

%changelog
