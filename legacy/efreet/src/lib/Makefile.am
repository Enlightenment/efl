
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
efreet_menu.c \
efreet_utils.c \
efreet_uri.c \
efreet_private.h \
efreet_xml.h \
efreet_cache.c \
$(EFREETHEADERS)

libefreet_la_SOURCES = \
	$(EFREETSOURCES)
libefreet_la_CPPFLAGS =  \
-DPACKAGE_DATA_DIR=\"$(datadir)\" \
-I$(top_builddir) \
-I$(top_srcdir) \
@EFL_EFREET_BUILD@ \
@EFREET_CFLAGS@
libefreet_la_LIBADD = @EFREET_LIBS@ @win32_libs@
libefreet_la_LDFLAGS = -no-undefined -version-info @version_info@ @efreet_release_info@

EFREETMIMEHEADERS = \
Efreet_Mime.h

EFREETMIMESOURCES = \
efreet_mime.c \
$(EFREETMIMEHEADERS)

libefreet_mime_la_SOURCES = \
$(EFREETMIMESOURCES)
libefreet_mime_la_CPPFLAGS = \
-I$(top_builddir) \
-I$(top_srcdir) \
@EFL_EFREET_MIME_BUILD@ \
@EFREET_CFLAGS@


EFREETTRASHHEADERS = \
Efreet_Trash.h

EFREETTRASHSOURCES = \
efreet_trash.c \
$(EFREETTRASHHEADERS)

libefreet_trash_la_SOURCES = \
$(EFREETTRASHSOURCES)
libefreet_trash_la_CPPFLAGS = \
-I$(top_builddir) \
-I$(top_srcdir) \
@EFL_EFREET_TRASH_BUILD@ \
@EFREET_CFLAGS@


installed_headersdir = $(prefix)/include/efreet
installed_headers_DATA = $(EFREETHEADERS) $(EFREETMIMEHEADERS) $(EFREETTRASHHEADERS)

libefreet_mime_la_LDFLAGS = -no-undefined -version-info @version_info@ @efreet_mime_release_info@
libefreet_trash_la_LDFLAGS = -no-undefined -version-info @version_info@ @efreet_trash_release_info@

libefreet_mime_la_LIBADD = @EFREET_LIBS@ libefreet.la @win32_libs@
libefreet_trash_la_LIBADD = @EFREET_LIBS@ libefreet.la
