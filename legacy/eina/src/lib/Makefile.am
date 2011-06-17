MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/include \
-I$(top_builddir)/src/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EINA_CPPFLAGS@ \
@EFL_EINA_BUILD@

base_sources = \
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
eina_inlist.c \
eina_iterator.c \
eina_lalloc.c \
eina_list.c \
eina_log.c \
eina_magic.c \
eina_main.c \
eina_matrixsparse.c \
eina_mempool.c \
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
eina_value.c

# Will be back for developper after 1.1
# eina_object.c

if EINA_HAVE_WIN32
base_sources += eina_file_win32.c
else
base_sources += eina_file.c
endif

EXTRA_DIST = \
eina_share_common.h \
eina_private.h \
eina_strbuf_common.h \
eina_strbuf_template_c.x \
eina_binbuf_template_c.x



if EINA_STATIC_BUILD_BUDDY
base_sources += $(top_srcdir)/src/modules/mp/buddy/eina_buddy.c
endif

if EINA_STATIC_BUILD_CHAINED_POOL
base_sources += $(top_srcdir)/src/modules/mp/chained_pool/eina_chained_mempool.c
endif

if EINA_STATIC_BUILD_EMEMOA_FIXED
base_sources += $(top_srcdir)/src/modules/mp/ememoa_fixed/eina_ememoa_fixed.c
endif

if EINA_STATIC_BUILD_EMEMOA_UNKNOWN
base_sources += $(top_srcdir)/src/modules/mp/ememoa_unknown/eina_ememoa_unknown.c
endif

if EINA_STATIC_BUILD_FIXED_BITMAP
base_sources += $(top_srcdir)/src/modules/mp/fixed_bitmap/eina_fixed_bitmap.c
endif

if EINA_STATIC_BUILD_ONE_BIG
base_sources += $(top_srcdir)/src/modules/mp/one_big/eina_one_big.c
endif

if EINA_STATIC_BUILD_PASS_THROUGH
base_sources += $(top_srcdir)/src/modules/mp/pass_through/eina_pass_through.c
endif

lib_LTLIBRARIES = libeina.la

if EINA_AMALGAMATION
nodist_libeina_la_SOURCES = eina_amalgamation.c

eina_amalgamation.c: $(base_sources) Makefile
	-rm -f eina_amalgamation.c

	@echo "#ifdef HAVE_CONFIG_H" >> eina_amalgamation.c
	@echo "#include \"config.h\"" >> eina_amalgamation.c
	@echo "#endif" >> eina_amalgamation.c

	@echo "#ifdef HAVE_ALLOCA_H" >> eina_amalgamation.c
	@echo "# include <alloca.h>" >> eina_amalgamation.c
	@echo "#elif defined __GNUC__" >> eina_amalgamation.c
	@echo "# define alloca __builtin_alloca" >> eina_amalgamation.c
	@echo "#elif defined _AIX" >> eina_amalgamation.c
	@echo "# define alloca __alloca" >> eina_amalgamation.c
	@echo "#elif defined _MSC_VER" >> eina_amalgamation.c
	@echo "# include <malloc.h>" >> eina_amalgamation.c
	@echo "# define alloca _alloca" >> eina_amalgamation.c
	@echo "#else" >> eina_amalgamation.c
	@echo "# include <stddef.h>" >> eina_amalgamation.c
	@echo "# ifdef __cplusplus" >> eina_amalgamation.c
	@echo "#extern \"C\"" >> eina_amalgamation.c
	@echo "# endif" >> eina_amalgamation.c
	@echo "#void *alloca (size_t);" >> eina_amalgamation.c
	@echo "#endif" >> eina_amalgamation.c

	@echo "#include <stdio.h>" >> eina_amalgamation.c
	@echo "#include <stdlib.h>" >> eina_amalgamation.c
	@echo "#include <string.h>" >> eina_amalgamation.c
	@echo "#include <dlfcn.h>" >> eina_amalgamation.c
	@echo "#include <sys/types.h>" >> eina_amalgamation.c
	@echo "#include <dirent.h>" >> eina_amalgamation.c
	@echo "#include <assert.h>" >> eina_amalgamation.c
	@echo "#include <errno.h>" >> eina_amalgamation.c
	@echo "#include <fnmatch.h>" >> eina_amalgamation.c

	@echo "#ifdef HAVE_EVIL" >> eina_amalgamation.c
	@echo "# include <Evil.h>" >> eina_amalgamation.c
	@echo "#endif" >> eina_amalgamation.c

	@echo "#include \"eina_config.h\"" >> eina_amalgamation.c
	@echo "#include \"eina_private.h\"" >> eina_amalgamation.c
	@echo "#include \"eina_safety_checks.h\"" >> eina_amalgamation.c
	@echo "#include \"Eina.h\"" >> eina_amalgamation.c
	@echo "#include \"eina_strbuf_common.h\"" >> eina_amalgamation.c
	@echo "#include \"eina_share_common.h\"" >> eina_amalgamation.c

	@for f in $(base_sources); do \
	   if [ `echo $$f | sed -e 's/^...\(.\).*/\1/'` != '/' ]; then \
		  file="$(srcdir)/$$f" ; \
	   else \
		  file="$$f" ; \
	   fi ; \
	   echo "/* file: $$file */" >> eina_amalgamation.c; \
	   grep -v -e '^# *include \+.\(config\|eina_[a-z_]\+\|Evil\|stdio\|stdlib\|string\|dlfcn\|dirent\|assert\|errno\|fnmatch\)[.]h.*' $$file >> eina_amalgamation.c; \
	done
	@echo "eina_amalgamation.c generated"

else
libeina_la_SOURCES = $(base_sources)
endif

libeina_la_LIBADD = @EINA_LIBS@ @dlopen_libs@
libeina_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@ @EFL_PTHREAD_LIBS@
libeina_la_CFLAGS = @EINA_CFLAGS@ @EFL_PTHREAD_CFLAGS@

clean-local:
	rm -rf *.gcno eina_amalgamation.c
