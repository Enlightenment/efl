
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir) \
-I$(top_builddir) \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EVIL_CFLAGS@ \
@EINA_CFLAGS@

bin_PROGRAMS = embryo_cc

embryo_cc_SOURCES = \
embryo_cc_amx.h \
embryo_cc_osdefs.h \
embryo_cc_sc.h \
embryo_cc_sc1.c \
embryo_cc_sc2.c \
embryo_cc_sc3.c \
embryo_cc_sc4.c \
embryo_cc_sc5.c \
embryo_cc_sc6.c \
embryo_cc_sc7.c \
embryo_cc_scexpand.c \
embryo_cc_sclist.c \
embryo_cc_scvars.c \
embryo_cc_prefix.c \
embryo_cc_prefix.h

embryo_cc_CFLAGS = @WIN32_CFLAGS@
embryo_cc_LDADD = $(top_builddir)/src/lib/libembryo.la @EVIL_LIBS@ -lm
embryo_cc_LDFLAGS = @lt_enable_auto_import@

EXTRA_DIST = \
embryo_cc_sc5.scp \
embryo_cc_sc7.scp
