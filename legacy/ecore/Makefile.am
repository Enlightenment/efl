## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = doc src

if HAVE_PO

SUBDIRS += po

endif

MAINTAINERCLEANFILES = \
ABOUT-NLS \
Makefile.in \
aclocal.m4 \
config.guess \
config.h.in \
config.h.in~ \
config.rpath \
config.sub \
configure \
depcomp \
install-sh \
ltmain.sh \
missing \
mkinstalldirs \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.gz \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc.tar.bz2 \
m4/libtool.m4 \
m4/lt~obsolete.m4 \
m4/ltoptions.m4 \
m4/ltsugar.m4 \
m4/ltversion.m4 \
m4/codeset.m4 \
m4/gettext.m4 \
m4/glibc21.m4 \
m4/iconv.m4 \
m4/intdiv0.m4 \
m4/inttypes_h.m4 \
m4/inttypes.m4 \
m4/inttypes-pri.m4 \
m4/isc-posix.m4 \
m4/lcmessage.m4 \
m4/lib-ld.m4 \
m4/lib-link.m4 \
m4/lib-prefix.m4 \
m4/nls.m4 \
m4/po.m4 \
m4/progtest.m4 \
m4/stdint_h.m4 \
m4/uintmax_t.m4 \
m4/ulonglong.m4

if HAVE_PO

MAINTAINERCLEANFILES += \
po/boldquot.sed \
po/en@boldquot.header \
po/en@quot.header \
po/insert-header.sin \
po/Makefile.in.in \
po/Makevars.template \
po/quot.sed \
po/remove-potcdate.sin \
po/Rules-quot

endif

bin_SCRIPTS = 

EXTRA_DIST = \
AUTHORS \
COPYING \
autogen.sh \
ecore.supp \
ecore.pc.in \
ecore-con.pc.in \
ecore-config.pc.in \
ecore-directfb.pc.in\
ecore-evas.pc.in \
ecore-fb.pc.in \
ecore-file.pc.in \
ecore-imf.pc.in \
ecore-imf-evas.pc.in \
ecore-ipc.pc.in \
ecore-x.pc.in \
ecore-win32.pc.in \
ecore-sdl.pc.in \
ecore-cocoa.pc.in \
ecore-input.pc.in \
ecore-wince.pc.in \
ecore.spec.in \
ecore.spec \
m4/ac_abstract_socket.m4 \
m4/ac_attribute.m4 \
m4/check_x_extension.m4 \
m4/ecore_check_module.m4 \
m4/ecore_check_options.m4 \
m4/efl_doxygen.m4 \
m4/efl_path_max.m4 \
m4/efl_tests.m4 \
m4/efl_threads.m4


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ecore.pc

if BUILD_ECORE_CON
pkgconfig_DATA += ecore-con.pc
endif

if BUILD_ECORE_CONFIG
pkgconfig_DATA += ecore-config.pc
endif

if BUILD_ECORE_DIRECTFB
pkgconfig_DATA += ecore-directfb.pc
endif

if BUILD_ECORE_EVAS
pkgconfig_DATA += ecore-evas.pc
endif

if BUILD_ECORE_FB
pkgconfig_DATA += ecore-fb.pc
endif

if BUILD_ECORE_FILE
pkgconfig_DATA += ecore-file.pc
endif

if BUILD_ECORE_IMF
pkgconfig_DATA += ecore-imf.pc
endif

if BUILD_ECORE_IMF_EVAS
pkgconfig_DATA += ecore-imf-evas.pc
endif

if BUILD_ECORE_INPUT
pkgconfig_DATA += ecore-input.pc
endif

if BUILD_ECORE_INPUT_EVAS
pkgconfig_DATA += ecore-input-evas.pc
endif

if BUILD_ECORE_IPC
pkgconfig_DATA += ecore-ipc.pc
endif

if BUILD_ECORE_X
pkgconfig_DATA += ecore-x.pc
endif

if BUILD_ECORE_WIN32
pkgconfig_DATA += ecore-win32.pc
endif

if BUILD_ECORE_WINCE
pkgconfig_DATA += ecore-wince.pc
endif

if BUILD_ECORE_SDL
pkgconfig_DATA += ecore-sdl.pc
endif

if BUILD_ECORE_COCOA
pkgconfig_DATA += ecore-cocoa.pc
endif

.PHONY: doc

# Documentation

doc:
	@echo "entering doc/"
	make -C doc doc

# Unit tests

if EFL_ENABLE_TESTS

check-local:
	@./src/tests/ecore_suite

else

check-local:
	@echo "reconfigure with --enable-tests"

endif
