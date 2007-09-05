
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-I$(top_builddir) \
-I$(top_srcdir) \
-g -O0 -W -Wall \
@ECORE_CFLAGS@

lib_LTLIBRARIES = libefreet.la libefreet_mime.la

EFREETHEADERS = \
Efreet.h \
efreet_base.h \
efreet_desktop.h \
efreet_icon.h \
efreet_ini.h \
efreet_menu.h \
efreet_utils.h 

EFREETSOURCES = \
efreet.c \
efreet_base.c \
efreet_icon.c \
efreet_xml.c \
efreet_ini.c \
efreet_desktop.c \
efreet_menu.c \
efreet_utils.c \
efreet_private.h \
efreet_xml.h \
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

installed_headersdir = $(prefix)/include/efreet
installed_headers_DATA = $(EFREETHEADERS) $(EFREETMIMEHEADERS)

libefreet_la_LIBADD = @ECORE_LIBS@
libefreet_la_LDFLAGS = -version-info 1:0:0

libefreet_mime_la_LIBADD = @ECORE_LIBS@ libefreet.la