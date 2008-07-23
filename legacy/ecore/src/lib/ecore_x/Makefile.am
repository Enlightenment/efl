MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = xlib xcb

if BUILD_ECORE_X

if BUILD_ECORE_X_XCB
DEP = xcb/libecore_x_xcb.la
else
DEP = xlib/libecore_x_xlib.la
endif

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_txt \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_txt

lib_LTLIBRARIES = libecore_x.la

libecore_x_la_SOURCES =

libecore_x_la_LIBADD = $(DEP)

libecore_x_la_LDFLAGS = -version-info @version_info@

libecore_x_la_DEPENDENCIES = $(DEP)

include_HEADERS = \
Ecore_X.h \
Ecore_X_Atoms.h \
Ecore_X_Cursor.h

endif

EXTRA_DIST = ecore_x_atoms_decl.h
