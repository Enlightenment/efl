## Process this file with automake to produce Makefile.in

INCLUDES = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
@my_cflags@ @edje_cflags@

bin_PROGRAMS = \
emotion_test

emotion_test_SOURCES = \
emotion_test_main.c

emotion_test_LDADD = \
@my_libs@ @edje_libs@ \
$(top_builddir)/src/lib/libemotion.la


