## Process this file with automake to produce Makefile.in

noinst_PROGRAMS = \
timer_example \
event_handler_example \
args_example \
config_basic_example 

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_config

LDADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_config/libecore_config.la \
-lm @iconv_libs@

DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_config/libecore_config.la

timer_example_SOURCES         = timer_example.c
event_handler_example_SOURCES = event_handler_example.c
args_example_SOURCES          = args_example.c
config_basic_example_SOURCES  = config_basic_example.c
