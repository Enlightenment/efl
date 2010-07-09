
MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libefreet.la libefreet_mime.la libefreet_trash.la

EFREETHEADERS = \
Efreet.h \
efreet_base.h \
efreet_desktop.h \
efreet_icon.h \
efreet_ini.h \
efreet_menu.h \
efreet_utils.h \
efreet_uri.h

EFREETSOURCES = \
efreet.c \
efreet_base.c \
efreet_icon.c \
efreet_xml.c \
efreet_ini.c \
efreet_desktop.c \
efreet_desktop_command.c \
efreet_menu.c \
efreet_utils.c \
efreet_uri.c

includes_HEADERS = $(EFREETHEADERS) Efreet_Mime.h Efreet_Trash.h
includesdir = $(includedir)/efreet-@VMAJ@

# Not sure if this was for 'make dist', so left it in but commented - dh
# dist_installed_headers_DATA = $(EFREETHEADERS) Efreet_Mime.h Efreet_Trash.h

libefreet_la_SOURCES = $(EFREETSOURCES)
libefreet_la_CPPFLAGS =  \
-DPACKAGE_DATA_DIR=\"$(datadir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-I$(top_builddir)/src/lib \
-I$(top_srcdir)/src/lib \
@EFL_EFREET_BUILD@ \
@EFREET_CFLAGS@
libefreet_la_LIBADD = @EFREET_LIBS@ @WIN32_LIBS@
libefreet_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@


libefreet_mime_la_SOURCES = efreet_mime.c
libefreet_mime_la_CPPFLAGS = \
-I$(top_builddir)/src/lib \
-I$(top_srcdir)/src/lib \
@EFL_EFREET_MIME_BUILD@ \
@EFREET_CFLAGS@
libefreet_mime_la_LIBADD = @EFREET_LIBS@ libefreet.la @WIN32_LIBS@
libefreet_mime_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@


libefreet_trash_la_SOURCES = efreet_trash.c
libefreet_trash_la_CPPFLAGS = \
-I$(top_builddir)/src/lib \
-I$(top_srcdir)/src/lib \
@EFL_EFREET_TRASH_BUILD@ \
@EFREET_CFLAGS@
libefreet_trash_la_LIBADD = @EFREET_LIBS@ libefreet.la
libefreet_trash_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@

EXTRA_DIST = efreet_private.h efreet_xml.h
