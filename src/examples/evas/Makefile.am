MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/evas \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/evas \
@EVAS_CFLAGS@

#put here all EDCs one needs to the tests
EDCS = aspect.edc

EXTRA_PROGRAMS =

if BUILD_ENGINE_BUFFER
AM_CPPFLAGS += -I$(top_srcdir)/src/modules/evas/engines/buffer

EXTRA_PROGRAMS += evas_buffer_simple
evas_buffer_simple_SOURCES = evas-buffer-simple.c
evas_buffer_simple_LDADD = $(top_builddir)/src/lib/evas/libevas.la @EVAS_LDFLAGS@
endif

EXTRA_PROGRAMS += evas_init_shutdown
evas_init_shutdown_SOURCES = evas-init-shutdown.c
evas_init_shutdown_LDADD = $(top_builddir)/src/lib/evas/libevas.la @EVAS_LDFLAGS@

ECORE_EVAS_COMMON_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/evas \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/evas \
-I$(top_builddir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore_evas \
-DPACKAGE_EXAMPLES_DIR=\".\" \
@EVAS_CFLAGS@

ECORE_EVAS_COMMON_LDADD = \
$(top_builddir)/src/lib/eina/libeina.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
$(top_builddir)/src/lib/ecore_evas/libecore_evas.la \
$(top_builddir)/src/lib/evas/libevas.la \
@EVAS_LDFLAGS@

EXTRA_PROGRAMS += evas_images
evas_images_SOURCES = evas-images.c
evas_images_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_images_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_events
evas_events_SOURCES = evas-events.c
evas_events_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_events_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_object_manipulation
evas_object_manipulation_SOURCES = evas-object-manipulation.c
evas_object_manipulation_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_object_manipulation_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_object_manipulation-eo
evas_object_manipulation_eo_SOURCES = evas-object-manipulation-eo.c
evas_object_manipulation_eo_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_object_manipulation_eo_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_hints
evas_hints_SOURCES = evas-hints.c
evas_hints_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_hints_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_stacking
evas_stacking_SOURCES = evas-stacking.c
evas_stacking_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_stacking_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_images2
evas_images2_SOURCES = evas-images2.c
evas_images2_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_images2_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_images3
evas_images3_SOURCES = evas-images3.c
evas_images3_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_images3_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_text
evas_text_SOURCES = evas-text.c
evas_text_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_text_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_smart_object
evas_smart_object_SOURCES = evas-smart-object.c
evas_smart_object_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_smart_object_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_smart_interface
evas_smart_interface_SOURCES = evas-smart-interface.c
evas_smart_interface_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_smart_interface_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

EXTRA_PROGRAMS += evas_box
evas_box_SOURCES = evas-box.c
evas_box_LDADD = $(ECORE_EVAS_COMMON_LDADD)
evas_box_CPPFLAGS = $(ECORE_EVAS_COMMON_CPPFLAGS)

#the ones using ecore_evas and edje follow
#AM_CPPFLAGS += @EDJE_CFLAGS@

#EXTRA_PROGRAMS += evas_aspect_hints
#evas_aspect_hints_DEPS = $(srcdir)/aspect.edc
#evas_aspect_hints_SOURCES = evas-aspect-hints.c
#evas_aspect_hints_LDADD = $(top_builddir)/src/lib/evas/libevas.la @ECORE_EVAS_LIBS@ @EDJE_LIBS@

#EXTRA_PROGRAMS += evas_map_utils
#evas_map_utils_SOURCES = evas-map-utils.c
#evas_map_utils_LDADD = $(top_builddir)/src/lib/evas/libevas.la @ECORE_EVAS_LIBS@

#aspect.edj: ${evas_aspect_hints_DEPS}

#.edc.edj:
#	$(edje_cc) -v -id $(srcdir) $< $(builddir)/$(@F)

#EDJS = $(EDCS:%.edc=%.edj)

#SRCS = $(EDJS)

#SRCS += $(EDCS)
SRCS = $(EDCS)

SRCS += \
enlightenment.png \
red.png \
im1.png \
cube1.png

SRCS += \
evas-buffer-simple.c \
evas-init-shutdown.c \
evas-images.c \
evas-images2.c \
evas-images3.c \
evas-object-manipulation.c \
evas-object-manipulation-eo.c \
evas-events.c \
evas-aspect-hints.c \
evas-hints.c \
evas-stacking.c \
evas-map-utils.c \
evas-text.c \
evas-smart-object.c \
evas-smart-interface.c \
evas-box.c

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	mkdir -p $(datadir)/evas/examples
	$(install_sh_DATA) -c $(SRCS) $(datadir)/evas/examples

uninstall-local:
	for f in $(SRCS) ; do \
	  rm -f $(datadir)/evas/examples/$$f ; \
	done

EXTRA_DIST = \
$(EDCS) \
enlightenment.png \
red.png \
im1.png \
cube1.png
