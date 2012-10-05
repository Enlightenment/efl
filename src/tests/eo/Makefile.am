MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
@EFL_EO_BUILD@ \
@EO_CFLAGS@ \
@CHECK_CFLAGS@

check_PROGRAMS = \
test_access \
test_composite_objects \
test_constructors \
test_function_overrides \
test_interface \
test_mixin \
test_signals

if EFL_ENABLE_TESTS
check_PROGRAMS += eo_suite
endif

test_access_SOURCES = \
access/access_inherit.c \
access/access_inherit.h \
access/access_main.c \
access/access_simple.c \
access/access_simple.h \
access/access_simple_protected.h

test_access_LDADD = $(top_builddir)/src/lib/eo/libeo.la @EO_LIBS@

test_composite_objects_SOURCES = \
composite_objects/composite_objects_comp.c \
composite_objects/composite_objects_comp.h \
composite_objects/composite_objects_main.c \
composite_objects/composite_objects_simple.c \
composite_objects/composite_objects_simple.h

test_composite_objects_LDADD = $(top_builddir)/src/lib/eo/libeo.la @EO_LIBS@

test_constructors_SOURCES = \
constructors/constructors_main.c \
constructors/constructors_mixin.c \
constructors/constructors_mixin.h \
constructors/constructors_simple.c \
constructors/constructors_simple.h \
constructors/constructors_simple2.c \
constructors/constructors_simple2.h \
constructors/constructors_simple3.c \
constructors/constructors_simple3.h \
constructors/constructors_simple4.c \
constructors/constructors_simple4.h \
constructors/constructors_simple5.c \
constructors/constructors_simple5.h \
constructors/constructors_simple6.c \
constructors/constructors_simple6.h \
constructors/constructors_simple7.c \
constructors/constructors_simple7.h

test_constructors_LDADD = $(top_builddir)/src/lib/eo/libeo.la @EO_LIBS@

if EFL_ENABLE_TESTS

eo_suite_SOURCES = \
suite/eo_test_class_simple.c \
suite/eo_test_class_simple.h \
suite/eo_suite.c \
suite/eo_suite.h \
suite/eo_test_class_errors.c \
suite/eo_test_general.c \
suite/eo_test_init.c

eo_suite_LDADD = $(top_builddir)/src/lib/eo/libeo.la @EO_LIBS@ @CHECK_LIBS@

endif

test_function_overrides_SOURCES = \
function_overrides/function_overrides_inherit.c \
function_overrides/function_overrides_inherit.h \
function_overrides/function_overrides_inherit2.c \
function_overrides/function_overrides_inherit2.h \
function_overrides/function_overrides_inherit3.c \
function_overrides/function_overrides_inherit3.h \
function_overrides/function_overrides_main.c \
function_overrides/function_overrides_simple.c \
function_overrides/function_overrides_simple.h

test_function_overrides_LDADD = $(top_builddir)/src/lib/eo/libeo.la @EO_LIBS@

test_interface_SOURCES = \
interface/interface_interface.c \
interface/interface_interface.h \
interface/interface_interface2.c \
interface/interface_interface2.h \
interface/interface_main.c \
interface/interface_simple.c \
interface/interface_simple.h

test_interface_LDADD = $(top_builddir)/src/lib/eo/libeo.la @EO_LIBS@

test_mixin_SOURCES = \
mixin/mixin_inherit.c \
mixin/mixin_inherit.h \
mixin/mixin_main.c \
mixin/mixin_mixin.c \
mixin/mixin_mixin.h \
mixin/mixin_mixin2.c \
mixin/mixin_mixin2.h \
mixin/mixin_mixin3.c \
mixin/mixin_mixin3.h \
mixin/mixin_mixin4.c \
mixin/mixin_mixin4.h \
mixin/mixin_simple.c \
mixin/mixin_simple.h

test_mixin_LDADD = $(top_builddir)/src/lib/eo/libeo.la @EO_LIBS@

test_signals_SOURCES = \
signals/signals_main.c \
signals/signals_simple.c \
signals/signals_simple.h

test_signals_LDADD = $(top_builddir)/src/lib/eo/libeo.la @EO_LIBS@

EXTRA_DIST = eunit_tests.h
