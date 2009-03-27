
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir) \
-I$(top_srcdir) \
@EFREET_CFLAGS@

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

EFREETMIMEHEADERS = \
Efreet_Mime.h

EFREETMIMESOURCES = \
efreet_mime.c \
$(EFREETMIMEHEADERS)

libefreet_mime_la_SOURCES = \
$(EFREETMIMESOURCES)


EFREETTRASHHEADERS = \
Efreet_Trash.h

EFREETTRASHSOURCES = \
efreet_trash.c \
$(EFREETTRASHHEADERS)

libefreet_trash_la_SOURCES = \
$(EFREETTRASHSOURCES)


installed_headersdir = $(prefix)/include/efreet
installed_headers_DATA = $(EFREETHEADERS) $(EFREETMIMEHEADERS) $(EFREETTRASHHEADERS)

libefreet_la_LIBADD = @EFREET_LIBS@
libefreet_la_LDFLAGS = -version-info @version_info@

libefreet_mime_la_LIBADD = @EFREET_LIBS@ libefreet.la
libefreet_trash_la_LIBADD = @EFREET_LIBS@ libefreet.la
