MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/include/eina \
-I$(top_builddir)/src/include/eina \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EFL_EINA_BUILD@ \
@EFL_PTHREAD_CFLAGS@ \
@EINA_CFLAGS@ \
@VALGRIND_CFLAGS@

lib_LTLIBRARIES = libeina.la

libeina_la_SOURCES = \
eina_accessor.c \
eina_array.c \
eina_benchmark.c \
eina_binbuf.c \
eina_binshare.c \
eina_convert.c \
eina_counter.c \
eina_cpu.c \
eina_error.c \
eina_fp.c \
eina_hamster.c \
eina_hash.c \
eina_inarray.c \
eina_inlist.c \
eina_iterator.c \
eina_lalloc.c \
eina_list.c \
eina_log.c \
eina_magic.c \
eina_main.c \
eina_matrixsparse.c \
eina_mempool.c \
eina_mmap.c \
eina_module.c \
eina_prefix.c \
eina_quadtree.c \
eina_rbtree.c \
eina_rectangle.c \
eina_safety_checks.c \
eina_sched.c \
eina_share_common.c \
eina_simple_xml_parser.c \
eina_str.c \
eina_strbuf.c \
eina_strbuf_common.c \
eina_stringshare.c \
eina_tiler.c \
eina_unicode.c \
eina_ustrbuf.c \
eina_ustringshare.c \
eina_value.c \
eina_xattr.c \
eina_share_common.h \
eina_private.h \
eina_strbuf_common.h

# Will be back for developper after 1.2
# eina_model.c \
# eina_object.c

if HAVE_WIN32
libeina_la_SOURCES += eina_file_win32.c
else
libeina_la_SOURCES += eina_file.c
endif

EXTRA_DIST = \
eina_strbuf_template_c.x \
eina_binbuf_template_c.x



if EINA_STATIC_BUILD_BUDDY
libeina_la_SOURCES += $(top_srcdir)/src/modules/eina/mp/buddy/eina_buddy.c
endif

if EINA_STATIC_BUILD_CHAINED_POOL
libeina_la_SOURCES += $(top_srcdir)/src/modules/eina/mp/chained_pool/eina_chained_mempool.c
endif

if EINA_STATIC_BUILD_EMEMOA_FIXED
libeina_la_SOURCES += $(top_srcdir)/src/modules/eina/mp/ememoa_fixed/eina_ememoa_fixed.c
endif

if EINA_STATIC_BUILD_EMEMOA_UNKNOWN
libeina_la_SOURCES += $(top_srcdir)/src/modules/eina/mp/ememoa_unknown/eina_ememoa_unknown.c
endif

if EINA_STATIC_BUILD_FIXED_BITMAP
libeina_la_SOURCES += $(top_srcdir)/src/modules/eina/mp/fixed_bitmap/eina_fixed_bitmap.c
endif

if EINA_STATIC_BUILD_ONE_BIG
libeina_la_SOURCES += $(top_srcdir)/src/modules/eina/mp/one_big/eina_one_big.c
endif

if EINA_STATIC_BUILD_PASS_THROUGH
libeina_la_SOURCES += $(top_srcdir)/src/modules/eina/mp/pass_through/eina_pass_through.c
endif

libeina_la_LIBADD = @EINA_LIBS@ -lm
libeina_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@ @EFL_PTHREAD_LIBS@

clean-local:
	rm -rf *.gcno
