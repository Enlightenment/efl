## Process this file with automake to produce Makefile.in

SUBDIRS = canvas data file engines imaging include

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = @freetype_cflags@ @x_cflags@ @qt_cflags@ @DIRECTFB_CFLAGS@ @gl_cflags@ \
                       -I. \
                       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include

lib_LTLIBRARIES      = libevas.la

EXTRA_DIST = \
Evas_Engine_Software_X11.h \
Evas_Engine_DirectFB.h \
Evas_Engine_FB.h \
Evas_Engine_Buffer.h \
Evas_Engine_Software_Win32_GDI.h \
Evas_Engine_Software_Qtopia.h \
Evas_Engine_GL_X11.h \
Evas_Engine_Cairo_X11.h

if BUILD_ENGINE_SOFTWARE_X11
ENGINE_SOFTWARE_X11 = engines/software_x11/libevas_engine_software_x11.la
ENGINE_SOFTWARE_X11_INC = Evas_Engine_Software_X11.h
else
ENGINE_SOFTWARE_X11 =
ENGINE_SOFTWARE_X11_INC =
endif

if BUILD_ENGINE_DIRECTFB
ENGINE_DIRECTFB = engines/directfb/libevas_engine_directfb.la
ENGINE_DIRECTFB_INC = Evas_Engine_DirectFB.h
else
ENGINE_DIRECTFB =
ENGINE_DIRECTFB_INC =
endif

if BUILD_ENGINE_FB
ENGINE_FB = engines/fb/libevas_engine_fb.la
ENGINE_FB_INC = Evas_Engine_FB.h
else
ENGINE_FB =
ENGINE_FB_INC =
endif

if BUILD_ENGINE_BUFFER
ENGINE_BUFFER = engines/buffer/libevas_engine_buffer.la
ENGINE_BUFFER_INC = Evas_Engine_Buffer.h
else
ENGINE_BUFFER =
ENGINE_BUFFER_INC =
endif

if BUILD_ENGINE_SOFTWARE_QTOPIA
ENGINE_SOFTWARE_QTOPIA = engines/software_qtopia/libevas_engine_software_qtopia.la
ENGINE_SOFTWARE_QTOPIA_INC = Evas_Engine_Software_Qtopia.h
else
ENGINE_SOFTWARE_QTOPIA =
ENGINE_SOFTWARE_QTOPIA_INC =
endif

if BUILD_ENGINE_GL_COMMON
ENGINE_GL_COMMON = engines/gl_common/libevas_engine_gl_common.la
ENGINE_GL_COMMON_INC = 
else
ENGINE_GL_COMMON =
ENGINE_GL_COMMON_INC =
endif

if BUILD_ENGINE_GL_X11
ENGINE_GL_X11 = engines/gl_x11/libevas_engine_gl_x11.la
ENGINE_GL_X11_INC = Evas_Engine_GL_X11.h
else
ENGINE_GL_X11 =
ENGINE_GL_X11_INC =
endif

if BUILD_ENGINE_CAIRO_COMMON
ENGINE_CAIRO_COMMON = engines/cairo_common/libevas_engine_cairo_common.la
ENGINE_CAIRO_COMMON_INC = 
else
ENGINE_CAIRO_COMMON =
ENGINE_CAIRO_COMMON_INC =
endif

if BUILD_ENGINE_CAIRO_X11
ENGINE_CAIRO_X11 = engines/cairo_x11/libevas_engine_cairo_x11.la
ENGINE_CAIRO_X11_INC = Evas_Engine_Cairo_X11.h
else
ENGINE_CAIRO_X11 =
ENGINE_CAIRO_X11_INC =
endif

include_HEADERS      = \
Evas.h \
$(ENGINE_SOFTWARE_X11_INC) \
$(ENGINE_DIRECTFB_INC) \
$(ENGINE_FB_INC) \
$(ENGINE_BUFFER_INC) \
$(ENGINE_SOFTWARE_QTOPIA_INC) \
$(ENGINE_GL_COMMON_INC) \
$(ENGINE_GL_X11_INC) \
$(ENGINE_CAIRO_COMMON_INC) \
$(ENGINE_CAIRO_X11_INC)

libevas_la_SOURCES  = \
main.c

libevas_la_LIBADD       = $LDFLAGS \
                          canvas/libevas_canvas.la \
			  data/libevas_data.la \
			  file/libevas_file.la \
			  imaging/libevas_imaging.la \
			  -lm \
			  engines/common/libevas_engine_common.la \
			  @freetype_libs@ \
			  @png_libs@ @jpeg_libs@ @eet_libs@ @edb_libs@ \
			  $(ENGINE_SOFTWARE_X11) \
			  $(ENGINE_DIRECTFB) \
			  $(ENGINE_FB) \
			  $(ENGINE_BUFFER) \
			  $(ENGINE_SOFTWARE_QTOPIA) \
			  $(ENGINE_GL_COMMON) \
			  $(ENGINE_GL_X11) \
			  $(ENGINE_CAIRO_COMMON) \
			  $(ENGINE_CAIRO_X11)
			  

libevas_la_DEPENDENCIES = \
                          $(top_builddir)/config.h \
                          canvas/libevas_canvas.la \
			  data/libevas_data.la \
			  file/libevas_file.la \
			  imaging/libevas_imaging.la \
			  engines/common/libevas_engine_common.la \
			  $(ENGINE_SOFTWARE_X11) \
			  $(ENGINE_DIRECTFB) \
			  $(ENGINE_FB) \
			  $(ENGINE_BUFFER) \
			  $(ENGINE_SOFTWARE_QTOPIA) \
			  $(ENGINE_GL_COMMON) \
			  $(ENGINE_GL_X11) \
			  $(ENGINE_CAIRO_COMMON) \
			  $(ENGINE_CAIRO_X11)

libevas_la_LDFLAGS      = -version-info 1:0:0
