MAINTAINERCLEANFILES = Makefile.in

EINAHEADERS = \
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
eina_inlist.h \
eina_list.h \
eina_file.h \
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
eina_trash.h \
eina_iterator.h \
eina_main.h \
eina_cpu.h \
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
eina_xattr.h

# Will be back for developper after 1.1.
# eina_object.h

if EINA_HAVE_THREADS
if EINA_HAVE_WINCE
EINAHEADERS += eina_inline_lock_wince.x
else
if EINA_HAVE_WIN32
EINAHEADERS += eina_inline_lock_win32.x
else
EINAHEADERS += eina_inline_lock_posix.x
endif
endif
else
EINAHEADERS += eina_inline_lock_void.x
endif

installed_mainheaderdir = $(includedir)/eina-@VMAJ@
dist_installed_mainheader_DATA = Eina.h eina_config.h

installed_headersdir = $(includedir)/eina-@VMAJ@/eina
dist_installed_headers_DATA = $(EINAHEADERS)

EXTRA_DIST = \
eina_config.h.in
