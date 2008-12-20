
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@evas_engine_direct3d_cflags@

if BUILD_ENGINE_DIRECT3D

pkgdir = $(libdir)/evas/modules/engines/direct3d/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la
module_la_SOURCES = \
evas_engine.c \
evas_direct3d_context.cpp \
evas_direct3d_device.cpp \
evas_direct3d_image_cache.cpp \
evas_direct3d_main.cpp \
evas_direct3d_object.cpp \
evas_direct3d_object_font.cpp \
evas_direct3d_object_image.cpp \
evas_direct3d_object_line.cpp \
evas_direct3d_object_rect.cpp \
evas_direct3d_scene.cpp \
evas_direct3d_shader_pack.cpp \
evas_direct3d_vertex_buffer_cache.cpp

module_la_CXXFLAGS = -fno-exceptions
module_la_LIBADD = $(top_builddir)/src/lib/libevas.la @EINA_LIBS@ @evas_engine_direct3d_libs@
module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_Direct3D.h

endif

EXTRA_DIST = \
array.h \
evas_direct3d_context.h \
evas_direct3d_device.h \
evas_direct3d_image_cache.h \
evas_direct3d_object.h \
evas_direct3d_object_font.h \
evas_direct3d_object_image.h \
evas_direct3d_object_line.h \
evas_direct3d_object_rect.h \
evas_direct3d_scene.h \
evas_direct3d_shader_pack.h \
evas_direct3d_vertex_buffer_cache.h \
evas_engine.h \
ref.h
