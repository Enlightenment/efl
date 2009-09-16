MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = eina_amalgamation.c

AM_CPPFLAGS = \
-I$(top_srcdir)/src/include \
-I$(top_builddir)/src/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EINA_CPPFLAGS@

base_sources = \
eina_error.c \
eina_log.c \
eina_hash.c \
eina_lalloc.c \
eina_inlist.c \
eina_file.c \
eina_mempool.c \
eina_list.c \
eina_matrixsparse.c \
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
eina_rectangle.c \
eina_stringshare.c \
eina_cpu.c \
eina_tiler.c \
eina_hamster.c \
eina_safety_checks.c

DIST_SOURCES = $(base_sources)
sources_used = $(base_sources)

if EINA_STATIC_BUILD_CHAINED_POOL
sources_used += $(top_srcdir)/src/modules/mp/chained_pool/eina_chained_mempool.c
endif

if EINA_STATIC_BUILD_EMEMOA_FIXED
sources_used += $(top_srcdir)/src/modules/mp/ememoa_fixed/eina_ememoa_fixed.c
endif

if EINA_STATIC_BUILD_EMEMOA_UNKNOWN
sources_used += $(top_srcdir)/src/modules/mp/ememoa_unknown/eina_ememoa_unknown.c
endif

if EINA_STATIC_BUILD_FIXED_BITMAP
sources_used += $(top_srcdir)/src/modules/mp/fixed_bitmap/eina_fixed_bitmap.c
endif

if EINA_STATIC_BUILD_PASS_THROUGH
sources_used += $(top_srcdir)/src/modules/mp/pass_through/eina_pass_through.c
endif

lib_LTLIBRARIES = libeina.la

if EINA_AMALGAMATION
eina_sources_used = eina_amalgamation.c
BUILT_SOURCES = eina_amalgamation.c

$(builddir)/ eina_amalgamation.c: $(sources_used) Makefile
	-rm -f $(builddir)/eina_amalgamation.c

	@echo "#ifdef HAVE_CONFIG_H" >> $(builddir)/eina_amalgamation.c
	@echo "#include \"config.h\"" >> $(builddir)/eina_amalgamation.c
	@echo "#endif" >> $(builddir)/eina_amalgamation.c

	@echo "#ifndef _WIN32" >> $(builddir)/eina_amalgamation.c
	@echo "#define _GNU_SOURCE" >> $(builddir)/eina_amalgamation.c
	@echo "#endif" >> $(builddir)/eina_amalgamation.c

	@echo "#ifdef HAVE_ALLOCA_H" >> $(builddir)/eina_amalgamation.c
	@echo "# include <alloca.h>" >> $(builddir)/eina_amalgamation.c
	@echo "#elif defined __GNUC__" >> $(builddir)/eina_amalgamation.c
	@echo "# define alloca __builtin_alloca" >> $(builddir)/eina_amalgamation.c
	@echo "#elif defined _AIX" >> $(builddir)/eina_amalgamation.c
	@echo "# define alloca __alloca" >> $(builddir)/eina_amalgamation.c
	@echo "#elif defined _MSC_VER" >> $(builddir)/eina_amalgamation.c
	@echo "# include <malloc.h>" >> $(builddir)/eina_amalgamation.c
	@echo "# define alloca _alloca" >> $(builddir)/eina_amalgamation.c
	@echo "#else" >> $(builddir)/eina_amalgamation.c
	@echo "# include <stddef.h>" >> $(builddir)/eina_amalgamation.c
	@echo "# ifdef __cplusplus" >> $(builddir)/eina_amalgamation.c
	@echo "#extern \"C\"" >> $(builddir)/eina_amalgamation.c
	@echo "# endif" >> $(builddir)/eina_amalgamation.c
	@echo "#void *alloca (size_t);" >> $(builddir)/eina_amalgamation.c
	@echo "#endif" >> $(builddir)/eina_amalgamation.c

	@echo "#include <stdio.h>" >> $(builddir)/eina_amalgamation.c
	@echo "#include <stdlib.h>" >> $(builddir)/eina_amalgamation.c
	@echo "#include <string.h>" >> $(builddir)/eina_amalgamation.c
	@echo "#include <dlfcn.h>" >> $(builddir)/eina_amalgamation.c
	@echo "#include <sys/types.h>" >> $(builddir)/eina_amalgamation.c
	@echo "#include <dirent.h>" >> $(builddir)/eina_amalgamation.c

	@echo "#ifdef HAVE_EVIL" >> $(builddir)/eina_amalgamation.c
	@echo "# include <Evil.h>" >> $(builddir)/eina_amalgamation.c
	@echo "#endif" >> $(builddir)/eina_amalgamation.c

	@echo "#include \"eina_config.h\"" >> $(builddir)/eina_amalgamation.c
	@echo "#include \"eina_private.h\"" >> $(builddir)/eina_amalgamation.c
	@echo "#include \"eina_safety_checks.h\"" >> $(builddir)/eina_amalgamation.c
	@echo "#include \"Eina.h\"" >> $(builddir)/eina_amalgamation.c

	@for f in $(sources_used); do \
	   if [ `expr substr $$f 1 1` != '/' ]; then \
		  file="$(srcdir)/$$f" ; \
	   else \
		  file="$$f" ; \
	   fi ; \
	   echo "/* file: $$file */" >> $(builddir)/eina_amalgamation.c; \
	   grep -v -e '^# *include \+.\(config\|eina_[a-z_]\+\|Evil\|stdio\|stdlib\|string\|dlfcn\)[.]h.*' $$file >> $(builddir)/eina_amalgamation.c; \
	done
	@echo "eina_amalgamation.c generated"


else
eina_sources_used = $(sources_used)
endif

libeina_la_SOURCES = $(eina_sources_used)
libeina_la_LIBADD = @EINA_LIBS@ @dlopen_libs@
libeina_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@ @EFL_PTHREAD_LIBS@
libeina_la_CFLAGS = @EINA_CFLAGS@ @EFL_PTHREAD_CFLAGS@

clean-local:
	rm -rf *.gcno
