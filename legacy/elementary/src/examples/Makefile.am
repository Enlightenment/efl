MAINTAINERCLEANFILES = Makefile.in

include ../../Makefile_Elm_Helpers.am

.PHONY: screenshots

examplesdir = $(pkgdatadir)/examples
filesdir = $(pkgdatadir)/examples
files_DATA =

AM_CPPFLAGS = \
-Wno-unused-parameter \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
-DPACKAGE_DATA_DIR="\"$(pkgdatadir)\"" \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
@ELEMENTARY_ELOCATION_CFLAGS@ \
@ELEMENTARY_EWEATHER_CFLAGS@ \
@ELEMENTARY_EMAP_CFLAGS@ \
@ELEMENTARY_X_CFLAGS@ \
@ELEMENTARY_CFLAGS@

LDADD = \
@ELEMENTARY_LIBS@ \
@ELEMENTARY_ELOCATION_LIBS@ \
@ELEMENTARY_EMAP_LIBS@ \
@ELEMENTARY_X_LIBS@ \
$(top_builddir)/src/lib/libelementary.la

ELEMENTARY_CODEGEN = $(top_builddir)/src/bin/@ELEMENTARY_CODEGEN_PRG@
ELEMENTARY_CODEGEN_FLAGS = --prefix=codegen_example

SRCS = \
actionslider_example_01.c \
bg_example_01.c \
bg_example_02.c \
bg_example_03.c \
box_example_02.c \
box_cxx_example_02.cc \
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
codegen_example.edc \
colorselector_example_01.c \
conformant_example_01.c \
conformant_example_02.c \
ctxpopup_example_01.c \
datetime_example.c \
dayselector_example.c \
diskselector_example_01.c \
diskselector_example_02.c \
efl_thread_1.c \
efl_thread_2.c \
efl_thread_3.c \
efl_thread_4.c \
efl_thread_5.c \
efl_thread_6.c \
efl_thread_win32_1.c \
efl_thread_win32_2.c \
efl_thread_win32_3.c \
efl_thread_win32_4.c \
entry_example.c \
fileselector_button_example.c \
fileselector_entry_example.c \
fileselector_example.c \
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
glview_example_01.c \
hover_example_01.c \
hoversel_example_01.c \
icon_example_01.c \
image_example_01.c \
index_example_01.c \
index_example_02.c \
inwin_example.c \
label_example_01.c \
layout_example.edc \
layout_example_01.c \
layout_example_02.c \
layout_example_03.c \
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
segment_control_example.c \
separator_example_01.c \
slider_example.c \
slideshow_example.c \
spinner_example.c \
scroller_example_01.c \
table_example_01.c \
table_example_02.c \
theme_example.edc \
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
evas3d_scene_on_button_example.c \
evas3d_object_on_button_example.c \
evas3d_map_example.edc \
evas3d_map_example.c \
track_example_01.c

.edc.edj:
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) $< $@

.epc.epb:
	$(AM_V_EPB)$(ELM_PREFS_CC) $< $@

if EFL_INSTALL_EXAMPLES
files_DATA += $(SRCS) \
	dict.txt
endif

if EFL_BUILD_EXAMPLES
files_DATA += \
	prefs_example_01.epb \
	prefs_example_02.epb \
	prefs_example_03.epb \
	prefs_example_03.edj \
	theme_example.edj \
	layout_example.edj\
	codegen_example.edj \
   evas3d_map_example.edj

clean-local:
	rm -f *.edj *.epb *.cfg *.cfg.bkp

codegen_example_generated.c codegen_example_generated.h: codegen_example.edj
	$(ELEMENTARY_CODEGEN) $(ELEMENTARY_CODEGEN_FLAGS) codegen_example.edj \
	elm/example/mylayout/default codegen_example_generated.c codegen_example_generated.h

BUILT_SOURCES = codegen_example_generated.c codegen_example_generated.h

codegen_example_SOURCES = \
	codegen_example.c
nodist_codegen_example_SOURCES = \
codegen_example_generated.c \
codegen_example_generated.h

examples_PROGRAMS = \
actionslider_example_01 \
bg_example_01 \
bg_example_02 \
bg_example_03 \
box_example_02 \
box_cxx_example_02 \
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
ctxpopup_example_01 \
datetime_example \
dayselector_example \
diskselector_example_01 \
diskselector_example_02 \
efl_thread_1 \
efl_thread_2 \
efl_thread_3 \
efl_thread_4 \
efl_thread_5 \
efl_thread_6 \
entry_example \
fileselector_button_example \
fileselector_entry_example \
fileselector_example \
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
glview_example_01 \
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
evas3d_scene_on_button_example \
evas3d_object_on_button_example \
evas3d_map_example \
track_example_01

if ELEMENTARY_WINDOWS_BUILD
efl_thread_1_SOURCES = efl_thread_win32_1.c
efl_thread_2_SOURCES = efl_thread_win32_2.c
efl_thread_3_SOURCES = efl_thread_win32_3.c
efl_thread_4_SOURCES = efl_thread_win32_4.c
else
efl_thread_1_SOURCES = efl_thread_1.c
efl_thread_2_SOURCES = efl_thread_2.c
efl_thread_3_SOURCES = efl_thread_3.c
efl_thread_4_SOURCES = efl_thread_4.c
endif

box_cxx_example_02_SOURCES = box_cxx_example_02.cc
box_cxx_example_02_CXXFLAGS = \
@ELEMENTARY_ELOCATION_CFLAGS@ \
@ELEMENTARY_EWEATHER_CFLAGS@ \
@ELEMENTARY_EMAP_CFLAGS@ \
@ELEMENTARY_X_CFLAGS@ \
@ELEMENTARY_CFLAGS@ \
@ELEMENTARY_CXX_CFLAGS@

# This variable will hold the list of screenshots that will be made
# by "make screenshots". Each item in the list is of the form:
# <example executable>:<screenshot filename>:<delay in seconds>
SCREENSHOTS = \
actionslider_example_01:actionslider_01.png:0.0 \
bg_example_02:bg_01.png:0.0 \
box_example_02:box_example_02.png:1.3 \
box_cxx_example_02:box_cxx_example_02.png:1.3 \
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

HTML_SS_DIR=$(top_builddir)/doc/html/screenshots
LATEX_SS_DIR=$(top_builddir)/doc/latex/screenshots
screenshots: all
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

else

screenshots:
	@echo "Examples are not built. Run \"./configure --enable-build-examples\" first."

endif

EXTRA_DIST = dict.txt layout_example.edc theme_example.edc codegen_example.edc evas3d_map_example.edc
