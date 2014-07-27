MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST =
CLEANFILES =

########################################################################
# Embryo
embryofilesdir = $(datadir)/embryo/include
embryofiles_DATA = embryo/default.inc
EXTRA_DIST += $(embryofiles_DATA)

########################################################################
# Evas
evasfilesdir = $(datadir)/evas
evasfiles_DATA = evas/checkme
EXTRA_DIST += $(evasfiles_DATA)

########################################################################
# Eeze
eezefilesdir = $(datadir)/eeze
eezefiles_DATA = eeze/checkme
EXTRA_DIST += $(eezefiles_DATA)

########################################################################
# Ecore
ecorefilesdir = $(datadir)/ecore
ecorefiles_DATA = ecore/checkme
EXTRA_DIST += $(ecorefiles_DATA)

########################################################################
# Ecore_X
ecore_xfilesdir = $(datadir)/ecore_x
ecore_xfiles_DATA = ecore_x/checkme
EXTRA_DIST += $(ecore_xfiles_DATA)

########################################################################
# Ecore_Imf
ecoreimffilesdir = $(datadir)/ecore_imf
ecoreimffiles_DATA = ecore_imf/checkme
EXTRA_DIST += $(ecoreimffiles_DATA)

########################################################################
# Efreet
efreetfilesdir = $(datadir)/efreet
efreetfiles_DATA = efreet/checkme
EXTRA_DIST += $(efreetfiles_DATA)

########################################################################
# Eo
eogdbdir = $(datadir)/eo/gdb
eogdb_SCRIPTS = eo/eo_gdb.py

# Borrowed from gobject
libeo.so.@VMAJ@.@VMIN@.@VMIC@-gdb.py: eo/libeo-gdb.py.in
	$(AM_V_GEN) $(SED) -e "s|\@datadir\@|$(datadir)|" $(srcdir)/eo/libeo-gdb.py.in > $(builddir)/libeo.so.@VMAJ@.@VMIN@.@VMIC@-gdb.py

eogdbloaddir = $(datadir)/gdb/auto-load/$(libdir)
eogdbload_SCRIPTS = libeo.so.@VMAJ@.@VMIN@.@VMIC@-gdb.py

CLEANFILES += libeo.so.@VMAJ@.@VMIN@.@VMIC@-gdb.py

EXTRA_DIST += $(eogdb_SCRIPTS) eo/libeo-gdb.py.in

if BUILD_GUI

########################################################################
# Edje
edjefilesdir = $(datadir)/edje/include
edjefiles_DATA = edje/include/edje.inc
EXTRA_DIST += $(edjefiles_DATA)

mimedir = $(datadir)/mime/packages
mime_DATA = edje/edje.xml
EXTRA_DIST += $(mime_DATA)

EXTRA_DIST += \
edje/edje-mode.el \
edje/edc.vim \
edje/vim/autoload/edccomplete.vim \
edje/vim/ftplugin/edc.vim \
edje/vim/indent/edc.vim \
edje/vim/ftdetect/edc.vim \
edje/vim/plugin-info.txt \
edje/vim/snippets/edc.snippets \
edje/vim/syntax/edc.vim \
edje/vim/syntax/embryo.vim

# Helper for people using EDJ
include ../src/Makefile_Edje_Helper.am

########################################################################
# Emotion
emotionfilesdir = $(datadir)/emotion
emotionfiles_DATA = emotion/checkme
EXTRA_DIST += $(emotionfiles_DATA)

########################################################################
# Ethumb
ethumbfilesdir = $(datadir)/ethumb
ethumbfiles_DATA = ethumb/checkme
EXTRA_DIST += $(ethumbfiles_DATA)

ethumbframesdir = $(datadir)/ethumb/frames
ethumbframes_DATA = ethumb/frames/default.edj
CLEANFILES += ethumb/frames/default.edj

ethumb/frames/default.edj: ethumb/frames/default.edc
	@$(MKDIR_P) ethumb/frames/
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) -id $(srcdir)/ethumb/frames/ $< $@


EXTRA_DIST += \
ethumb/frames/default.edc \
ethumb/frames/border-0.jpg

########################################################################
# Ethumb_Client
ethumb_clientfilesdir = $(datadir)/ethumb_client
ethumb_clientfiles_DATA = ethumb_client/checkme
EXTRA_DIST += $(ethumb_clientfiles_DATA)
endif
