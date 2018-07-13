MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES = \
codegen_example_generated.c \
codegen_example_generated.h \
codegen_example.edj


include ../../../Makefile_Elm_Helpers.am
include ../../Makefile_Edje_Helper.am

.PHONY: screenshots

examplesdir = $(pkgdatadir)/examples
filesdir = $(pkgdatadir)/examples
files_DATA =

AM_CPPFLAGS = \
-Wno-unused-parameter \
-I. \
-I$(top_builddir)/src/bindings \
-I$(top_srcdir)/src/bindings \
-I$(top_builddir)/src/lib/efl \
-I$(top_builddir)/src/lib/efl/interfaces \
-I$(top_srcdir)/src/lib/eina \
-I$(top_builddir)/src/lib/eina \
-I$(top_srcdir)/src/bindings/cxx/eina_cxx \
-I$(top_builddir)/src/bindings/cxx/eina_cxx \
-I$(top_srcdir)/src/lib/eo \
-I$(top_builddir)/src/lib/eo \
-I$(top_srcdir)/src/bindings/cxx/eo_cxx \
-I$(top_builddir)/src/bindings/cxx/eo_cxx \
-I$(top_srcdir)/src/bindings/cxx/eldbus_cxx \
-I$(top_builddir)/src/bindings/cxx/eldbus_cxx \
-I$(top_srcdir)/src/bindings/cxx/ecore_cxx \
-I$(top_builddir)/src/bindings/cxx/ecore_cxx \
-I$(top_srcdir)/src/lib/ector \
-I$(top_builddir)/src/lib/ector \
-I$(top_srcdir)/src/lib/evas \
-I$(top_builddir)/src/lib/evas \
-I$(top_srcdir)/src/lib/evas/canvas \
-I$(top_builddir)/src/lib/evas/canvas \
-I$(top_srcdir)/src/lib/evas/gesture \
-I$(top_builddir)/src/lib/evas/gesture \
-I$(top_srcdir)/src/lib/elementary \
-I$(top_builddir)/src/lib/elementary \
-I$(top_srcdir)/src/lib/elocation\
-I$(top_builddir)/src/lib/elocation\
-DPACKAGE_DATA_DIR="\"$(datadir)/elementary\"" \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
@ELEMENTARY_CFLAGS@

AM_CXXFLAGS = \
@ELEMENTARY_CFLAGS@ \
@ELEMENTARY_CXX_CFLAGS@

LDADD = \
@ELEMENTARY_LDFLAGS@ \
-lm \
$(top_builddir)/src/lib/eina/libeina.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/efl/libefl.la \
$(top_builddir)/src/lib/eet/libeet.la \
$(top_builddir)/src/lib/evas/libevas.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_con/libecore_con.la \
$(top_builddir)/src/lib/ecore_evas/libecore_evas.la \
$(top_builddir)/src/lib/ecore_file/libecore_file.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
$(top_builddir)/src/lib/ecore_imf/libecore_imf.la \
$(top_builddir)/src/lib/ecore_imf_evas/libecore_imf_evas.la \
$(top_builddir)/src/lib/embryo/libembryo.la \
$(top_builddir)/src/lib/edje/libedje.la \
$(top_builddir)/src/lib/elementary/libelementary.la \
$(top_builddir)/src/lib/ethumb_client/libethumb_client.la \
$(top_builddir)/src/lib/elocation/libelocation.la

SRCS = \
actionslider_example_01.c \
bg_example_01.c \
bg_example_02.c \
bg_example_03.c \
box_example_02.c \
bubble_example_01.c \
button_example_00.c \
button_example_01.c \
calendar_example_01.c \
calendar_example_02.c \
calendar_example_03.c \
calendar_example_04.c \
calendar_example_05.c \
calendar_example_06.c \
check_example_01.c \
clock_example.c \
colorselector_example_01.c \
conformant_example_01.c \
conformant_example_02.c \
combobox_example_01.c \
ctxpopup_example_01.c \
datetime_example.c \
dayselector_example.c \
diskselector_example_01.c \
diskselector_example_02.c \
entry_example.c \
fileselector_button_example.c \
fileselector_entry_example.c \
fileselector_example.c \
fileviewlist.c \
filemvc.c \
flip_example_01.c \
flipselector_example.c \
frame_example_01.c \
general_funcs_example.c \
gengrid_example.c \
genlist_example_01.c \
genlist_example_02.c \
genlist_example_03.c \
genlist_example_04.c \
genlist_example_05.c \
hover_example_01.c \
hoversel_example_01.c \
icon_example_01.c \
image_example_01.c \
index_example_01.c \
index_example_02.c \
inwin_example.c \
label_example_01.c \
layout_example_01.c \
layout_example_02.c \
layout_example_03.c \
layout_model_connect.c \
list_example_01.c \
list_example_02.c \
list_example_03.c \
location_example_01.c \
map_example_01.c \
map_example_02.c \
map_example_03.c \
mapbuf_example.c \
menu_example_01.c \
naviframe_example.c \
notify_example_01.c \
panes_example.c \
panel_example_01.c \
photocam_example_01.c \
prefs_example_01.c \
prefs_example_02.c \
prefs_example_03.c \
popup_example_01.c \
popup_example_02.c \
popup_example_03.c \
progressbar_example.c \
radio_example_01.c \
radio_example_02.c \
segment_control_example.c \
separator_example_01.c \
slider_example.c \
slideshow_example.c \
spinner_example.c \
scroller_example_01.c \
table_example_01.c \
table_example_02.c \
theme_example_01.c \
theme_example_02.c \
thumb_example_01.c \
toolbar_example_01.c \
toolbar_example_02.c \
toolbar_example_03.c \
transit_example_01.c \
transit_example_02.c \
transit_example_03.c \
transit_example_04.c \
web_example_01.c \
web_example_02.c \
win_example.c \
track_example_01.c \
evas3d_object_on_button_example.c \
evas3d_scene_on_button_example.c \
evas3d_map_example.c \
efl_thread_1.c \
efl_thread_2.c \
efl_thread_3.c \
efl_thread_4.c \
efl_thread_5.c \
efl_thread_6.c \
efl_ui_list_example_1.c \
efl_ui_view_list_example_1.c \
efl_ui_view_list_example_2.c \
efl_ui_view_list_example_3.c

SRCS += \
bg_cxx_example_01.cc \
bg_cxx_example_02.cc \
box_cxx_example_02.cc \
button_cxx_example_00.cc \
button_cxx_example_01.cc \
calendar_cxx_example_01.cc \
calendar_cxx_example_02.cc \
calendar_cxx_example_03.cc \
calendar_cxx_example_04.cc \
calendar_cxx_example_05.cc \
clock_cxx_example.cc \
icon_cxx_example_01.cc \
menu_cxx_example_01.cc \
popup_cxx_example.cc \
radio_cxx_example_01.cc \
slider_cxx_example.cc \
spinner_cxx_example.cc \
table_cxx_example_01.cc \
table_cxx_example_02.cc \
toolbar_cxx_example_01.cc \
$(NULL)

EDCS = \
twitter_example_01.edc \
evas3d_map_example.edc \
theme_example.edc \
layout_example.edc \
codegen_example.edc \
efl_ui_view_list_example.edc

EDJS = $(EDCS:%.edc=%.edj)

.edc.edj:
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) $< $@

.epc.epb:
	$(AM_V_EPB)$(ELM_PREFS_CC) $< $@

files_DATA += $(SRCS) \
	dict.txt

files_DATA += \
	prefs_example_01.epb \
	prefs_example_02.epb \
	prefs_example_03.epb \
	prefs_example_03.edj \
	theme_example.edj \
	layout_example.edj\
	codegen_example.edj \
	evas3d_map_example.edj \
	twitter_example_01.edj \
	efl_ui_view_list_example.edj

CLEANFILES = \
	theme_example.edj \
	layout_example.edj\
	evas3d_map_example.edj \
	twitter_example_01.edj \
	efl_ui_view_list_example.edj

clean-local:
	rm -f *.epb *.cfg *.cfg.bkp
	rm -f prefs_example_03.edj

ELEMENTARY_CODEGEN_FLAGS = --prefix=codegen_example

codegen_example_generated.c codegen_example_generated.h: codegen_example.edj
	$(AM_V_CODEGEN)$(ELEMENTARY_CODEGEN) $(ELEMENTARY_CODEGEN_FLAGS) $(<) elm/example/mylayout/default codegen_example_generated.c codegen_example_generated.h

codegen_example.$(OBJEXT): codegen_example_generated.h

codegen_example_SOURCES = \
	codegen_example.c
nodist_codegen_example_SOURCES = \
codegen_example_generated.c \
codegen_example_generated.h

if HAVE_CSHARP

if HAVE_WIN32
MCSFLAGS ?=
MCSFLAGS += -define:WIN32
endif

efl_ui_slider_mono_SOURCES = \
	efl_ui_slider_mono.cs

efl_ui_slider_mono$(EXEEXT): $(efl_ui_slider_mono_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f efl_ui_slider_mono$(EXEEXT)
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

efl_ui_unit_converter_SOURCES = \
	efl_ui_unit_converter.cs

efl_ui_unit_converter$(EXEEXT): $(efl_ui_unit_converter_SOURCES) $(am_dirstamp) $(top_builddir)/src/lib/efl_mono/libefl_mono.dll
	@rm -f efl_ui_unit_converter$(EXEEXT)
	$(AM_V_MCS) $(MCS) $(MCSFLAGS) -r:$(abs_top_builddir)/src/lib/efl_mono/libefl_mono.dll -out:$@ $(filter %.cs, $(^))

endif

examples_PROGRAMS = \
actionslider_example_01 \
bg_example_01 \
bg_example_02 \
bg_example_03 \
box_example_02 \
bubble_example_01 \
button_example_00 \
button_example_01 \
calendar_example_01 \
calendar_example_02 \
calendar_example_03 \
calendar_example_04 \
calendar_example_05 \
calendar_example_06 \
check_example_01 \
clock_example \
codegen_example \
colorselector_example_01 \
conformant_example_01 \
conformant_example_02 \
combobox_example_01 \
ctxpopup_example_01 \
datetime_example \
dayselector_example \
diskselector_example_01 \
diskselector_example_02 \
entry_example \
fileselector_button_example \
fileselector_entry_example \
fileselector_example \
fileviewlist \
filemvc \
flip_example_01 \
flipselector_example \
frame_example_01 \
general_funcs_example \
gengrid_example \
genlist_example_01 \
genlist_example_02 \
genlist_example_03 \
genlist_example_04 \
genlist_example_05 \
hover_example_01 \
hoversel_example_01 \
icon_example_01 \
image_example_01 \
index_example_01 \
index_example_02 \
inwin_example \
label_example_01 \
layout_example_01 \
layout_example_02 \
layout_example_03 \
layout_model_connect \
list_example_01 \
list_example_02 \
list_example_03 \
location_example_01 \
map_example_01 \
map_example_02 \
map_example_03 \
mapbuf_example \
menu_example_01 \
naviframe_example \
notify_example_01 \
panes_example \
panel_example_01 \
photocam_example_01 \
prefs_example_01 \
prefs_example_02 \
prefs_example_03 \
popup_example_01 \
popup_example_02 \
popup_example_03 \
progressbar_example \
radio_example_01 \
radio_example_02 \
segment_control_example \
separator_example_01 \
slider_example \
slideshow_example \
spinner_example \
scroller_example_01 \
table_example_01 \
table_example_02 \
theme_example_01 \
theme_example_02 \
thumb_example_01 \
toolbar_example_01 \
toolbar_example_02 \
toolbar_example_03 \
transit_example_01 \
transit_example_02 \
transit_example_03 \
transit_example_04 \
web_example_01 \
web_example_02 \
win_example \
track_example_01 \
evas3d_object_on_button_example \
evas3d_scene_on_button_example \
evas3d_map_example \
efl_thread_1 \
efl_thread_2 \
efl_thread_3 \
efl_thread_4 \
efl_thread_5 \
efl_thread_6 \
efl_ui_list_example_1 \
efl_ui_view_list_example_1 \
efl_ui_view_list_example_2 \
efl_ui_view_list_example_3
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
examples_PROGRAMS += \
bg_cxx_example_01 \
bg_cxx_example_02 \
box_cxx_example_02 \
button_cxx_example_00 \
button_cxx_example_01 \
calendar_cxx_example_01 \
calendar_cxx_example_02 \
calendar_cxx_example_03 \
calendar_cxx_example_04 \
calendar_cxx_example_05 \
clock_cxx_example \
icon_cxx_example_01 \
menu_cxx_example_01 \
popup_cxx_example \
radio_cxx_example_01 \
slider_cxx_example \
spinner_cxx_example \
table_cxx_example_01 \
table_cxx_example_02 \
toolbar_cxx_example_01 \
$(NULL)
endif

if HAVE_CSHARP

examples_PROGRAMS += \
	efl_ui_slider_mono \
	efl_ui_unit_converter

endif

efl_thread_1_SOURCES = efl_thread_1.c
efl_thread_2_SOURCES = efl_thread_2.c
efl_thread_3_SOURCES = efl_thread_3.c
efl_thread_4_SOURCES = efl_thread_4.c

if HAVE_CXX
bg_cxx_example_01_SOURCES = bg_cxx_example_01.cc
bg_cxx_example_02_SOURCES = bg_cxx_example_02.cc
box_cxx_example_02_SOURCES = box_cxx_example_02.cc
button_cxx_example_00_SOURCES = button_cxx_example_00.cc
button_cxx_example_01_SOURCES = button_cxx_example_01.cc
calendar_cxx_example_01_SOURCES = calendar_cxx_example_01.cc
calendar_cxx_example_02_SOURCES = calendar_cxx_example_02.cc
calendar_cxx_example_03_SOURCES = calendar_cxx_example_03.cc
calendar_cxx_example_04_SOURCES = calendar_cxx_example_04.cc
calendar_cxx_example_05_SOURCES = calendar_cxx_example_05.cc
clock_cxx_example_SOURCES = clock_cxx_example.cc
icon_cxx_example_01_SOURCES = icon_cxx_example_01.cc
menu_cxx_example_01_SOURCES = menu_cxx_example_01.cc
radio_cxx_example_01_SOURCES = radio_cxx_example_01.cc
popup_cxx_example_SOURCES = popup_cxx_example.cc
slider_cxx_example_SOURCES = slider_cxx_example.cc
spinner_cxx_example_SOURCES = spinner_cxx_example.cc
table_cxx_example_01_SOURCES = table_cxx_example_01.cc
table_cxx_example_02_SOURCES = table_cxx_example_02.cc
toolbar_cxx_example_01_SOURCES = toolbar_cxx_example_01.cc
endif

# This variable will hold the list of screenshots that will be made
# by "make screenshots". Each item in the list is of the form:
# <example executable>:<screenshot filename>:<delay in seconds>
SCREENSHOTS = \
actionslider_example_01:actionslider_01.png:0.0 \
bg_example_02:bg_01.png:0.0 \
box_example_02:box_example_02.png:1.3 \
bubble_example_01:bubble_example_01.png:0.0 \
button_example_00:button_00.png:0.0 \
button_example_01:button_01.png:0.0 \
check_example_01:check_example_01.png:0.0 \
colorselector_example_01:colorselector_example_01.png:0.0 \
animator_example_01:animator_example_01.png:0.2 \
animator_example_01:animator_example_02.png:0.5 \
animator_example_01:animator_example_03.png:0.9 \
flip_example_01:flip_example_01.png:0.0 \
frame_example_01:frame_example_01.png:0.0 \
hover_example_01:hover_example_01.png:0.0 \
hoversel_example_01:hoversel_example_01.png:0.0 \
label_example_01:label_example_01.png:0.0 \
theme_example_01:theme_example_01.png:0.0 \
conformant_example_01:conformant_example_01.png:0.0 \
conformant_example_02:conformant_example_02.png:0.0 \
combobox_example_01:combobox_example_01.png:0.0 \
calendar_example_01:calendar_example_01.png:0.0 \
calendar_example_02:calendar_example_02.png:0.0 \
calendar_example_03:calendar_example_03.png:0.0 \
calendar_example_04:calendar_example_04.png:0.0 \
calendar_example_05:calendar_example_05.png:0.0 \
calendar_example_06:calendar_example_06.png:0.0 \
datetime_example:datetime_example.png:0.0 \
map_example_01:map_example_01.png:2 \
map_example_02:map_example_02.png:2.5 \
map_example_03:map_example_03.png:2 \
toolbar_example_01:toolbar_example_01.png:0.0 \
toolbar_example_02:toolbar_example_02.png:1 \
toolbar_example_03:toolbar_example_03.png:1 \
spinner_example:spinner_example.png:0.0 \
clock_example:clock_example.png:0.5 \
dayselector_example:dayselector_example.png:0.0 \
mapbuf_example:mapbuf_example.png:0.0 \
image_example_01:image_example_01.png:0.0 \
diskselector_example_01:diskselector_example_01.png:0.2 \
diskselector_example_02:diskselector_example_02.png:0.2 \
icon_example_01:icon_example_01.png:0.0 \
layout_example_01:layout_example_01.png:0.0 \
layout_example_02:layout_example_02.png:0.0 \
layout_example_03:layout_example_03.png:0.0 \
list_example_01:list_example_01.png:0.0 \
list_example_02:list_example_02.png:0.0 \
list_example_03:list_example_03.png:0.0 \
segment_control_example:segment_control_example.png:0.0 \
flipselector_example:flipselector_example.png:0.0 \
fileselector_example:fileselector_example.png:0.0 \
index_example_02:index_example_03.png:0.3 \
slider_example:slider_example.png:0.0 \
panes_example:panes_example.png:0.0 \
ctxpopup_example_01:ctxpopup_example_01.png:0.0 \
separator_example_01:separator_example_01.png:0.0 \
radio_example_01:radio_example_01.png:0.0 \
radio_example_02:radio_example_02.png:0.0 \
panel_example_01:panel_example_01.png:0.0 \
gengrid_example:gengrid_example.png:0.0 \
genlist_example_01:genlist_example_01.png:0.1 \
genlist_example_02:genlist_example_02.png:0.1 \
genlist_example_03:genlist_example_03.png:0.1 \
genlist_example_04:genlist_example_04.png:0.1 \
genlist_example_05:genlist_example_05.png:0.1 \
thumb_example_01:thumb_example_01.png:0.5 \
entry_example:entry_example.png:0.0 \
progressbar_example:progressbar_example.png:0.0 \
notify_example_01:notify_example_01.png:0.0 \
notify_example_01:notify_example_01_a.png:6.0 \
popup_example_01:popup_example_01.png:1.0 \
popup_example_01:popup_example_01_a.png:6.0 \
popup_example_02:popup_example_02.png:0.2 \
popup_example_03:popup_example_03.png:0.2 \
slideshow_example:slideshow_example.png:1.0 \
photocam_example_01:photocam_example_01.png:3 \
scroller_example_01:scroller_example_01.png:0.0 \
inwin_example:inwin_example.png:0.0 \
inwin_example:inwin_example_a.png:0.2 \
table_example_01:table_example_01.png:0.0 \
table_example_02:table_example_02.png:0.0 \
menu_example_01:menu_example_01.png:0.5

if HAVE_CXX
SCREENSHOTS += \
 bg_cxx_example_01:bg_cxx_example_01.png:0.0 \
 bg_cxx_example_02:bg_cxx_example_02.png:0.0 \
 box_cxx_example_02:box_cxx_example_02.png:1.3 \
 button_cxx_example_00:button_cxx_example_00.png:0.0 \
 button_cxx_example_01:button_cxx_example_01.png:0.0 \
 calendar_cxx_example_01:calendar_cxx_example_01.png:0.0 \
 calendar_cxx_example_02:calendar_cxx_example_02.png:0.0 \
 calendar_cxx_example_03:calendar_cxx_example_03.png:0.0 \
 calendar_cxx_example_04:calendar_cxx_example_04.png:0.0 \
 calendar_cxx_example_05:calendar_cxx_example_05.png:0.0 \
 clock_cxx_example:clock_cxx_example.png:0.5 \
 icon_cxx_example_01:icon_cxx_example_01.png:0.0 \
 menu_cxx_example_01:menu_cxx_example_01.png:0.5 \
 popup_cxx_example_01:popup_cxx_example_01_a.png:6.0 \
 radio_cxx_example_01:radio_cxx_example_01.png:0.0 \
 slider_cxx_example:slider_cxx_example.png:0.0 \
 spinner_cxx_example:spinner_cxx_example.png:0.0 \
 table_cxx_example_01:table_cxx_example_01.png:0.0 \
 table_cxx_example_02:table_cxx_example_02.png:0.0 \
 toolbar_cxx_example_01:toolbar_cxx_example_01.png:0.0 \
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

EXTRA_DIST = dict.txt layout_example.edc theme_example.edc codegen_example.edc evas3d_map_example.edc twitter_example_01.edc efl_ui_view_list_example.edc performance/layout.edc performance/background.png performance/target_texture.png sphere_hunter/sphere_hunter.edc sphere_hunter/score.jpg sphere_hunter/EFL_victory.png sphere_hunter/EFL_defeat.png prefs_example_01.epc prefs_example_02.epc prefs_example_03.epc prefs_example_03.edc

if HAVE_CSHARP
EXTRA_DIST += $(MONO_SRCS)
endif

examples: $(examples_PROGRAMS) $(EDJS)

install-examples:
	$(MKDIR_P) $(DESTDIR)$(datadir)/elementary/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(SRCS) $(DESTDIR)$(datadir)/elementary/examples

uninstall-local:
	for f in $(SRCS); do \
	  rm -f $(DESTDIR)$(datadir)/elementary/examples/$$f ; \
	done
