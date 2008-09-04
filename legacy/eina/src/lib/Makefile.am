MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EINA_CFLAGS@

lib_LTLIBRARIES = libeina.la

libeina_la_SOURCES = \
eina_error.c \
eina_hash.c \
eina_lalloc.c \
eina_inlist.c \
eina_file.c \
eina_mempool.c \
eina_list.c \
eina_module.c \
eina_value.c \
eina_array.c \
eina_magic.c \
eina_main.c \
eina_counter.c \
eina_iterator.c \
eina_accessor.c \
eina_convert.c \
eina_rbtree.c \
eina_benchmark.c \
eina_stringshare.c

libeina_la_LIBADD = @EINA_LIBS@
libeina_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@

clean-local:
	rm -rf *.gcno
