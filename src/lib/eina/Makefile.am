
### Library

lib_LTLIBRARIES = libeina.la

installed_einamainheadersdir = $(includedir)/eina-@VMAJ@
dist_installed_einamainheaders_DATA = \
Eina.h \
eina_config.h

installed_einaheadersdir = $(includedir)/eina-@VMAJ@/eina
dist_installed_einaheaders_DATA = \
eina_safety_checks.h \
eina_error.h \
eina_log.h \
eina_inline_log.x \
eina_fp.h \
eina_inline_f32p32.x \
eina_inline_f16p16.x \
eina_inline_f8p24.x \
eina_inline_fp.x \
eina_hash.h \
eina_inline_hash.x \
eina_lalloc.h \
eina_clist.h \
eina_inline_clist.x \
eina_inarray.h \
eina_inlist.h \
eina_inline_inlist.x \
eina_list.h \
eina_file.h \
eina_inline_file.x \
eina_mempool.h \
eina_module.h \
eina_rectangle.h \
eina_types.h \
eina_array.h \
eina_counter.h \
eina_inline_array.x \
eina_magic.h \
eina_stringshare.h \
eina_binshare.h \
eina_binbuf.h \
eina_ustringshare.h \
eina_inline_stringshare.x \
eina_inline_ustringshare.x \
eina_inline_list.x \
eina_accessor.h \
eina_convert.h \
eina_rbtree.h \
eina_benchmark.h \
eina_inline_rbtree.x \
eina_inline_mempool.x \
eina_inline_rectangle.x \
eina_inline_trash.x \
eina_thread.h \
eina_trash.h \
eina_iterator.h \
eina_main.h \
eina_cpu.h \
eina_inline_cpu.x \
eina_sched.h \
eina_tiler.h \
eina_hamster.h \
eina_matrixsparse.h \
eina_inline_tiler.x \
eina_str.h \
eina_inline_str.x \
eina_strbuf.h \
eina_ustrbuf.h \
eina_unicode.h \
eina_quadtree.h \
eina_simple_xml_parser.h \
eina_lock.h \
eina_prefix.h \
eina_refcount.h \
eina_mmap.h \
eina_xattr.h \
eina_value.h \
eina_inline_value.x \
eina_value_util.h \
eina_inline_value_util.x \
eina_inline_lock_barrier.x \
eina_inline_lock_posix.x \
eina_tmpstr.h \
eina_alloca.h \
eina_cow.h \
eina_inline_unicode.x \
eina_thread_queue.h \
eina_matrix.h \
eina_quad.h \
eina_crc.h \
eina_inline_crc.x \
eina_evlog.h \
eina_util.h \
eina_quaternion.h \
eina_vector.h \
eina_inline_vector.x \
eina_promise.h \
eina_bezier.h

libeina_la_SOURCES = \
eina_abi.c \
eina_accessor.c \
eina_array.c \
eina_benchmark.c \
eina_binbuf.c \
eina_binshare.c \
eina_convert.c \
eina_counter.c \
eina_cow.c \
eina_cpu.c \
eina_crc.c \
eina_debug.c \
eina_debug_bt.c \
eina_debug_bt_file.c \
eina_debug_chunk.c \
eina_debug_monitor.c \
eina_debug_proto.c \
eina_debug_thread.c \
eina_error.c \
eina_evlog.c \
eina_file_common.h \
eina_file_common.c \
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
eina_matrix.c \
eina_matrixsparse.c \
eina_mempool.c \
eina_mmap.c \
eina_module.c \
eina_prefix.c \
eina_quad.c \
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
eina_thread.c \
eina_thread_queue.c \
eina_tiler.c \
eina_tmpstr.c \
eina_unicode.c \
eina_ustrbuf.c \
eina_ustringshare.c \
eina_util.c \
eina_value.c \
eina_value_util.c \
eina_xattr.c \
eina_debug.h \
eina_private.h \
eina_share_common.h \
eina_strbuf_common.h \
eina_quaternion.c \
eina_promise.c \
eina_bezier.c

if HAVE_WIN32
libeina_la_SOURCES += eina_file_win32.c
else
libeina_la_SOURCES += eina_file.c
endif

EINA_MODULE_COMMON_CFLAGS = \
@EINA_CFLAGS@ \
@VALGRIND_CFLAGS@

if EINA_STATIC_BUILD_CHAINED_POOL
libeina_la_SOURCES += ../../modules/eina/mp/chained_pool/eina_chained_mempool.c
endif

if EINA_STATIC_BUILD_ONE_BIG
libeina_la_SOURCES += ../../modules/eina/mp/one_big/eina_one_big.c
endif

if EINA_STATIC_BUILD_PASS_THROUGH
libeina_la_SOURCES += ../../modules/eina/mp/pass_through/eina_pass_through.c
endif

libeina_la_CPPFLAGS = -I$(top_builddir)/src/lib/efl \
@EINA_CFLAGS@ \
@UNWIND_CFLAGS@ \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/eina\" \
@VALGRIND_CFLAGS@

libeina_la_LIBADD = @EINA_SUBBUILD_LIBS@ @UNWIND_LIBS@
libeina_la_DEPENDENCIES = @EINA_SUBBUILD_INTERNAL_LIBS@
libeina_la_LDFLAGS = @EFL_LTLIBRARY_FLAGS@
