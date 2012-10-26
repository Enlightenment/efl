
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/embryo \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/embryo \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EFL_CFLAGS@

if HAVE_WINDOWS
AM_CPPFLAGS += \
-I$(top_srcdir)/src/lib/evil \
-I$(top_builddir)/src/lib/evil
endif

bin_PROGRAMS = embryo_cc

embryo_cc_SOURCES = \
embryo_cc_amx.h \
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

embryo_cc_LDADD = \
$(top_builddir)/src/lib/embryo/libembryo.la \
$(top_builddir)/src/lib/eina/libeina.la \
-lm

if HAVE_WINDOWS
embryo_cc_LDADD += $(top_builddir)/src/lib/evil/libevil.la
endif

EXTRA_DIST = \
embryo_cc_sc5.scp \
embryo_cc_sc7.scp
