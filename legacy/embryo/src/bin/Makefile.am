MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -DLINUX -I$(top_srcdir)/src/lib -I$(top_srcdir) -I$(top_builddir) \
           -DPACKAGE_BIN_DIR=\"@PACKAGE_BIN_DIR@\" \
	   -DPACKAGE_LIB_DIR=\"@PACKAGE_LIB_DIR@\" \
	   -DPACKAGE_DATA_DIR=\"@PACKAGE_DATA_DIR@\"

bin_PROGRAMS = embryo embryo_cc

embryo_SOURCES = \
embryo_main.c

embryo_LDADD = $(top_builddir)/src/lib/libembryo.la
embryo_DEPENDENCIES = $(top_builddir)/src/lib/libembryo.la

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

embryo_cc_LDADD = 
embryo_cc_DEPENDENCIES = 

EXTRA_DIST = \
embryo_cc_sc5.scp \
embryo_cc_sc7.scp
