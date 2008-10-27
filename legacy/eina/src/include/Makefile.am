MAINTAINERCLEANFILES = Makefile.in

EINAHEADERS = \
eina_error.h \
eina_f16p16.h \
eina_hash.h \
eina_inline_hash.x \
eina_lalloc.h \
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
eina_inline_list.x \
eina_accessor.h \
eina_convert.h \
eina_rbtree.h \
eina_benchmark.h \
eina_inline_rbtree.x \
eina_inline_mempool.x \
eina_inline_rectangle.x \
eina_iterator.h

installed_mainheaderdir = $(prefix)/include/eina-@VMAJ@
installed_mainheader_DATA = Eina.h eina_config.h

installed_headersdir = $(prefix)/include/eina-@VMAJ@/eina
installed_headers_DATA = $(EINAHEADERS)

EXTRA_DIST = \
Eina.h \
eina_config.h.in \
$(EINAHEADERS) \
eina_private.h
