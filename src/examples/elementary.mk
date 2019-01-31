DISTCLEANFILES += \
elementary/codegen_example_generated.c \
elementary/codegen_example_generated.h \
elementary/codegen_example.edj


include ../../Makefile_Elm_Helpers.am

PHONIES += elementary/screenshots

ELM_SRCS = \
elementary/actionslider_example_01.c \
elementary/bg_example_01.c \
elementary/bg_example_02.c \
elementary/bg_example_03.c \
elementary/box_example_02.c \
elementary/bubble_example_01.c \
elementary/button_example_00.c \
elementary/button_example_01.c \
elementary/calendar_example_01.c \
elementary/calendar_example_02.c \
elementary/calendar_example_03.c \
elementary/calendar_example_04.c \
elementary/calendar_example_05.c \
elementary/calendar_example_06.c \
elementary/check_example_01.c \
elementary/clock_example.c \
elementary/colorselector_example_01.c \
elementary/conformant_example_01.c \
elementary/conformant_example_02.c \
elementary/combobox_example_01.c \
elementary/ctxpopup_example_01.c \
elementary/datetime_example.c \
elementary/dayselector_example.c \
elementary/diskselector_example_01.c \
elementary/diskselector_example_02.c \
elementary/entry_example.c \
elementary/fileselector_button_example.c \
elementary/fileselector_entry_example.c \
elementary/fileselector_example.c \
elementary/fileviewlist.c \
elementary/filemvc.c \
elementary/flip_example_01.c \
elementary/flipselector_example.c \
elementary/frame_example_01.c \
elementary/general_funcs_example.c \
elementary/gengrid_example.c \
elementary/genlist_example_01.c \
elementary/genlist_example_02.c \
elementary/genlist_example_03.c \
elementary/genlist_example_04.c \
elementary/genlist_example_05.c \
elementary/glview_example_01.c \
elementary/hover_example_01.c \
elementary/hoversel_example_01.c \
elementary/icon_example_01.c \
elementary/image_example_01.c \
elementary/index_example_01.c \
elementary/index_example_02.c \
elementary/inwin_example.c \
elementary/label_example_01.c \
elementary/label_example_02.c \
elementary/label_example_03.c \
elementary/layout_example_01.c \
elementary/layout_example_02.c \
elementary/layout_example_03.c \
elementary/layout_property_bind.c \
elementary/list_example_01.c \
elementary/list_example_02.c \
elementary/list_example_03.c \
elementary/location_example_01.c \
elementary/map_example_01.c \
elementary/map_example_02.c \
elementary/map_example_03.c \
elementary/mapbuf_example.c \
elementary/menu_example_01.c \
elementary/naviframe_example.c \
elementary/notify_example_01.c \
elementary/panes_example.c \
elementary/panel_example_01.c \
elementary/photocam_example_01.c \
elementary/prefs_example_01.c \
elementary/prefs_example_02.c \
elementary/prefs_example_03.c \
elementary/popup_example_01.c \
elementary/popup_example_02.c \
elementary/popup_example_03.c \
elementary/progressbar_example.c \
elementary/radio_example_01.c \
elementary/radio_example_02.c \
elementary/segment_control_example.c \
elementary/separator_example_01.c \
elementary/slider_example.c \
elementary/slideshow_example.c \
elementary/spinner_example.c \
elementary/scroller_example_01.c \
elementary/table_example_01.c \
elementary/table_example_02.c \
elementary/theme_example_01.c \
elementary/theme_example_02.c \
elementary/thumb_example_01.c \
elementary/toolbar_example_01.c \
elementary/toolbar_example_02.c \
elementary/toolbar_example_03.c \
elementary/transit_example_01.c \
elementary/transit_example_02.c \
elementary/transit_example_03.c \
elementary/transit_example_04.c \
elementary/web_example_01.c \
elementary/web_example_02.c \
elementary/win_example.c \
elementary/track_example_01.c \
elementary/evas3d_object_on_button_example.c \
elementary/evas3d_scene_on_button_example.c \
elementary/evas3d_map_example.c \
elementary/efl_thread_1.c \
elementary/efl_thread_2.c \
elementary/efl_thread_3.c \
elementary/efl_thread_4.c \
elementary/efl_thread_5.c \
elementary/efl_thread_6.c \
elementary/efl_ui_list_example_1.c \
elementary/efl_ui_list_view_example_1.c \
elementary/efl_ui_list_view_example_2.c \
elementary/efl_ui_list_view_example_3.c \
elementary/efl_canvas_layout_text.c \
elementary/efl_ui_theme_example_01.c \
elementary/efl_ui_theme_example_02.c

ELM_SRCS += \
elementary/bg_cxx_example_01.cc \
elementary/bg_cxx_example_02.cc \
elementary/box_cxx_example_02.cc \
elementary/button_cxx_example_00.cc \
elementary/button_cxx_example_01.cc \
elementary/calendar_cxx_example_01.cc \
elementary/calendar_cxx_example_02.cc \
elementary/calendar_cxx_example_03.cc \
elementary/calendar_cxx_example_04.cc \
elementary/calendar_cxx_example_05.cc \
elementary/clock_cxx_example.cc \
elementary/icon_cxx_example_01.cc \
elementary/menu_cxx_example_01.cc \
elementary/popup_cxx_example.cc \
elementary/radio_cxx_example_01.cc \
elementary/slider_cxx_example.cc \
elementary/spinner_cxx_example.cc \
elementary/table_cxx_example_01.cc \
elementary/table_cxx_example_02.cc \
elementary/toolbar_cxx_example_01.cc \
$(NULL)

ELM_EDCS = \
elementary/twitter_example_01.edc \
elementary/evas3d_map_example.edc \
elementary/theme_example.edc \
elementary/layout_example.edc \
elementary/codegen_example.edc \
elementary/efl_ui_list_view_example.edc \
elementary/prefs_example_03.edc \
elementary/efl_canvas_layout_text.edc \
elementary/efl_ui_theme_example.edc

ELM_EPCS = \
elementary/prefs_example_01.epc \
elementary/prefs_example_02.epc \
elementary/prefs_example_03.epc

.epc.epb:
	$(AM_V_EPB)$(ELM_PREFS_CC) $< $@

ELM_DATA_FILES = \
elementary/sky_01.jpg \
elementary/dict.txt \
$(ELM_EDCS) \
$(ELM_EPCS)

clean-local-elm:
	rm -f elementary/*.epb elementary/*.cfg elementary/*.cfg.bkp elementary/*.edj

ELEMENTARY_CODEGEN_FLAGS = --prefix=codegen_example
ELEMENTARY_CODEGEN_FILES = \
elementary/codegen_example_generated.c \
elementary/codegen_example_generated.h

$(ELEMENTARY_CODEGEN_FILES): elementary/codegen_example.edj
	$(AM_V_CODEGEN)$(ELEMENTARY_CODEGEN) $(ELEMENTARY_CODEGEN_FLAGS) $(<) elm/example/mylayout/default $(ELEMENTARY_CODEGEN_FILES)


elementary_codegen_example_SOURCES = \
elementary/codegen_example.c
elementary_codegen_example_CPPFLAGS = \
$(AM_CPPFLAGS) \
-I$(top_builddir)/src/examples/elementary

nodist_elementary_codegen_example_SOURCES = \
$(ELEMENTARY_CODEGEN_FILES)

if HAVE_AM_16
elementary/codegen_example-codegen_example.$(OBJEXT): $(ELEMENTARY_CODEGEN_FILES)
else
elementary/elementary_codegen_example-codegen_example.$(OBJEXT): $(ELEMENTARY_CODEGEN_FILES)
endif


if HAVE_CSHARP

if HAVE_WIN32
MCSFLAGS ?=
MCSFLAGS += -define:WIN32
endif

elementary_efl_ui_slider_mono_SOURCES = \
	elementary/efl_ui_slider_mono.cs

elementary/efl_ui_slider_mono$(EXEEXT): $(elementary_efl_ui_slider_mono_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f elementary/efl_ui_slider_mono$(EXEEXT)
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

elementary_efl_ui_unit_converter_SOURCES = \
	elementary/efl_ui_unit_converter.cs

elementary/efl_ui_unit_converter$(EXEEXT): $(elementary_efl_ui_unit_converter_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f elementary/efl_ui_unit_converter$(EXEEXT)
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

endif

EXTRA_PROGRAMS += \
elementary/actionslider_example_01 \
elementary/bg_example_01 \
elementary/bg_example_02 \
elementary/bg_example_03 \
elementary/box_example_02 \
elementary/bubble_example_01 \
elementary/button_example_00 \
elementary/button_example_01 \
elementary/calendar_example_01 \
elementary/calendar_example_02 \
elementary/calendar_example_03 \
elementary/calendar_example_04 \
elementary/calendar_example_05 \
elementary/calendar_example_06 \
elementary/check_example_01 \
elementary/clock_example \
elementary/codegen_example \
elementary/colorselector_example_01 \
elementary/conformant_example_01 \
elementary/conformant_example_02 \
elementary/combobox_example_01 \
elementary/ctxpopup_example_01 \
elementary/datetime_example \
elementary/dayselector_example \
elementary/diskselector_example_01 \
elementary/diskselector_example_02 \
elementary/entry_example \
elementary/fileselector_button_example \
elementary/fileselector_entry_example \
elementary/fileselector_example \
elementary/fileviewlist \
elementary/filemvc \
elementary/flip_example_01 \
elementary/flipselector_example \
elementary/frame_example_01 \
elementary/general_funcs_example \
elementary/gengrid_example \
elementary/genlist_example_01 \
elementary/genlist_example_02 \
elementary/genlist_example_03 \
elementary/genlist_example_04 \
elementary/genlist_example_05 \
elementary/glview_example_01 \
elementary/hover_example_01 \
elementary/hoversel_example_01 \
elementary/icon_example_01 \
elementary/image_example_01 \
elementary/index_example_01 \
elementary/index_example_02 \
elementary/inwin_example \
elementary/label_example_01 \
elementary/label_example_02 \
elementary/label_example_03 \
elementary/layout_example_01 \
elementary/layout_example_02 \
elementary/layout_example_03 \
elementary/layout_property_bind \
elementary/list_example_01 \
elementary/list_example_02 \
elementary/list_example_03 \
elementary/location_example_01 \
elementary/map_example_01 \
elementary/map_example_02 \
elementary/map_example_03 \
elementary/mapbuf_example \
elementary/menu_example_01 \
elementary/naviframe_example \
elementary/notify_example_01 \
elementary/panes_example \
elementary/panel_example_01 \
elementary/photocam_example_01 \
elementary/prefs_example_01 \
elementary/prefs_example_02 \
elementary/prefs_example_03 \
elementary/popup_example_01 \
elementary/popup_example_02 \
elementary/popup_example_03 \
elementary/progressbar_example \
elementary/radio_example_01 \
elementary/radio_example_02 \
elementary/segment_control_example \
elementary/separator_example_01 \
elementary/slider_example \
elementary/slideshow_example \
elementary/spinner_example \
elementary/scroller_example_01 \
elementary/table_example_01 \
elementary/table_example_02 \
elementary/theme_example_01 \
elementary/theme_example_02 \
elementary/thumb_example_01 \
elementary/toolbar_example_01 \
elementary/toolbar_example_02 \
elementary/toolbar_example_03 \
elementary/transit_example_01 \
elementary/transit_example_02 \
elementary/transit_example_03 \
elementary/transit_example_04 \
elementary/web_example_01 \
elementary/web_example_02 \
elementary/win_example \
elementary/track_example_01 \
elementary/evas3d_object_on_button_example \
elementary/evas3d_scene_on_button_example \
elementary/evas3d_map_example \
elementary/efl_thread_1 \
elementary/efl_thread_2 \
elementary/efl_thread_3 \
elementary/efl_thread_4 \
elementary/efl_thread_5 \
elementary/efl_thread_6 \
elementary/efl_ui_list_example_1 \
elementary/efl_ui_list_view_example_1 \
elementary/efl_ui_list_view_example_2 \
elementary/efl_ui_list_view_example_3 \
elementary/efl_canvas_layout_text \
elementary/efl_ui_theme_example_01 \
elementary/efl_ui_theme_example_02
#benchmark3d
#sphere-hunter

# benchmark3d_SOURCES = \
	performance/performance.c \
	performance/camera_light.c \
	performance/tools_private.h \
	performance/graphical_struct.h \
	performance/Tools.h \
	performance/graphical.c

# sphere_hunter_SOURCES = \
	sphere_hunter/evas_3d_sphere_hunter.c

# files_DATA += performance/layout.edj sphere_hunter/sphere_hunter.edj

# performance/layout.edj: performance/layout.edc
#	$(AM_V_EDJ) \
	$(MKDIR_P) performance/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) -id performance/ $< $@

# sphere_hunter/sphere_hunter.edj: sphere_hunter/sphere_hunter.edc
#	$(AM_V_EDJ) \
	$(MKDIR_P) sphere_hunter/; \
	$(EDJE_CC) $(EDJE_CC_FLAGS) -id sphere_hunter/ -id $(top_srcdir)/data/elementary/themes $< $@

if HAVE_CXX
EXTRA_PROGRAMS += \
elementary/bg_cxx_example_01 \
elementary/bg_cxx_example_02 \
elementary/box_cxx_example_02 \
elementary/button_cxx_example_00 \
elementary/button_cxx_example_01 \
elementary/calendar_cxx_example_01 \
elementary/calendar_cxx_example_02 \
elementary/calendar_cxx_example_03 \
elementary/calendar_cxx_example_04 \
elementary/calendar_cxx_example_05 \
elementary/clock_cxx_example \
elementary/icon_cxx_example_01 \
elementary/menu_cxx_example_01 \
elementary/popup_cxx_example \
elementary/radio_cxx_example_01 \
elementary/slider_cxx_example \
elementary/spinner_cxx_example \
elementary/table_cxx_example_01 \
elementary/table_cxx_example_02 \
elementary/toolbar_cxx_example_01 \
$(NULL)

elementary_bg_cxx_example_01_SOURCES = elementary/bg_cxx_example_01.cc
elementary_bg_cxx_example_02_SOURCES = elementary/bg_cxx_example_02.cc
elementary_box_cxx_example_02_SOURCES = elementary/box_cxx_example_02.cc
elementary_button_cxx_example_00_SOURCES = elementary/button_cxx_example_00.cc
elementary_button_cxx_example_01_SOURCES = elementary/button_cxx_example_01.cc
elementary_calendar_cxx_example_01_SOURCES = elementary/calendar_cxx_example_01.cc
elementary_calendar_cxx_example_02_SOURCES = elementary/calendar_cxx_example_02.cc
elementary_calendar_cxx_example_03_SOURCES = elementary/calendar_cxx_example_03.cc
elementary_calendar_cxx_example_04_SOURCES = elementary/calendar_cxx_example_04.cc
elementary_calendar_cxx_example_05_SOURCES = elementary/calendar_cxx_example_05.cc
elementary_clock_cxx_example_SOURCES = elementary/clock_cxx_example.cc
elementary_icon_cxx_example_01_SOURCES = elementary/icon_cxx_example_01.cc
elementary_menu_cxx_example_01_SOURCES = elementary/menu_cxx_example_01.cc
elementary_radio_cxx_example_01_SOURCES = elementary/radio_cxx_example_01.cc
elementary_popup_cxx_example_SOURCES = elementary/popup_cxx_example.cc
elementary_slider_cxx_example_SOURCES = elementary/slider_cxx_example.cc
elementary_spinner_cxx_example_SOURCES = elementary/spinner_cxx_example.cc
elementary_table_cxx_example_01_SOURCES = elementary/table_cxx_example_01.cc
elementary_table_cxx_example_02_SOURCES = elementary/table_cxx_example_02.cc
elementary_toolbar_cxx_example_01_SOURCES = elementary/toolbar_cxx_example_01.cc


endif

if HAVE_CSHARP

EXTRA_PROGRAMS += \
elementary/efl_ui_slider_mono \
elementary/efl_ui_unit_converter

endif

# This variable will hold the list of screenshots that will be made
# by "make screenshots". Each item in the list is of the form:
# <example executable>:<screenshot filename>:<delay in seconds>
SCREENSHOTS = \
elementary/actionslider_example_01:actionslider_01.png:0.0 \
elementary/bg_example_02:bg_01.png:0.0 \
elementary/box_example_02:box_example_02.png:1.3 \
elementary/bubble_example_01:bubble_example_01.png:0.0 \
elementary/button_example_00:button_00.png:0.0 \
elementary/button_example_01:button_01.png:0.0 \
elementary/check_example_01:check_example_01.png:0.0 \
elementary/colorselector_example_01:colorselector_example_01.png:0.0 \
elementary/animator_example_01:animator_example_01.png:0.2 \
elementary/animator_example_01:animator_example_02.png:0.5 \
elementary/animator_example_01:animator_example_03.png:0.9 \
elementary/flip_example_01:flip_example_01.png:0.0 \
elementary/frame_example_01:frame_example_01.png:0.0 \
elementary/hover_example_01:hover_example_01.png:0.0 \
elementary/hoversel_example_01:hoversel_example_01.png:0.0 \
elementary/label_example_01:label_example_01.png:0.0 \
elementary/theme_example_01:theme_example_01.png:0.0 \
elementary/conformant_example_01:conformant_example_01.png:0.0 \
elementary/conformant_example_02:conformant_example_02.png:0.0 \
elementary/combobox_example_01:combobox_example_01.png:0.0 \
elementary/calendar_example_01:calendar_example_01.png:0.0 \
elementary/calendar_example_02:calendar_example_02.png:0.0 \
elementary/calendar_example_03:calendar_example_03.png:0.0 \
elementary/calendar_example_04:calendar_example_04.png:0.0 \
elementary/calendar_example_05:calendar_example_05.png:0.0 \
elementary/calendar_example_06:calendar_example_06.png:0.0 \
elementary/datetime_example:datetime_example.png:0.0 \
elementary/map_example_01:map_example_01.png:2 \
elementary/map_example_02:map_example_02.png:2.5 \
elementary/map_example_03:map_example_03.png:2 \
elementary/toolbar_example_01:toolbar_example_01.png:0.0 \
elementary/toolbar_example_02:toolbar_example_02.png:1 \
elementary/toolbar_example_03:toolbar_example_03.png:1 \
elementary/spinner_example:spinner_example.png:0.0 \
elementary/clock_example:clock_example.png:0.5 \
elementary/dayselector_example:dayselector_example.png:0.0 \
elementary/mapbuf_example:mapbuf_example.png:0.0 \
elementary/image_example_01:image_example_01.png:0.0 \
elementary/diskselector_example_01:diskselector_example_01.png:0.2 \
elementary/diskselector_example_02:diskselector_example_02.png:0.2 \
elementary/icon_example_01:icon_example_01.png:0.0 \
elementary/layout_example_01:layout_example_01.png:0.0 \
elementary/layout_example_02:layout_example_02.png:0.0 \
elementary/layout_example_03:layout_example_03.png:0.0 \
elementary/list_example_01:list_example_01.png:0.0 \
elementary/list_example_02:list_example_02.png:0.0 \
elementary/list_example_03:list_example_03.png:0.0 \
elementary/segment_control_example:segment_control_example.png:0.0 \
elementary/flipselector_example:flipselector_example.png:0.0 \
elementary/fileselector_example:fileselector_example.png:0.0 \
elementary/index_example_02:index_example_03.png:0.3 \
elementary/slider_example:slider_example.png:0.0 \
elementary/panes_example:panes_example.png:0.0 \
elementary/ctxpopup_example_01:ctxpopup_example_01.png:0.0 \
elementary/separator_example_01:separator_example_01.png:0.0 \
elementary/radio_example_01:radio_example_01.png:0.0 \
elementary/radio_example_02:radio_example_02.png:0.0 \
elementary/panel_example_01:panel_example_01.png:0.0 \
elementary/gengrid_example:gengrid_example.png:0.0 \
elementary/genlist_example_01:genlist_example_01.png:0.1 \
elementary/genlist_example_02:genlist_example_02.png:0.1 \
elementary/genlist_example_03:genlist_example_03.png:0.1 \
elementary/genlist_example_04:genlist_example_04.png:0.1 \
elementary/genlist_example_05:genlist_example_05.png:0.1 \
elementary/thumb_example_01:thumb_example_01.png:0.5 \
elementary/entry_example:entry_example.png:0.0 \
elementary/progressbar_example:progressbar_example.png:0.0 \
elementary/notify_example_01:notify_example_01.png:0.0 \
elementary/notify_example_01:notify_example_01_a.png:6.0 \
elementary/popup_example_01:popup_example_01.png:1.0 \
elementary/popup_example_01:popup_example_01_a.png:6.0 \
elementary/popup_example_02:popup_example_02.png:0.2 \
elementary/popup_example_03:popup_example_03.png:0.2 \
elementary/slideshow_example:slideshow_example.png:1.0 \
elementary/photocam_example_01:photocam_example_01.png:3 \
elementary/scroller_example_01:scroller_example_01.png:0.0 \
elementary/inwin_example:inwin_example.png:0.0 \
elementary/inwin_example:inwin_example_a.png:0.2 \
elementary/table_example_01:table_example_01.png:0.0 \
elementary/table_example_02:table_example_02.png:0.0 \
elementary/menu_example_01:menu_example_01.png:0.5

if HAVE_CXX
SCREENSHOTS += \
elementary/bg_cxx_example_01:bg_cxx_example_01.png:0.0 \
elementary/bg_cxx_example_02:bg_cxx_example_02.png:0.0 \
elementary/box_cxx_example_02:box_cxx_example_02.png:1.3 \
elementary/button_cxx_example_00:button_cxx_example_00.png:0.0 \
elementary/button_cxx_example_01:button_cxx_example_01.png:0.0 \
elementary/calendar_cxx_example_01:calendar_cxx_example_01.png:0.0 \
elementary/calendar_cxx_example_02:calendar_cxx_example_02.png:0.0 \
elementary/calendar_cxx_example_03:calendar_cxx_example_03.png:0.0 \
elementary/calendar_cxx_example_04:calendar_cxx_example_04.png:0.0 \
elementary/calendar_cxx_example_05:calendar_cxx_example_05.png:0.0 \
elementary/clock_cxx_example:clock_cxx_example.png:0.5 \
elementary/icon_cxx_example_01:icon_cxx_example_01.png:0.0 \
elementary/menu_cxx_example_01:menu_cxx_example_01.png:0.5 \
elementary/popup_cxx_example_01:popup_cxx_example_01_a.png:6.0 \
elementary/radio_cxx_example_01:radio_cxx_example_01.png:0.0 \
elementary/slider_cxx_example:slider_cxx_example.png:0.0 \
elementary/spinner_cxx_example:spinner_cxx_example.png:0.0 \
elementary/table_cxx_example_01:table_cxx_example_01.png:0.0 \
elementary/table_cxx_example_02:table_cxx_example_02.png:0.0 \
elementary/toolbar_cxx_example_01:toolbar_cxx_example_01.png:0.0 \
$(NULL)
endif

HTML_SS_DIR=$(top_builddir)/doc/html/screenshots
LATEX_SS_DIR=$(top_builddir)/doc/latex/screenshots
screenshots: examples
	@mkdir -p $(HTML_SS_DIR)
	@mkdir -p $(LATEX_SS_DIR)
	@for ss in $(SCREENSHOTS); do \
		SS_ENTRY=($${ss//:/ }) ; \
		EXAMPLE=$${SS_ENTRY[0]} ; \
		SS_FILE=$${SS_ENTRY[1]} ; \
		SS_DELAY=$${SS_ENTRY[2]} ; \
		ILLUME_KBD=0,0,240,123 ILLUME_IND=0,0,240,32 ILLUME_STK=0,288,240,32 ELM_ENGINE="shot:delay=$${SS_DELAY}:file=$(HTML_SS_DIR)/$${SS_FILE}" ./$${EXAMPLE} ; \
		convert $(HTML_SS_DIR)/$${SS_FILE} $(LATEX_SS_DIR)/$${SS_FILE/.png/.eps} ; \
	done

ELM_DATA_FILES += \
elementary/performance/layout.edc \
elementary/performance/background.png \
elementary/performance/target_texture.png \
elementary/sphere_hunter/sphere_hunter.edc \
elementary/sphere_hunter/score.jpg \
elementary/sphere_hunter/EFL_victory.png \
elementary/sphere_hunter/EFL_defeat.png

ELM_EDJS = $(ELM_EDCS:%.edc=%.edj)
ELM_EPBS = $(ELM_EPCS:%.epc=%.epb)
EDCS += $(ELM_EDCS)

if HAVE_CSHARP
ELM_DATA_FILES += $(MONO_SRCS)
endif

DATA_FILES += $(ELM_DATA_FILES)

install-examples-elm:
	$(MKDIR_P) $(DESTDIR)$(datadir)/elementary/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(ELM_SRCS) $(ELM_EDCS) $(ELM_DATA_FILES) $(DESTDIR)$(datadir)/elementary/examples
	cd $(builddir) && $(install_sh_DATA) -c $(ELM_EDJS) $(DESTDIR)$(datadir)/elementary/examples

uninstall-local-elm:
	for f in $(ELM_SRCS); do \
	  rm -f $(DESTDIR)$(datadir)/elementary/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-elm
UNINSTALL_EXAMPLES += uninstall-local-elm
