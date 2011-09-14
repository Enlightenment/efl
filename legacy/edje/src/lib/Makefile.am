
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir) \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EVIL_CFLAGS@ \
@EDJE_CFLAGS@ \
@ECORE_IMF_CFLAGS@ \
@EFL_EDJE_BUILD@

lib_LTLIBRARIES = libedje.la

includes_HEADERS = Edje.h Edje_Edit.h
includesdir = $(includedir)/edje-@VMAJ@

base_sources = \
edje_box_layout.c \
edje_cache.c \
edje_calc.c \
edje_callbacks.c \
edje_container.c \
edje_data.c \
edje_edit.c \
edje_embryo.c \
edje_entry.c \
edje_external.c \
edje_load.c \
edje_lua.c \
edje_lua2.c \
edje_lua_script_only.c \
edje_main.c \
edje_match.c \
edje_message_queue.c \
edje_misc.c \
edje_module.c \
edje_program.c \
edje_script_only.c \
edje_smart.c \
edje_text.c \
edje_textblock_styles.c \
edje_util.c \
edje_var.c

if EDJE_AMALGAMATION
nodist_libedje_la_SOURCES = edje_amalgamation.c

edje_amalgamation.c: $(base_sources) Makefile
	-rm -f edje_amalgamation.c

	@echo "#ifdef HAVE_CONFIG_H" >> edje_amalgamation.c
	@echo "# include \"config.h\"" >> edje_amalgamation.c
	@echo "#endif" >> edje_amalgamation.c

	@echo "#ifndef _WIN32" >> edje_amalgamation.c
	@echo "#define _GNU_SOURCE" >> edje_amalgamation.c
	@echo "#endif" >> edje_amalgamation.c

	@echo "#ifdef HAVE_ALLOCA_H" >> edje_amalgamation.c
	@echo "# include <alloca.h>" >> edje_amalgamation.c
	@echo "#elif defined __GNUC__" >> edje_amalgamation.c
	@echo "# define alloca __builtin_alloca" >> edje_amalgamation.c
	@echo "#elif defined _AIX" >> edje_amalgamation.c
	@echo "# define alloca __alloca" >> edje_amalgamation.c
	@echo "#elif defined _MSC_VER" >> edje_amalgamation.c
	@echo "# include <malloc.h>" >> edje_amalgamation.c
	@echo "# define alloca _alloca" >> edje_amalgamation.c
	@echo "#else" >> edje_amalgamation.c
	@echo "# include <stddef.h>" >> edje_amalgamation.c
	@echo "# ifdef __cplusplus" >> edje_amalgamation.c
	@echo "#extern \"C\"" >> edje_amalgamation.c
	@echo "# endif" >> edje_amalgamation.c
	@echo "#void *alloca (size_t);" >> edje_amalgamation.c
	@echo "#endif" >> edje_amalgamation.c

	@echo "#include <string.h>" >> edje_amalgamation.c
	@echo "#include <math.h>" >> edje_amalgamation.c
	@echo "#include <time.h>" >> edje_amalgamation.c
	@echo "#include <limits.h>" >> edje_amalgamation.c
	@echo "#include <errno.h>" >> edje_amalgamation.c
	@echo "#include <sys/stat.h>" >> edje_amalgamation.c

	@echo "#ifndef _MSC_VER" >> edje_amalgamation.c
	@echo "# include <unistd.h>" >> edje_amalgamation.c
	@echo "#endif" >> edje_amalgamation.c

	@echo "#ifdef HAVE_LOCALE_H" >> edje_amalgamation.c
	@echo "# include <locale.h>" >> edje_amalgamation.c
	@echo "#endif" >> edje_amalgamation.c

	@echo "#include <lua.h>" >> edje_amalgamation.c
	@echo "#include <lauxlib.h>" >> edje_amalgamation.c

	@echo "#ifdef HAVE_EVIL" >> edje_amalgamation.c
	@echo "# include <Evil.h>" >> edje_amalgamation.c
	@echo "#endif" >> edje_amalgamation.c

	@echo "#include <Eina.h>" >> edje_amalgamation.c
	@echo "#include <Edje.h>" >> edje_amalgamation.c
	@echo "#include <Evas.h>" >> edje_amalgamation.c
	@echo "#include <Ecore.h>" >> edje_amalgamation.c
	@echo "#include <Embryo.h>" >> edje_amalgamation.c

	@echo "#ifdef HAVE_ECORE_IMF" >> edje_amalgamation.c
	@echo "# include <Ecore_IMF.h>" >> edje_amalgamation.c
	@echo "#endif" >> edje_amalgamation.c

	@echo "#include <edje_private.h>" >> edje_amalgamation.c

	@for f in $(base_sources); do \
	   if [ `echo $$f | sed -e 's/^...\(.\).*/\1/'` != '/' ]; then \
		  file="$(srcdir)/$$f" ; \
	   else \
		  file="$$f" ; \
	   fi ; \
	   echo "/* file: $$file */" >> edje_amalgamation.c; \
	   grep -v -e '^# *include \+.\(config\|\|Evil\|Eina\|Eet\|Evas\|Ecore\|Embryo\|Ecore_IMF\|string\|math\|limits\|sys/stat\|errno\|time\|unistd\|locale\|lua\|lauxlib\|edje_private\)[.]h.*' $$file >> edje_amalgamation.c; \
	done
	@echo "edje_amalgamation.c generated"

else
libedje_la_SOURCES = $(base_sources)
endif

libedje_la_LIBADD = @EDJE_LIBS@ @ECORE_IMF_LIBS@ @EVIL_LIBS@ -lm
libedje_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@

EXTRA_DIST = edje_private.h edje_container.h edje_convert.h

clean-local:
	@rm -rf *.gcno edje_amalgamation.c
