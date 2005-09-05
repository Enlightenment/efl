MAINTAINERCLEANFILES = Makefile.in
EXAMPLES = \
timer_example \
event_handler_example \
args_example \
list_example \
list_destroy_example

if BUILD_ECORE_CON
CON_EXAMPLES = con_server_example con_client_example
endif

if BUILD_ECORE_CONFIG
CONFIG_EXAMPLES = config_basic_example
endif

if BUILD_ECORE_X
X_EXAMPLES = x_window_example
endif

noinst_PROGRAMS = $(EXAMPLES) $(CON_EXAMPLES) $(CONFIG_EXAMPLES) $(X_EXAMPLES)

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_config \
-I$(top_srcdir)/src/lib/ecore_x \
-I$(top_srcdir)/src/lib/ecore_con \
@EVAS_CFLAGS@

LDADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
-lm @iconv_libs@

DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la

timer_example_SOURCES         = timer_example.c
list_example_SOURCES          = list_example.c
list_destroy_example_SOURCES  = list_destroy_example.c
event_handler_example_SOURCES = event_handler_example.c
args_example_SOURCES          = args_example.c

if BUILD_ECORE_CON
con_server_example_SOURCES    = con_server_example.c
con_server_example_LDADD      = $(top_builddir)/src/lib/ecore/libecore.la \
                                $(top_builddir)/src/lib/ecore_con/libecore_con.la

con_client_example_SOURCES    = con_client_example.c
con_client_example_LDADD      = $(top_builddir)/src/lib/ecore/libecore.la \
                                $(top_builddir)/src/lib/ecore_con/libecore_con.la
endif

if BUILD_ECORE_CONFIG
config_basic_example_SOURCES  = config_basic_example.c
config_basic_example_LDADD    = $(top_builddir)/src/lib/ecore/libecore.la \
                                $(top_builddir)/src/lib/ecore_config/libecore_config.la \
                                $(top_builddir)/src/lib/ecore_ipc/libecore_ipc.la \
                                $(top_builddir)/src/lib/ecore_con/libecore_con.la
endif

if BUILD_ECORE_X
x_window_example_SOURCES      = x_window_example.c
x_window_example_LDADD        = $(top_builddir)/src/lib/ecore/libecore.la \
                                $(top_builddir)/src/lib/ecore_txt/libecore_txt.la \
                                $(top_builddir)/src/lib/ecore_job/libecore_job.la \
                                $(top_builddir)/src/lib/ecore_x/libecore_x.la
				
endif
