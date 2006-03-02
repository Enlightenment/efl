MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = \
software_generic \
buffer \
cairo_common \
cairo_x11 \
directfb \
fb \
gl_common \
gl_x11 \
software_qtopia \
software_win32_gdi \
software_x11 \
software_xcb \
xrender_x11

# FIXME: these should go into engine subdirs so engines are much more fully
# self-contained.

EXTRA_DIST = \
Evas_Engine_DirectFB.h \
Evas_Engine_GL_X11.h \
Evas_Engine_Cairo_X11.h

if BUILD_ENGINE_DIRECTFB
ENGINE_DIRECTFB_INC = Evas_Engine_DirectFB.h
else
ENGINE_DIRECTFB_INC = 
endif

if BUILD_ENGINE_GL_X11
ENGINE_GL_X11_INC = Evas_Engine_GL_X11.h
else
ENGINE_GL_X11_INC =
endif

if BUILD_ENGINE_CAIRO_X11
ENGINE_CAIRO_X11_INC = Evas_Engine_Cairo_X11.h
else
ENGINE_CAIRO_X11_INC =
endif

include_HEADERS      = \
$(ENGINE_DIRECTFB_INC) \
$(ENGINE_GL_X11_INC) \
$(ENGINE_CAIRO_X11_INC)
