MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = \
op_blend_color_.c \
op_blend_color_i386.c \
op_blend_color_neon.c \
op_blend_color_sse3.c \
op_blend_mask_color_.c \
op_blend_mask_color_i386.c \
op_blend_mask_color_neon.c \
op_blend_mask_color_sse3.c \
op_blend_pixel_.c \
op_blend_pixel_color_.c \
op_blend_pixel_color_i386.c \
op_blend_pixel_color_neon.c \
op_blend_pixel_color_sse3.c \
op_blend_pixel_i386.c \
op_blend_pixel_mask_.c \
op_blend_pixel_mask_i386.c \
op_blend_pixel_mask_neon.c \
op_blend_pixel_mask_sse3.c \
op_blend_pixel_neon.c \
op_blend_pixel_sse3.c

noinst_LTLIBRARIES      = libevas_engine_common_op_blend_master_sse3.la

libevas_engine_common_op_blend_master_sse3_la_SOURCES = \
op_blend_master_sse3.c

libevas_engine_common_op_blend_master_sse3_la_CFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/engines/common \
-I$(top_srcdir)/src/lib/engines/common/evas_op_blend \
-I$(top_srcdir)/src/lib/cserve \
-I$(top_srcdir)/src/lib/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@FREETYPE_CFLAGS@ @VALGRIND_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@EET_CFLAGS@ @pthread_cflags@ \
@WIN32_CFLAGS@ @EINA_CFLAGS@ \
@FRIBIDI_CFLAGS@ @HARFBUZZ_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@EVAS_SSE3_CFLAGS@
                       
libevas_engine_common_op_blend_master_sse3_la_DEPENENCIES = \
$(top_builddir)/config.h

