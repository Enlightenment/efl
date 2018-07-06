MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/src/Makefile_Eolian_Helper.am
include $(top_srcdir)/src/Makefile_Eolian_Mono_Helper.am
include $(top_srcdir)/src/Makefile_Efl_Mono_MSBuild_Gen_Helper.am

# Eolian_Files_Helper
# Thus we need to override it here in order to make its pattern
# rules work properly.
_EOLIAN_GEN_DEP = ../../bin/eolian/eolian_gen${EXEEXT}

SRCS = example_numberwrapper.c
EXTRA_FILES = example_numberwrapper.eo
EXTRA_PROGRAMS =
GENERATED =
CLEANFILES =

if HAVE_CSHARP

# Simple object used by the examples

EOLIAN_FLAGS := $(EOLIAN_FLAGS)
EOLIAN_FLAGS += \
-I$(srcdir) \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/efl

COMMON_FLAGS = \
@EINA_CFLAGS@ @EINA_LDFLAGS@ \
-L$(top_builddir)/src/lib/eina/.libs -leina \
@EO_CFLAGS@ @EO_LDFLAGS@ \
-L$(top_builddir)/src/lib/eo/.libs -leo \
@EFL_CFLAGS@ @EFL_LDFLAGS@ \
-L$(top_builddir)/src/lib/efl/.libs -lefl

if HAVE_WIN32
COMMON_FLAGS += \
@EVIL_CFLAGS@ @EVIL_LDFLAGS@ \
@EFL_WINDOWS_VERSION_CFLAGS@ \
-L$(top_builddir)/src/lib/evil/.libs -levil
else
COMMON_FLAGS += \
-fPIC -DPIC
endif

example_numberwrapper.c: example_numberwrapper.eo.h example_numberwrapper.eo.c

noinst_lib_LTLIBRARIES = libexample_numberwrapper.la
noinst_libdir = $(abs_top_builddir)

libexample_numberwrapper_la_SOURCES = example_numberwrapper.c
libexample_numberwrapper_la_LDFLAGS = -shared @EFL_LTMODULE_FLAGS@
libexample_numberwrapper_la_CFLAGS = $(COMMON_FLAGS) -DEFL_BETA_API_SUPPORT

GENERATED += example_numberwrapper.eo.h example_numberwrapper.eo.c EoInherit01$(EXEEXT).config

# CSharp examples

EXTRA_PROGRAMS += \
EoInherit01 \
FunctionPointer01

example_numberwrapper.eo.cs: example_numberwrapper.eo
	$(AM_V_EOLMONO) \
	$(EOLIAN_MONO) $(EOLIAN_FLAGS) $(EOLIAN_MONO_FLAGS) --dllimport example_numberwrapper -o $@ -r $(top_builddir)/src/bindings/mono/efl_mono/efl_libs.csv $<

EoInherit01_SOURCES = EoInherit01.cs
#EoInherit01_files = $(EoInherit01_exe_srcs) example_numberwrapper.eo.cs
EoInherit01$(EXEEXT): $(EoInherit01_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll libexample_numberwrapper.la EoInherit01$(EXEEXT).config example_numberwrapper.eo.cs
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

EoInherit01.csproj: $(EoInherit01_SOURCES) $(am_dirstamp) libexample_numberwrapper.la
	@rm -f $@
	$(EFL_MONO_MSBUILD_GEN) $(MSBUILD_GEN_FLAGS) -o $@ -a EoInherit01$(EXEEXT) -r libefl_mono.dll -t exe $(patsubst %.cs,src/examples/efl_mono/%.cs,$(filter %.cs, $(^)))

EoInherit01$(EXEEXT).config:
	echo "<configuration>" > $@
	echo "  <dllmap dll=\"example_numberwrapper\" target=\"$(abs_top_builddir)/src/examples/efl_mono/.libs/libexample_numberwrapper.so\"/>" >> $@
	echo "  <dllmap dll=\"efl_mono\" target=\"$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll\"/>" >> $@
	echo "  <runtime>" >> $@
	echo "    <assemblyBinding xmlns=\"urn:schemas-microsoft-com:asm.v1\">" >> $@
	echo "      <probing privatePath=\"$(abs_top_builddir)/src/lib/efl_mono\" />" >> $@
	echo "    </assemblyBinding>" >> $@
	echo "  </runtime>" >> $@
	echo "</configuration>" >> $@

FunctionPointer01_SOURCES = FunctionPointer01.cs
FunctionPointer01$(EXEEXT): $(FunctionPointer01_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll libexample_numberwrapper.la example_numberwrapper.eo.cs
	@rm -f $@
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

FunctionPointer01.csproj: $(FunctionPointer01_SOURCES) $(am_dirstamp) libexample_numberwrapper.la
	@rm -f $@
	$(EFL_MONO_MSBUILD_GEN) $(MSBUILD_GEN_FLAGS) -o $@ -a FunctionPointer01$(EXEEXT) -r libefl_mono.dll -t exe $(patsubst %.cs,src/examples/efl_mono/%.cs,$(filter %.cs, $(^)))

msbuildcsprojs: EoInherit01.csproj $(abs_top_builddir)/FunctionPointer01.csproj

GENERATED += example_numberwrapper.eo.cs


# Finishing
CLEANFILES += $(GENERATED)
endif

EXTRA_DIST = $(EXTRA_FILES)

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS) $(GENERATED)

install-examples:
	$(MKDIR_P) $(DESTDIR)$(datadir)/efl_mono/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(SRCS) $(EXTRA_FILES) $(DESTDIR)$(datadir)/efl_mono/examples

uninstall-local:
	for f in $(SRCS) $(EXTRA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/efl_mono/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif

