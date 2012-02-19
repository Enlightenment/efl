MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = @edje_cc@
EDJE_CC_FLAGS_VERBOSE_0 =
EDJE_CC_FLAGS_VERBOSE_1 = -v
EDJE_CC_FLAGS = $(EDJE_CC_FLAGS_$(V)) -id $(srcdir) -fd $(srcdir)

examplesdir = $(datadir)/$(PACKAGE)/examples

if ENABLE_MULTISENSE
MULTISENSE_EDC_FILE = multisense.edc
SND_DIR = -sd $(srcdir)
endif

#put here all EDCs one needs to the examples
EDCS = \
        basic.edc \
	swallow.edc \
	text.edc \
	table.edc \
	box.edc \
	drag.edc \
	signals-messages.edc \
	color-class.edc \
	perspective.edc \
	animations.edc \
	lua_script.edc \
	sigtest.edc \
	$(MULTISENSE_EDC_FILE)

.edc.edj:
	$(EDJE_CC) $(EDJE_CC_FLAGS) $(SND_DIR) $< $(builddir)/$(@F)


EDJS = $(EDCS:%.edc=%.edj)

filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA = \
        $(EDCS) \
        bubble.png \
	red.png \
	test.png \
	Vera.ttf \
	edje-basic.c \
	edje-swallow.c \
	edje-text.c \
	edje-table.c \
	edje-box.c \
	edje-box2.c \
	edje-drag.c \
	edje-signals-messages.c \
	edje-color-class.c \
	edje-perspective.c \
	edje-animations.c \
	sigtest.c

EXTRA_DIST = $(files_DATA)

if BUILD_EXAMPLES

AM_CPPFLAGS = \
        -I. \
        -I$(top_srcdir)/src/lib \
        -I$(top_srcdir)/src/lib/include \
        -DPACKAGE_BIN_DIR=\"$(bindir)\" \
        -DPACKAGE_LIB_DIR=\"$(libdir)\" \
        -DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
        -DPACKAGE_EXAMPLES_DIR=\"$(datadir)/$(PACKAGE)/examples\" \
        @EDJE_CFLAGS@

files_DATA += $(EDJS)

examples_PROGRAMS = \
	edje-animations \
	edje-basic \
	edje-box \
	edje-box2 \
	edje-color-class \
	edje-drag\
	edje-perspective \
	edje-signals-messages \
	edje-swallow \
	edje-table \
	edje-text \
	sigtest

LDADD = $(top_builddir)/src/lib/libedje.la @EDJE_LIBS@

endif

clean-local:
	rm -f *.edj
