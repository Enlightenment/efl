MAINTAINERCLEANFILES = Makefile.in

.PHONY: screenshots

pkglibdir = $(datadir)/$(PACKAGE)/examples
filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA =

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
-DPACKAGE_DATA_DIR="\"$(datadir)/elementary\"" \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
@ELEMENTARY_EDBUS_CFLAGS@ \
@ELEMENTARY_EFREET_CFLAGS@ \
@ELEMENTARY_ETHUMB_CFLAGS@ \
@ELEMENTARY_CFLAGS@

LDADD = \
	@ELEMENTARY_LIBS@ \
	$(top_builddir)/src/lib/libelementary.la

EDJE_CC = @edje_cc@
EDJE_FLAGS_VERBOSE_ =
EDJE_FLAGS_VERBOSE_0 =
EDJE_FLAGS_VERBOSE_1 = -v
EDJE_FLAGS = $(EDJE_FLAGS_$(V))

SRCS = \
	actionslider_example_01.c \
	anchorblock_example_01.c \
	animator_example_01.c \
	bubble_example_01.c \
	button_example_01.c \
	check_example_01.c \
	colorselector_example_01.c
	frame_example_01.c \
	transit_example_01.c \
	transit_example_02.c \
	transit_example_03.c \
	transit_example_04.c \
	bg_example_01.c \
	bg_example_02.c \
	bg_example_03.c \
	box_example_02.c \
	flip_example_01.c \
	general_funcs_example.c \
	hover_example_01.c \
	hoversel_example_01.c \
	label_example_01.c \
	theme_example_01.c \
	theme_example_02.c \
	calendar_example_01.c \
	calendar_example_02.c \
	calendar_example_03.c \
	calendar_example_04.c \
	calendar_example_05.c \
	calendar_example_06.c \
	clock_example.c \
	conformant_example_01.c \
	conformant_example_02.c \
	image_example_01.c \
	icon_example_01.c \
	flipselector_example.c \
	fileselector_example.c \
	fileselector_button_example.c \
	fileselector_entry_example.c \
	diskselector_example_01.c \
	diskselector_example_02.c \
	theme_example.edc

pkglib_PROGRAMS =

.edc.edj:
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

if EFL_INSTALL_EXAMPLES
files_DATA += $(SRCS)
endif

if EFL_BUILD_EXAMPLES
files_DATA += \
	theme_example.edj

clean-local:
	rm -f *.edj

pkglib_PROGRAMS += \
	actionslider_example_01 \
	anchorblock_example_01 \
	animator_example_01 \
	bubble_example_01 \
	button_example_01 \
	check_example_01 \
	colorselector_example_01 \
	frame_example_01 \
	transit_example_01 \
	transit_example_02 \
	transit_example_03 \
	transit_example_04 \
	bg_example_01 \
	bg_example_02 \
	bg_example_03 \
	box_example_02 \
	flip_example_01 \
	general_funcs_example \
	hover_example_01 \
	hoversel_example_01 \
	label_example_01 \
	theme_example_01 \
	theme_example_02 \
	calendar_example_01 \
	calendar_example_02 \
	calendar_example_03 \
	calendar_example_04 \
	calendar_example_05 \
	calendar_example_06 \
	clock_example \
	conformant_example_01 \
	conformant_example_02 \
	image_example_01 \
	diskselector_example_01 \
	diskselector_example_02 \
	icon_example_01 \
	flipselector_example \
	fileselector_example \
	fileselector_button_example \
	fileselector_entry_example

# This variable will hold the list of screenshots that will be made
# by "make screenshots". Each item in the list is of the form:
# <example executable>:<screenshot filename>:<delay in seconds>
SCREENSHOTS = \
	actionslider_example_01:actionslider_01.png:0.0 \
	bg_example_02:bg_01.png:0.0 \
	box_example_02:box_example_02.png:1.3 \
	bubble_example_01:bubble_example_01.png:0.0 \
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
	calendar_example_01:calendar_example_01.png:0.0 \
	calendar_example_02:calendar_example_02.png:0.0 \
	calendar_example_03:calendar_example_03.png:0.0 \
	calendar_example_04:calendar_example_04.png:0.0 \
	calendar_example_05:calendar_example_05.png:0.0 \
	calendar_example_06:calendar_example_06.png:0.0 \
	clock_example:clock_example.png:0.5 \
	image_example_01:image_example_01.png:0.0 \
	diskselector_example_01:diskselector_example_01.png:0.2 \
	diskselector_example_02:diskselector_example_02.png:0.2 \
	icon_example_01:icon_example_01.png:0.0 \
	flipselector_example:flipselector_example.png:0.0 \
	fileselector_example:fileselector_example.png:0.0

screenshots: all
	@mkdir -p $(top_srcdir)/doc/img/screenshots
	@for ss in $(SCREENSHOTS); do \
		SS_ENTRY=($${ss//:/ }) ; \
		EXAMPLE=$${SS_ENTRY[0]} ; \
		SS_FILE=$${SS_ENTRY[1]} ; \
		SS_DELAY=$${SS_ENTRY[2]} ; \
		ELM_ENGINE="shot:delay=$${SS_DELAY}:file=$(top_srcdir)/doc/img/screenshots/$${SS_FILE}" ./$${EXAMPLE} ; \
		convert $(top_srcdir)/doc/img/screenshots/$${SS_FILE} $(top_srcdir)/doc/img/screenshots/$${SS_FILE/.png/.eps} ; \
	done

else

screenshots:
	@echo "Examples are not built. Run \"./configure --enable-build-examples\" first."

endif
