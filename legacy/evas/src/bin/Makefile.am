MAINTAINERCLEANFILES = Makefile.in

if EVAS_CSERVE

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/lib/cserve \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@EET_CFLAGS@ \
@FONTCONFIG_CFLAGS@ \
@pthread_cflags@ \
@PIXMAN_CFLAGS@

bin_PROGRAMS = evas_cserve evas_cserve_tool

evas_cserve_SOURCES = \
evas_cserve_main.c

evas_cserve_LDADD = \
$(top_builddir)/src/lib/libevas.la \
@pthread_libs@ \
@EINA_LIBS@

evas_cserve_tool_LDFLAGS =

evas_cserve_tool_SOURCES = \
evas_cserve_tool.c

evas_cserve_tool_LDADD = \
$(top_builddir)/src/lib/libevas.la \
@pthread_libs@ \
@EINA_LIBS@

endif

if EVAS_CSERVE2

SUBDIRS = loaders

libexec_PROGRAMS = evas_cserve2 evas_cserve2_slave dummy_slave
bin_PROGRAMS = evas_cserve2_client

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/lib/cserve2 \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_LIBEXEC_DIR=\"$(libexecdir)\" \
@EINA_CFLAGS@

evas_cserve2_SOURCES = \
evas_cserve2.h \
evas_cserve2_slave.h \
evas_cserve2_main.c \
evas_cserve2_slaves.c \
evas_cserve2_messages.c \
evas_cserve2_shm.c \
evas_cserve2_cache.c \
evas_cserve2_requests.c \
evas_cserve2_main_loop_linux.c

evas_cserve2_LDADD = \
@EINA_LIBS@ \
@EFL_SHM_OPEN_LIBS@

evas_cserve2_client_SOURCES = \
evas_cserve2_client.c

evas_cserve2_slave_SOURCES = \
evas_cserve2_slave.c \
evas_cserve2_utils.c

evas_cserve2_slave_LDFLAGS = -export-dynamic

evas_cserve2_slave_LDADD = \
@EINA_LIBS@ \
@EFL_SHM_OPEN_LIBS@

dummy_slave_SOURCES = \
dummy_slave.c

dummy_slave_LDADD = \
@EINA_LIBS@ \
@EFL_SHM_OPEN_LIBS@

endif
