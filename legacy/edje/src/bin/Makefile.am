
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EDJE_CFLAGS@

bin_SCRIPTS = \
@EDJE_RECC_PRG@

EXTRA_DIST = @EDJE_RECC_PRG@
EXTRA_SCRIPTS = edje_recc

bin_PROGRAMS = \
edje_cc \
edje_decc

EXTRA_PROGRAMS = \
edje_cc \
edje_decc

edje_cc_SOURCES = \
edje_cc.c \
edje_cc.h \
edje_cc_out.c \
edje_cc_parse.c \
edje_cc_mem.c \
edje_cc_handlers.c \
edje_cc_sources.c \
edje_prefix.c \
edje_prefix.h \
edje_main.h

edje_cc_LDADD = \
$(top_builddir)/src/lib/libedje.la $(EDJE_LIBS)

edje_cc_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la


edje_decc_SOURCES = \
edje_decc.c \
edje_decc.h \
edje_cc_mem.c \
edje_cc_sources.c

edje_decc_LDADD = \
$(top_builddir)/src/lib/libedje.la @edje_decc_lib@ $(EDJE_LIBS)

edje_decc_DEPENDENCIES = $(top_builddir)/src/lib/libedje.la
