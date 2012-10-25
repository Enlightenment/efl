MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib/eina \
-I$(top_builddir)/src/lib/eina \
-DEFL_EINA_BUILD \
@VALGRIND_CFLAGS@

controllerdir = $(libdir)/eina/modules/mp/one_big/$(MODULE_ARCH)
controller_LTLIBRARIES = module.la

module_la_SOURCES = eina_one_big.c

module_la_CFLAGS = @EINA_CFLAGS@ @EFL_PTHREAD_CFLAGS@
module_la_LIBADD = $(top_builddir)/src/lib/eina/libeina.la @EINA_LIBS@
module_la_LDFLAGS = -no-undefined -module -avoid-version @EFL_PTHREAD_LIBS@
module_la_LIBTOOLFLAGS = --tag=disable-static

install-data-hook:
	rm -f $(DESTDIR)$(controllerdir)/$(controller_LTLIBRARIES)
uninstall-hook:
	rm -f $(DESTDIR)$(controllerdir)/module.*

clean-local:
	rm -rf *.gcno
