## Process this file with automake to produce Makefile.in

if BUILD_ECORE_X
ECORE_X_INC = -I$(top_srcdir)/src/lib/ecore_x
ECORE_X_LIB = $(top_builddir)/src/lib/ecore_x/libecore_x.la
ECORE_X_LDF = -L$(top_builddir)/src/lib/ecore_x/.libs
else
ECORE_X_INC =
ECORE_X_LIB = 
ECORE_X_LDF = 
endif

if BUILD_ECORE_FB
ECORE_FB_INC = -I$(top_srcdir)/src/lib/ecore_fb
ECORE_FB_LIB = $(top_builddir)/src/lib/ecore_fb/libecore_fb.la
ECORE_FB_LDF = -L$(top_builddir)/src/lib/ecore_fb/.libs
else
ECORE_FB_INC =
ECORE_FB_LIB =
ECORE_FB_LDF = 
endif

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore \
$(ECORE_X_INC) \
$(ECORE_FB_INC) \
@evas_cflags@ 

libecore_evas_la_LDFLAGS = -version-info 1:0:0 \
$(ECORE_X_LDF) \
$(ECORE_FB_LDF) \
-L$(top_builddir)/src/lib/ecore/.libs

if BUILD_ECORE_EVAS

lib_LTLIBRARIES = libecore_evas.la
include_HEADERS = \
Ecore_Evas.h

libecore_evas_la_SOURCES = \
ecore_evas.c \
ecore_evas_x.c \
ecore_evas_fb.c \
ecore_evas_private.h

libecore_evas_la_LIBADD = \
$(ECORE_X_LIB) \
$(ECORE_FB_LIB) \
$(top_builddir)/src/lib/ecore/libecore.la \
@evas_libs@

libecore_evas_la_DEPENDENCIES = \
$(ECORE_X_LIB) \
$(ECORE_FB_LIB) \
$(top_builddir)/src/lib/ecore/libecore.la

endif

EXTRA_DIST = \
Ecore_Evas.h \
ecore_evas.c \
ecore_evas_x.c \
ecore_evas_fb.c \
ecore_evas_private.h

