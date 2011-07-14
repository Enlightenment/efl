MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EINA_CFLAGS@

noinst_PROGRAMS=makekeys

makekeys_SOURCES = \
  makekeys.c

makekeys_LDADD = \
  @EINA_LIBS@

EXTRA_DIST = mkks.sh
