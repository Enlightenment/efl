MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
@EVIL_CFLAGS@ \
@EINA_CFLAGS@

AM_CFLAGS = @WIN32_CFLAGS@

if BUILD_ECORE_JOB

lib_LTLIBRARIES = libecore_job.la
include_HEADERS = \
Ecore_Job.h

libecore_job_la_SOURCES = \
ecore_job.c

libecore_job_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
@EINA_LIBS@ \
@EVIL_LIBS@

libecore_job_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@

endif

EXTRA_DIST = ecore_job_private.h
