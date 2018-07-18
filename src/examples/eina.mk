EINA_SRCS = \
eina/eina_accessor_01.c \
eina/eina_array_01.c \
eina/eina_array_02.c \
eina/eina_error_01.c \
eina/eina_file_01.c \
eina/eina_file_02.c \
eina/eina_hash_01.c \
eina/eina_hash_02.c \
eina/eina_hash_03.c \
eina/eina_hash_04.c \
eina/eina_hash_05.c \
eina/eina_hash_06.c \
eina/eina_hash_07.c \
eina/eina_hash_08.c \
eina/eina_iterator_01.c \
eina/eina_list_01.c \
eina/eina_list_02.c \
eina/eina_list_03.c \
eina/eina_list_04.c \
eina/eina_log_01.c \
eina/eina_log_02.c \
eina/eina_log_03.c \
eina/eina_inlist_01.c \
eina/eina_inlist_02.c \
eina/eina_inlist_03.c \
eina/eina_str_01.c \
eina/eina_strbuf_01.c \
eina/eina_stringshare_01.c \
eina/eina_tmpstr_01.c \
eina/eina_tiler_01.c \
eina/eina_simple_xml_parser_01.c \
eina/eina_value_01.c \
eina/eina_value_02.c \
eina/eina_value_03.c \
eina/eina_inarray_01.c \
eina/eina_inarray_02.c \
eina/eina_inarray_03.c \
eina/eina_magic_01.c \
eina/eina_xattr_01.c \
eina/eina_xattr_02.c

EXTRA_PROGRAMS += \
eina/eina_accessor_01 \
eina/eina_array_01 \
eina/eina_array_02 \
eina/eina_error_01 \
eina/eina_file_01 \
eina/eina_file_02 \
eina/eina_hash_01 \
eina/eina_hash_02 \
eina/eina_hash_03 \
eina/eina_hash_04 \
eina/eina_hash_05 \
eina/eina_hash_06 \
eina/eina_hash_07 \
eina/eina_hash_08 \
eina/eina_iterator_01 \
eina/eina_list_01 \
eina/eina_list_02 \
eina/eina_list_03 \
eina/eina_list_04 \
eina/eina_log_01 \
eina/eina_log_02 \
eina/eina_log_03 \
eina/eina_inlist_01 \
eina/eina_inlist_02 \
eina/eina_inlist_03 \
eina/eina_str_01 \
eina/eina_strbuf_01 \
eina/eina_stringshare_01 \
eina/eina_tmpstr_01 \
eina/eina_magic_01 \
eina/eina_simple_xml_parser_01 \
eina/eina_value_01 \
eina/eina_value_02 \
eina/eina_value_03 \
eina/eina_inarray_01 \
eina/eina_inarray_02 \
eina/eina_inarray_03 \
eina/eina_xattr_01 \
eina/eina_xattr_02 \
eina/eina_tiler_01

if HAVE_CSHARP

EXTRA_PROGRAMS += \
eina/EinaArray01Mono \
eina/EinaBinbuf01Mono \
eina/EinaError01Mono \
eina/EinaHash01Mono

eina_EinaArray01Mono_SOURCES = eina/EinaArray01.cs
eina_EinaBinbuf01Mono_SOURCES = eina/EinaBinbuf01.cs
eina_EinaError01Mono_SOURCES = eina/EinaError01.cs
eina_EinaHash01Mono_SOURCES = eina/EinaHash01.cs

eina/EinaArray01Mono$(EXEEXT): $(eina_EinaArray01Mono_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

eina/EinaArray01.csproj: $(eina_EinaArray01Mono_SOURCES) $(am_dirstamp)
	@rm -f $@
	$(EFL_MONO_MSBUILD_GEN) $(MSBUILD_GEN_FLAGS) -o $@ -a eina/EinaArray01Mono$(EXEEXT) -r libefl_mono.dll -t exe $(patsubst %.cs,src/examples/eina/%.cs,$(filter %.cs, $(^)))

eina/EinaBinbuf01Mono$(EXEEXT): $(eina_EinaBinbuf01Mono_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

eina/EinaBinbuf01.csproj: $(eina_EinaBinbuf01Mono_SOURCES) $(am_dirstamp)
	@rm -f $@
	$(EFL_MONO_MSBUILD_GEN) $(MSBUILD_GEN_FLAGS) -o $@ -a eina/EinaBinbuf01Mono$(EXEEXT) -r libefl_mono.dll -t exe $(patsubst %.cs,src/examples/eina/%.cs,$(filter %.cs, $(^)))

eina/EinaError01Mono$(EXEEXT): $(eina_EinaError01Mono_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

eina/EinaError01.csproj: $(eina_EinaError01Mono_SOURCES) $(am_dirstamp)
	@rm -f $@
	$(EFL_MONO_MSBUILD_GEN) $(MSBUILD_GEN_FLAGS) -o $@ -a eina/EinaError01Mono$(EXEEXT) -r libefl_mono.dll -t exe $(patsubst %.cs,src/examples/eina/%.cs,$(filter %.cs, $(^)))

eina/EinaHash01Mono$(EXEEXT): $(eina_EinaHash01Mono_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

eina/EinaHash01.csproj: $(eina_EinaHash01Mono_SOURCES) $(am_dirstamp)
	@rm -f $@
	$(EFL_MONO_MSBUILD_GEN) $(MSBUILD_GEN_FLAGS) -o $@ -a eina/EinaHash01Mono$(EXEEXT) -r libefl_mono.dll -t exe $(patsubst %.cs,src/examples/eina/%.cs,$(filter %.cs, $(^)))

msbuildcsprojs: $(abs_top_builddir)/EinaArray01.csproj $(abs_top_builddir)/EinaBinbuf01.csproj $(abs_top_builddir)/EinaError01.csproj $(abs_top_builddir)/EinaHash01.csproj

endif

EINA_DATA_FILES = \
eina/addr_book.txt \
eina/chat.xml \
eina/Makefile.examples

DATA_FILES += $(EINA_DATA_FILES)

install-examples-eina:
	$(MKDIR_P) $(DESTDIR)$(datadir)/eina/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(EINA_SRCS) $(EINA_DATA_FILES) $(DESTDIR)$(datadir)/eina/examples

uninstall-local-eina:
	for f in $(EINA_SRCS) $(EINA_DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/eina/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-eina
UNINSTALL_EXAMPLES += uninstall-local-eina
