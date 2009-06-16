MAINTAINERCLEANFILES = Makefile.in

SUBDIRS =

if !EVAS_STATIC_BUILD_SOFTWARE_GENERIC
SUBDIRS += software_generic
endif
if !EVAS_STATIC_BUILD_BUFFER
SUBDIRS += buffer
endif
if !EVAS_STATIC_BUILD_DIRECT3D
SUBDIRS += direct3d
endif
if !EVAS_STATIC_BUILD_DIRECTFB
SUBDIRS += directfb
endif
if !EVAS_STATIC_BUILD_FB
SUBDIRS += fb
endif
if !EVAS_STATIC_BUILD_GL_GLEW
SUBDIRS += gl_common gl_glew
endif
if !EVAS_STATIC_BUILD_GL_X11
SUBDIRS += gl_common gl_x11
endif
if !EVAS_STATIC_BUILD_QUARTZ
SUBDIRS += quartz
endif
if !EVAS_STATIC_BUILD_SOFTWARE_16
SUBDIRS += software_16
endif
if !EVAS_STATIC_BUILD_SOFTWARE_16_DDRAW
SUBDIRS += software_16_ddraw
endif
if !EVAS_STATIC_BUILD_SOFTWARE_16_WINCE
SUBDIRS += software_16_wince
endif
if !EVAS_STATIC_BUILD_SOFTWARE_16_X11
SUBDIRS += software_16_x11
endif
if !EVAS_STATIC_BUILD_SOFTWARE_SDL
SUBDIRS += software_16_sdl software_sdl
endif
if !EVAS_STATIC_BUILD_SOFTWARE_DDRAW
SUBDIRS += software_ddraw
endif
if !EVAS_STATIC_BUILD_SOFTWARE_GDI
SUBDIRS += software_gdi
endif
if !EVAS_STATIC_BUILD_SOFTWARE_X11
SUBDIRS += software_x11
endif
if !EVAS_STATIC_BUILD_XRENDER_X11
SUBDIRS += xrender_x11
endif


#if !EVAS_STATIC_BUILD_CAIRO_X11
#SUBDIRS +=
#cairo_common
#cairo_x11
#endif
#if !EVAS_STATIC_BUILD_SOFTWARE_QTOPIA
#SUBDIRS += software_qtopia
#endif

DIST_SUBDIRS = \
software_generic \
buffer \
direct3d \
directfb \
fb \
gl_common \
gl_glew \
gl_x11 \
quartz \
software_16 \
software_16_ddraw \
software_16_wince \
software_16_x11 \
software_16_sdl \
software_sdl \
software_ddraw \
software_gdi \
software_x11 \
xrender_x11
#cairo_common \
#cairo_x11 \
#software_qtopia
