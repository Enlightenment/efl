## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       @FREETYPE_CFLAGS@ \
		       -DPACKAGE_BIN_DIR=\"@PACKAGE_BIN_DIR@\" \
		       -DPACKAGE_LIB_DIR=\"@PACKAGE_LIB_DIR@\" \
		       -DPACKAGE_DATA_DIR=\"@PACKAGE_DATA_DIR@\"

noinst_LTLIBRARIES      = libevas_file.la
libevas_file_la_SOURCES  = \
evas_path.c \
evas_module.c

libevas_file_la_DEPENDENCIES = $(top_builddir)/config.h
