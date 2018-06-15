MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST =
CLEANFILES =
SUBDIRS =

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
eogdb_DATA = eo/eo_gdb.py
EXTRA_DIST += $(eogdb_DATA)

# Borrowed from gobject
libeo.so.@VMAJ@.@VMIN@.@VMIC@-gdb.py: eo/libeo-gdb.py.in
	$(AM_V_GEN) $(SED) -e "s|\@datadir\@|$(datadir)|" $(srcdir)/eo/libeo-gdb.py.in > $(builddir)/libeo.so.@VMAJ@.@VMIN@.@VMIC@-gdb.py

eogdbloaddir = $(datadir)/gdb/auto-load/$(libdir)
eogdbload_DATA = libeo.so.@VMAJ@.@VMIN@.@VMIC@-gdb.py
EXTRA_DIST += $(eogdbload_DATA)

CLEANFILES += libeo.so.@VMAJ@.@VMIN@.@VMIC@-gdb.py

EXTRA_DIST += $(eogdb_SCRIPTS) eo/libeo-gdb.py.in

########################################################################
# Edje
edjefilesdir = $(datadir)/edje/include
edjefiles_DATA = edje/include/edje.inc
EXTRA_DIST += $(edjefiles_DATA)

mimedir = $(datadir)/mime/packages
mime_DATA = edje/edje.xml
EXTRA_DIST += $(mime_DATA)

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
	$(AM_V_EDJ) \
	$(MKDIR_P) ethumb/frames/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) -id $(srcdir)/ethumb/frames/ $< $@


EXTRA_DIST += \
ethumb/frames/default.edc \
ethumb/frames/border-0.jpg

########################################################################
# Ethumb_Client
ethumb_clientfilesdir = $(datadir)/ethumb_client
ethumb_clientfiles_DATA = ethumb_client/checkme
EXTRA_DIST += $(ethumb_clientfiles_DATA)

########################################################################
# Elua
eluafilesdir = $(datadir)/elua
eluafiles_DATA = elua/checkme
EXTRA_DIST += $(eluafiles_DATA)

#######################################################################
# Elementary

# themes

SUBDIRS += elementary/config

elementaryfilesdir = $(datadir)/elementary/themes
elementaryfiles_DATA = elementary/themes/default.edj
CLEANFILES += elementary/themes/default.edj

include elementary/themes/Makefile.am

elementary/themes/default.edj: elementary/themes/default.edc $(elementary_themes_files) $(efl_ui_themes_files) $(elementary_fdo_files) $(top_builddir)/src/bin/edje/edje_cc${EXEEXT}
	$(AM_V_EDJ) \
	$(MKDIR_P) elementary/themes/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) $(EDJE_CC_ELM_FLAGS) -N $< $@

# desktop files

elementarydesktopdir = $(datadir)/applications
elementarydesktop_DATA = elementary/desktop/elementary_test.desktop elementary/desktop/elementary_config.desktop

elementaryicondir = $(datadir)/icons
elementaryicon_DATA = elementary/desktop/elementary.png

EXTRA_DIST += $(elementarydesktop_DATA) $(elementaryicon_DATA)

# fdo icon theme

elementaryfdodir = $(datadir)/icons/Enlightenment-X/
elementaryfdo_DATA = elementary/themes/fdo/index.theme elementary/themes/fdo/README

EXTRA_DIST += $(elementaryfdo_DATA)

elementaryfdo_actions_128dir = $(datadir)/icons/Enlightenment-X/actions/128
elementaryfdo_actions_128_DATA = $(elementary_fdo_actions_128_files)

elementaryfdo_categories_16dir = $(datadir)/icons/Enlightenment-X/categories/16
elementaryfdo_categories_16_DATA = $(elementary_fdo_categories_16_files)

elementaryfdo_categories_128dir = $(datadir)/icons/Enlightenment-X/categories/128
elementaryfdo_categories_128_DATA = $(elementary_fdo_categories_128_files)

elementaryfdo_emblems_96dir = $(datadir)/icons/Enlightenment-X/emblems/96
elementaryfdo_emblems_96_DATA = $(elementary_fdo_emblems_96_files)

elementaryfdo_places_16dir = $(datadir)/icons/Enlightenment-X/places/16
elementaryfdo_places_16_DATA = $(elementary_fdo_places_16_files)

elementaryfdo_places_128dir = $(datadir)/icons/Enlightenment-X/places/128
elementaryfdo_places_128_DATA = $(elementary_fdo_places_128_files)

elementaryfdo_status_128dir = $(datadir)/icons/Enlightenment-X/status/128
elementaryfdo_status_128_DATA = $(elementary_fdo_status_128_files)

elementaryfdo_devices_128dir = $(datadir)/icons/Enlightenment-X/devices/128
elementaryfdo_devices_128_DATA = $(elementary_fdo_devices_128_files)

elementaryfdo_apps_64dir = $(datadir)/icons/Enlightenment-X/apps/64
elementaryfdo_apps_64_DATA = $(elementary_fdo_apps_64_files)

elementaryfdo_intl_128dir = $(datadir)/icons/Enlightenment-X/intl/128
elementaryfdo_intl_128_DATA = $(elementary_fdo_intl_128_files)

elementaryfdo_mimetypes_16dir = $(datadir)/icons/Enlightenment-X/mimetypes/16
elementaryfdo_mimetypes_16_DATA = $(elementary_fdo_mimetypes_16_files)

elementaryfdo_mimetypes_128dir = $(datadir)/icons/Enlightenment-X/mimetypes/128
elementaryfdo_mimetypes_128_DATA = $(elementary_fdo_mimetypes_128_files)

# edje_externals

elementaryedje_externalsdir = $(datadir)/elementary/edje_externals
elementaryedje_externals_DATA = elementary/edje_externals/icons.edj
CLEANFILES += $(elementaryedje_externals_DATA)

include elementary/edje_externals/Makefile.am

elementary/edje_externals/icons.edj: elementary/edje_externals/icons.edc
	$(AM_V_EDJ) \
	$(MKDIR_P) elementary/edje_externals/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) $< $@

# images

include elementary/images/Makefile.am

elementaryimagesdir = $(datadir)/elementary/images
elementaryimages_DATA = $(elementary_images_files)

elementaryimages_glayerdir = $(datadir)/elementary/images/g_layer
elementaryimages_glayer_DATA = $(elementary_images_glayer_files)

EXTRA_DIST += $(elementary_images_files) $(elementary_images_glayer_files)

# objects

include ../Makefile_Elm_Helpers.am

elementaryobjectsdir = $(datadir)/elementary/objects
elementaryobjects_DATA = \
elementary/objects/test.edj \
elementary/objects/test_external.edj \
elementary/objects/test_masking.edj \
elementary/objects/multip.edj \
elementary/objects/cursors.edj \
elementary/objects/font_preview.edj \
elementary/objects/postit_ent.edj \
elementary/objects/multibuttonentry.edj \
elementary/objects/test_prefs.edj \
elementary/objects/test_prefs.epb \
elementary/objects/test_focus_style.edj \
elementary/objects/test_tooltip.edj \
elementary/objects/test_tab_pager.edj \
$(NULL)

CLEANFILES += $(elementaryobjects_DATA)

include elementary/objects/Makefile.am

elementary/objects/test.edj: elementary/objects/test.edc
	$(AM_V_EDJ) \
	$(MKDIR_P) elementary/objects/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) $< $@

elementary/objects/test_external.edj: elementary/objects/test_external.edc
	$(AM_V_EDJ) \
	$(MKDIR_P) elementary/objects/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) $< $@

elementary/objects/test_masking.edj: elementary/objects/test_masking.edc
	$(AM_V_EDJ) \
	$(MKDIR_P) elementary/objects/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) $< $@

elementary/objects/multip.edj: elementary/objects/multip.edc
	$(AM_V_EDJ) \
	$(MKDIR_P) elementary/objects/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) $< $@

elementary/objects/cursors.edj: elementary/objects/cursors.edc
	$(AM_V_EDJ) \
	$(MKDIR_P) elementary/objects/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) $< $@

elementary/objects/font_preview.edj: elementary/objects/font_preview.edc
	$(AM_V_EDJ) \
	$(MKDIR_P) elementary/objects/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) $< $@

elementary/objects/postit_ent.edj: elementary/objects/postit_ent.edc
	$(AM_V_EDJ) \
	$(MKDIR_P) elementary/objects/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) $< $@

elementary/objects/multibuttonentry.edj: elementary/objects/multibuttonentry.edc
	$(AM_V_EDJ) \
	$(MKDIR_P) elementary/objects/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) $< $@

elementary/objects/test_prefs.edj: elementary/objects/test_prefs.edc
	$(AM_V_EDJ) \
	$(MKDIR_P) elementary/objects/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) $< $@

elementary/objects/test_focus_style.edj: elementary/objects/test_focus_style.edc
	$(AM_V_EDJ) \
	$(MKDIR_P) elementary/objects/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) -id $(top_srcdir)/data/elementary/themes/img $< $@

elementary/objects/test_tooltip.edj: elementary/objects/test_tooltip.edc
	$(AM_V_EDJ) \
	$(MKDIR_P) elementary/objects/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) -id $(top_srcdir)/data/elementary/themes/img $< $@

elementary/objects/test_prefs.epb: elementary/objects/test_prefs.epc
	$(AM_V_EPB) \
	$(MKDIR_P) elementary/objects/; \
	$(ELM_PREFS_CC) $(ELM_PREFS_FLAGS) $< $@

elementary/objects/test_tab_pager.edj: elementary/objects/test_tab_pager.edc
	$(AM_V_EDJ) \
	$(MKDIR_P) elementary/objects/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) $< $@
