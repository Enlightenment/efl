include $(top_srcdir)/src/Makefile_Eolian_Mono_Helper.am

EFL_MONO_SRCS = efl_mono/example_numberwrapper.c
EFL_MONO_DATA_FILES = efl_mono/example_numberwrapper.eo
EFL_MONO_GENERATED =

if HAVE_CSHARP

# Simple object used by the examples

COMMON_FLAGS = \
@EINA_CFLAGS@ @EINA_LDFLAGS@ \
-L$(top_builddir)/src/lib/eina/.libs -leina \
@EO_CFLAGS@ @EO_LDFLAGS@ \
-L$(top_builddir)/src/lib/eo/.libs -leo \
@EFL_CFLAGS@ @EFL_LDFLAGS@ \
-L$(top_builddir)/src/lib/efl/.libs -lefl \
-I$(abs_top_builddir)/src/examples/efl_mono

if HAVE_WIN32
COMMON_FLAGS += \
@EVIL_CFLAGS@ @EVIL_LDFLAGS@ \
@EFL_WINDOWS_VERSION_CFLAGS@ \
-L$(top_builddir)/src/lib/evil/.libs -levil
else
COMMON_FLAGS += \
-fPIC -DPIC
endif

efl_mono/example_numberwrapper.c: efl_mono/example_numberwrapper.eo.h efl_mono/example_numberwrapper.eo.c

noinst_lib_LTLIBRARIES = efl_mono/libexample_numberwrapper.la
noinst_libdir = $(abs_top_builddir)

efl_mono_libexample_numberwrapper_la_SOURCES = efl_mono/example_numberwrapper.c
efl_mono_libexample_numberwrapper_la_LDFLAGS = -shared @EFL_LTMODULE_FLAGS@
efl_mono_libexample_numberwrapper_la_CFLAGS = $(COMMON_FLAGS) -DEFL_BETA_API_SUPPORT

EFL_MONO_GENERATED += efl_mono/example_numberwrapper.eo.h efl_mono/example_numberwrapper.eo.c efl_mono/EoInherit01$(EXEEXT).config

# CSharp examples

EXTRA_PROGRAMS += \
efl_mono/EoInherit01 \
efl_mono/FunctionPointer01

efl_mono/example_numberwrapper.eo.cs: efl_mono/example_numberwrapper.eo
	$(AM_V_EOLMONO) \
	$(EOLIAN_MONO) $(EOLIAN_FLAGS) $(EOLIAN_MONO_FLAGS) --dllimport efl_mono/example_numberwrapper -o $@ -r $(top_builddir)/src/bindings/mono/efl_mono/efl_libs.csv $<

efl_mono_EoInherit01_SOURCES = efl_mono/EoInherit01.cs
#efl_mono_EoInherit01_files = $(EoInherit01_exe_srcs) efl_mono/example_numberwrapper.eo.cs
efl_mono/EoInherit01$(EXEEXT): $(efl_mono_EoInherit01_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll efl_mono/libexample_numberwrapper.la efl_mono/EoInherit01$(EXEEXT).config efl_mono/example_numberwrapper.eo.cs
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

efl_mono/EoInherit01.csproj: $(efl_mono_EoInherit01_SOURCES) $(am_dirstamp) efl_mono/libexample_numberwrapper.la
	@rm -f $@
	$(EFL_MONO_MSBUILD_GEN) $(MSBUILD_GEN_FLAGS) -o $@ -a efl_mono/EoInherit01$(EXEEXT) -r libefl_mono.dll -t exe $(patsubst %.cs,src/examples/efl_mono/%.cs,$(filter %.cs, $(^)))

efl_mono/EoInherit01$(EXEEXT).config:
	echo "<configuration>" > $@
	echo "  <dllmap dll=\"example_numberwrapper\" target=\"$(abs_top_builddir)/src/examples/efl_mono/.libs/libexample_numberwrapper.so\"/>" >> $@
	echo "  <dllmap dll=\"efl_mono\" target=\"$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll\"/>" >> $@
	echo "  <runtime>" >> $@
	echo "    <assemblyBinding xmlns=\"urn:schemas-microsoft-com:asm.v1\">" >> $@
	echo "      <probing privatePath=\"$(abs_top_builddir)/src/lib/efl_mono\" />" >> $@
	echo "    </assemblyBinding>" >> $@
	echo "  </runtime>" >> $@
	echo "</configuration>" >> $@

efl_mono_FunctionPointer01_SOURCES = efl_mono/FunctionPointer01.cs
efl_mono/FunctionPointer01$(EXEEXT): $(efl_mono_FunctionPointer01_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll efl_mono/libexample_numberwrapper.la efl_mono/example_numberwrapper.eo.cs
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

efl_mono/FunctionPointer01.csproj: $(efl_mono_FunctionPointer01_SOURCES) $(am_dirstamp) efl_mono/libexample_numberwrapper.la
	@rm -f $@
	$(EFL_MONO_MSBUILD_GEN) $(MSBUILD_GEN_FLAGS) -o $@ -a efl_mono/FunctionPointer01$(EXEEXT) -r libefl_mono.dll -t exe $(patsubst %.cs,src/examples/efl_mono/%.cs,$(filter %.cs, $(^)))

msbuildcsprojs: efl_mono/EoInherit01.csproj $(abs_top_builddir)/FunctionPointer01.csproj

EFL_MONO_GENERATED += efl_mono/example_numberwrapper.eo.cs


# Finishing
CLEANFILES += $(EFL_MONO_GENERATED)
endif

DATA_FILES += $(EFL_MONO_DATA_FILES)

install-examples-eflmono:
	$(MKDIR_P) $(DESTDIR)$(datadir)/efl_mono/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(EFL_MONO_SRCS) $(EFL_MONO_DATA_FILES) $(DESTDIR)$(datadir)/efl_mono/examples

uninstall-local-eflmono:
	for f in $(EFL_MONO_SRCS) $(EFL_MONO_DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/efl_mono/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-eflmono
UNINSTALL_EXAMPLES += uninstall-local-eflmono

