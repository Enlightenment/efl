MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-I. -I$(top_srcdir)/src/lib @ECORE_CFLAGS@

if HAVE_ECORE_DESKTOP
bin_PROGRAMS = efreet_alloc ecore_alloc compare_results efreet_menu_alloc
else
bin_PROGRAMS = efreet_alloc efreet_menu_alloc
endif

efreet_menu_alloc_DEPENDENCIES = $(top_builddir)/src/lib/libefreet.la
efreet_menu_alloc_SOURCES = efreet_menu_alloc.c comp.h
efreet_menu_alloc_LDADD = $(top_builddir)/src/lib/libefreet.la @ECORE_LIBS@

efreet_alloc_DEPENDENCIES = $(top_builddir)/src/lib/libefreet.la
efreet_alloc_SOURCES = efreet_alloc.c comp.h
efreet_alloc_LDADD = $(top_builddir)/src/lib/libefreet.la @ECORE_LIBS@

if HAVE_ECORE_DESKTOP
ecore_alloc_DEPENDENCIES = 
ecore_alloc_SOURCES = ecore_alloc.c comp.h
ecore_alloc_LDADD = @ECORE_LIBS@

compare_results_DEPENDENCIES = $(top_builddir)/src/lib/libefreet.la
compare_results_SOURCES = compare_results.c comp.h
compare_results_LDADD = $(top_builddir)/src/lib/libefreet.la @ECORE_LIBS@
endif

