MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
@EINA_CFLAGS@

AM_CFLAGS = @WIN32_CFLAGS@

if BUILD_ECORE_JOB

lib_LTLIBRARIES = libecore_job.la
include_HEADERS = \
Ecore_Job.h

libecore_job_la_SOURCES = \
ecore_job.c \
ecore_job_private.h

libecore_job_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
@EINA_LIBS@

libecore_job_la_LDFLAGS = @lt_no_undefined@ @lt_enable_auto_import@ -version-info @version_info@

libecore_job_la_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la

endif

EXTRA_DIST = \
Ecore_Job.h \
ecore_job.c \
ecore_job_private.h
