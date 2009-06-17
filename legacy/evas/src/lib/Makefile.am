MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = canvas cache cserve file engines imaging include
EVAS_STATIC_MODULE =
EVAS_STATIC_LIBADD =

if EVAS_STATIC_BUILD_SOFTWARE_GENERIC
SUBDIRS += ../modules/engines/software_generic/
EVAS_STATIC_MODULE += ../modules/engines/software_generic/libevas_engine_software_generic.la
endif
if EVAS_STATIC_BUILD_BUFFER
SUBDIRS += ../modules/engines/buffer/
EVAS_STATIC_MODULE += ../modules/engines/buffer/libevas_engine_buffer.la
endif
if EVAS_STATIC_BUILD_DIRECT3D
SUBDIRS += ../modules/engines/direct3d/
EVAS_STATIC_MODULE += ../modules/engines/direct3d/libevas_engine_direct3d.la
EVAS_STATIC_LIBADD += @evas_engine_direct3d_libs@
endif
if EVAS_STATIC_BUILD_DIRECTFB
SUBDIRS += ../modules/engines/directfb/
EVAS_STATIC_MODULE += ../modules/engines/directfb/libevas_engine_directfb.la
EVAS_STATIC_LIBADD += @evas_engine_directfb_libs@
endif
if EVAS_STATIC_BUILD_FB
SUBDIRS += ../modules/engines/fb/
EVAS_STATIC_MODULE += ../modules/engines/fb/libevas_engine_fb.la
EVAS_STATIC_LIBADD += @evas_engine_fb_libs@
endif
if EVAS_STATIC_BUILD_GL_GLEW
SUBDIRS += \
	../modules/engines/gl_common \
	../modules/engines/gl_glew
EVAS_STATIC_MODULE += \
	../modules/engines/gl_common/libevas_engine_gl_common.la \
	../modules/engines/gl_glew/libevas_engine_gl_glew.la
EVAS_STATIC_LIBADD += @evas_engine_gl_common_libs@ @evas_engine_gl_glew_libs@
endif
if EVAS_STATIC_BUILD_GL_X11
SUBDIRS += \
	../modules/engines/gl_common \
	../modules/engines/gl_x11
EVAS_STATIC_MODULE += \
	../modules/engines/gl_common/libevas_engine_gl_common.la \
	../modules/engines/gl_x11/libevas_engine_gl_x11.la
EVAS_STATIC_LIBADD += @evas_engine_gl_common_libs@ @evas_engine_gl_x11_libs@
endif
if EVAS_STATIC_BUILD_QUARTZ
SUBDIRS += ../modules/engines/quartz/
EVAS_STATIC_MODULE += ../modules/engines/quartz/libevas_engine_quartz.la
EVAS_STATIC_LIBADD += @evas_engine_quartz_libs@
endif
if EVAS_STATIC_BUILD_SOFTWARE_16
SUBDIRS += ../modules/engines/software_16/
EVAS_STATIC_MODULE += ../modules/engines/software_16/libevas_engine_software_16.la
endif
if EVAS_STATIC_BUILD_SOFTWARE_16_DDRAW
SUBDIRS += ../modules/engines/software_16_ddraw/
EVAS_STATIC_MODULE += ../modules/engines/software_16_ddraw/libevas_engine_software_16_ddraw.la
EVAS_STATIC_LIBADD += @evas_engine_software_16_ddraw_libs@
endif
if EVAS_STATIC_BUILD_SOFTWARE_16_WINCE
SUBDIRS += ../modules/engines/software_16_wince/
EVAS_STATIC_MODULE += ../modules/engines/software_16_wince/libevas_engine_software_16_wince.la
EVAS_STATIC_LIBADD += @evas_engine_software_16_wince_libs@
endif
if EVAS_STATIC_BUILD_SOFTWARE_16_X11
SUBDIRS += ../modules/engines/software_16_x11/
EVAS_STATIC_MODULE += ../modules/engines/software_16_x11/libevas_engine_software_16_x11.la
EVAS_STATIC_LIBADD += @evas_engine_software_16_x11_libs@
endif
if EVAS_STATIC_BUILD_SOFTWARE_SDL
SUBDIRS += \
	../modules/engines/software_16_sdl/ \
	../modules/engines/software_sdl/
EVAS_STATIC_MODULE += \
	../modules/engines/software_16_sdl/libevas_engine_software_16_sdl.la \
	../modules/engines/software_sdl/libevas_engine_software_sdl.la
EVAS_STATIC_LIBADD += @SDL_LIBS@
endif
if EVAS_STATIC_BUILD_SOFTWARE_DDRAW
SUBDIRS += ../modules/engines/software_ddraw/
EVAS_STATIC_MODULE += ../modules/engines/software_ddraw/libevas_engine_software_ddraw.la
EVAS_STATIC_LIBADD += @evas_engine_software_ddraw_libs@
endif
if EVAS_STATIC_BUILD_SOFTWARE_GDI
SUBDIRS += ../modules/engines/software_gdi/
EVAS_STATIC_MODULE += ../modules/engines/software_gdi/libevas_engine_software_gdi.la
EVAS_STATIC_LIBADD += @evas_engine_software_gdi_libs@
endif
if EVAS_STATIC_BUILD_SOFTWARE_X11
SUBDIRS += ../modules/engines/software_x11/
EVAS_STATIC_MODULE += ../modules/engines/software_x11/libevas_engine_software_x11.la
EVAS_STATIC_LIBADD += @evas_engine_software_x11_libs@ @evas_engine_software_xcb_libs@
endif
if EVAS_STATIC_BUILD_XRENDER_X11
SUBDIRS += ../modules/engines/xrender_x11/
EVAS_STATIC_MODULE += ../modules/engines/xrender_x11/libevas_engine_xrender_x11.la
EVAS_STATIC_LIBADD += @evas_engine_xrender_x11_libs@ @evas_engine_xrender_xcb_libs@
endif
if EVAS_STATIC_BUILD_EDB
SUBDIRS += ../modules/savers/edb ../modules/loaders/edb
EVAS_STATIC_MODULE += ../modules/savers/edb/libevas_saver_edb.la ../modules/loaders/edb/libevas_loader_edb.la
EVAS_STATIC_LIBADD += @evas_image_loader_edb_libs@
endif
if EVAS_STATIC_BUILD_EET
SUBDIRS += ../modules/savers/eet ../modules/loaders/eet
EVAS_STATIC_MODULE += ../modules/savers/eet/libevas_saver_eet.la ../modules/loaders/eet/libevas_loader_eet.la
EVAS_STATIC_LIBADD += @evas_image_loader_eet_libs@
endif
if EVAS_STATIC_BUILD_GIF
SUBDIRS += ../modules/loaders/gif
EVAS_STATIC_MODULE += ../modules/loaders/gif/libevas_loader_gif.la
EVAS_STATIC_LIBADD += @evas_image_loader_gif_libs@
endif
if EVAS_STATIC_BUILD_JPEG
SUBDIRS += ../modules/savers/jpeg ../modules/loaders/jpeg
EVAS_STATIC_MODULE += ../modules/savers/jpeg/libevas_saver_jpeg.la ../modules/loaders/jpeg/libevas_loader_jpeg.la
EVAS_STATIC_LIBADD += @evas_image_loader_jpeg_libs@
endif
if EVAS_STATIC_BUILD_PMAPS
SUBDIRS += ../modules/loaders/pmaps
EVAS_STATIC_MODULE += ../modules/loaders/pmaps/libevas_loader_pmaps.la
EVAS_STATIC_LIBADD += @evas_image_loader_pmaps_libs@
endif
if EVAS_STATIC_BUILD_PNG
SUBDIRS += ../modules/savers/png ../modules/loaders/png
EVAS_STATIC_MODULE += ../modules/savers/png/libevas_saver_png.la ../modules/loaders/png/libevas_loader_png.la
EVAS_STATIC_LIBADD += @evas_image_loader_png_libs@
endif
if EVAS_STATIC_BUILD_SVG
SUBDIRS += ../modules/loaders/svg
EVAS_STATIC_MODULE += ../modules/loaders/svg/libevas_loader_svg.la
EVAS_STATIC_LIBADD += @evas_image_loader_svg_libs@
endif
if EVAS_STATIC_BUILD_TIFF
SUBDIRS += ../modules/savers/tiff ../modules/loaders/tiff
EVAS_STATIC_MODULE += ../modules/savers/tiff/libevas_saver_tiff.la ../modules/loaders/tiff/libevas_loader_tiff.la
EVAS_STATIC_LIBADD += @evas_image_loader_tiff_libs@
endif
if EVAS_STATIC_BUILD_XPM
SUBDIRS += ../modules/loaders/xpm
EVAS_STATIC_MODULE += ../modules/loaders/xpm/libevas_loader_xpm.la
EVAS_STATIC_LIBADD += @evas_image_loader_xpm_libs@
endif


#if EVAS_STATIC_BUILD_CAIRO_X11
#SUBDIRS += \
#	../modules/engines/cairo_common \
#	../modules/engines/cairo_x11
#EVAS_STATIC_MODULE += \
#	../modules/engines/cairo_common/libevas_engine_cairo_common.la \
#	../modules/engines/cairo_x11/libevas_engine_cairo_x11.la
#EVAS_STATIC_LIBADD += @CAIRO_CFLAGS@
#endif
#if EVAS_STATIC_BUILD_SOFTWARE_QTOPIA
#SUBDIRS += ../modules/engines/software_qtopia/
#EVAS_STATIC_MODULE += ../modules/engines/software_qtopia/libevas_engine_software_qtopia.la
#EVAS_STATIC_LIBADD += @evas_engine_software_qtopia_libs@
#endif

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@EET_CFLAGS@ \
@FONTCONFIG_CFLAGS@ \
@pthread_cflags@

AM_CFLAGS = @WIN32_CFLAGS@

lib_LTLIBRARIES = libevas.la

include_HEADERS = Evas.h

libevas_la_SOURCES = main.c

if EVAS_CSERVE

libevas_cserve_la = cserve/libevas_cserve.la

endif

libevas_la_LIBADD = \
canvas/libevas_canvas.la \
file/libevas_file.la \
cache/libevas_cache.la \
imaging/libevas_imaging.la \
$(libevas_cserve_la) \
engines/common/libevas_engine_common.la \
@dlopen_libs@ \
@FREETYPE_LIBS@ \
@FRIBIDI_LIBS@ \
@EET_LIBS@ \
@FONTCONFIG_LIBS@ \
@pthread_libs@ \
@EINA_LIBS@ \
$(EVAS_STATIC_MODULE) \
$(EVAS_STATIC_LIBADD) \
-lm

libevas_la_DEPENDENCIES = \
canvas/libevas_canvas.la \
file/libevas_file.la \
cache/libevas_cache.la \
imaging/libevas_imaging.la \
$(libevas_cserve_la) \
engines/common/libevas_engine_common.la \
$(EVAS_STATIC_MODULE)

if BUILD_ENGINE_SOFTWARE_16

libevas_la_LIBADD += engines/common_16/libevas_engine_common_16.la
libevas_la_DEPENDENCIES += engines/common_16/libevas_engine_common_16.la

endif

libevas_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@

DIST_SUBDIRS = canvas data cache cserve file engines imaging include
