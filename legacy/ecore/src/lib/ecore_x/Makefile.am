## Process this file with automake to produce Makefile.in

INCLUDES = \
@Xcursor_cflags@ \
@x_cflags@ \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_txt \
-I$(top_srcdir)/src/lib/ecore_job \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_txt \
-I$(top_builddir)/src/lib/ecore_job

libecore_x_la_LDFLAGS = -version-info 1:0:0 \
-L$(top_builddir)/src/lib/ecore/.libs \
-L$(top_builddir)/src/lib/ecore_txt/.libs \
-L$(top_builddir)/src/lib/ecore_job/.libs

XCURSOR_LIBS=@Xcursor_libs@

if BUILD_ECORE_X

lib_LTLIBRARIES = libecore_x.la
include_HEADERS = \
Ecore_X.h \
Ecore_X_Atoms.h \
Ecore_X_Cursor.h

libecore_x_la_SOURCES = \
ecore_x.c \
ecore_x_dnd.c \
ecore_x_error.c \
ecore_x_events.c \
ecore_x_icccm.c \
ecore_x_netwm.c \
ecore_x_mwm.c \
ecore_x_selection.c \
ecore_x_window.c \
ecore_x_window_prop.c \
ecore_x_window_shape.c \
ecore_x_pixmap.c \
ecore_x_gc.c \
ecore_x_private.h

### FUCK FUCK FIXME:
# This line:
###@Xcursor_libs@ \
# has been replaced by
###-lXcursor
# because -f or some unknonw reason libtool REFUSES to put -lXcursor into the
# .la file for libecore_x in the dependency libs UNLESS this is explicitly
# states and not used as a replace term. if someone knows why... let me know
# and send patches... but i'm fresh out of ideas.
#
# After trying to get this working for the last 2+ hours... i only have one
# thing to say... FUCK YOU LIBSPLEEN! FUCK YOU!
#
# GRRRR... (this is just an example of what shits developers off about auto
# foo - somehting that should obviously "just work" and if i make my own
# Makefiles it works. its happy. but once you go through the 368 layers of
# autofuck things mysteriously stop working and if you're lucky you may just
# run across a workaround as I have here... ARGH!)

libecore_x_la_LIBADD = \
-lXcursor \
@x_ldflags@ \
@x_libs@ \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_txt/libecore_txt.la \
$(top_builddir)/src/lib/ecore_job/libecore_job.la

libecore_x_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_txt/libecore_txt.la \
$(top_builddir)/src/lib/ecore_job/libecore_job.la

endif

EXTRA_DIST = \
Ecore_X.h \
Ecore_X_Atoms.h \
Ecore_X_Cursor.h \
ecore_x.c \
ecore_x_dnd.c \
ecore_x_error.c \
ecore_x_events.c \
ecore_x_icccm.c \
ecore_x_netwm.c \
ecore_x_selection.c \
ecore_x_window.c \
ecore_x_window_prop.c \
ecore_x_window_shape.c \
ecore_x_pixmap.c \
ecore_x_gc.c \
ecore_x_private.h
