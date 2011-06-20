MAINTAINERCLEANFILES = Makefile.in

.PHONY: screenshots

pkglibdir = $(datadir)/$(PACKAGE)/examples

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
-DPACKAGE_DATA_DIR="\"$(datadir)/elementary\"" \
@ELEMENTARY_EDBUS_CFLAGS@ \
@ELEMENTARY_EFREET_CFLAGS@ \
@ELEMENTARY_ETHUMB_CFLAGS@ \
@ELEMENTARY_CFLAGS@

LDADD = \
	@ELEMENTARY_LIBS@ \
	$(top_builddir)/src/lib/libelementary.la

SRCS = \
	actionslider_example_01.c \
	anchorblock_example_01.c \
	animator_example_01.c \
	bg_example_01.c \
	bg_example_02.c \
	bg_example_03.c

pkglib_PROGRAMS =

if EFL_INSTALL_EXAMPLES
filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA = $(SRCS)
endif

if EFL_BUILD_EXAMPLES
pkglib_PROGRAMS += \
	actionslider_example_01 \
	anchorblock_example_01 \
	animator_example_01 \
	bg_example_01 \
	bg_example_02 \
	bg_example_03

# This variable will hold the list of screenshots that will be made
# by "make screenshots". Each item in the list is of the form:
# <example executable>:<screenshot filename>:<delay in seconds>
SCREENSHOTS = \
	actionslider_example_01:actionslider_01.png:0.0 \
	bg_example_02:bg_01.png:0.0 \
	animator_example_01:animator_example_01.png:0.2 \
	animator_example_01:animator_example_02.png:0.5 \
	animator_example_01:animator_example_03.png:0.9

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
