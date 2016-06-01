EOLIAN_FLAGS = -I$(srcdir)

include ../../Makefile_Eolian_Subbuild_Helper.am

### Library
ector_eolian_files_generic = \
	ector_surface.eo \
	ector_buffer.eo \
	ector_renderer.eo \
	ector_renderer_shape.eo \
	ector_renderer_buffer.eo \
	ector_renderer_gradient.eo \
	ector_renderer_gradient_radial.eo \
	ector_renderer_gradient_linear.eo
ector_eolian_generic_h = $(ector_eolian_files_generic:%.eo=%.eo.h)

# Handle cairo backend
ector_eolian_files_cairo = \
	cairo/ector_cairo_surface.eo \
	cairo/ector_cairo_software_surface.eo \
	cairo/ector_renderer_cairo.eo \
	cairo/ector_renderer_cairo_shape.eo \
	cairo/ector_renderer_cairo_gradient_linear.eo \
	cairo/ector_renderer_cairo_gradient_radial.eo
ector_eolian_cairo_h = $(ector_eolian_files_cairo:%.eo=%.eo.h)

# Handle default software backend
ector_eolian_files_software = \
	software/ector_software_surface.eo \
	software/ector_software_buffer.eo \
	software/ector_software_buffer_base.eo \
	software/ector_renderer_software.eo \
	software/ector_renderer_software_shape.eo \
	software/ector_renderer_software_buffer.eo \
	software/ector_renderer_software_gradient_radial.eo \
	software/ector_renderer_software_gradient_linear.eo
ector_eolian_software_h = $(ector_eolian_files_software:%.eo=%.eo.h)

# Handle gl backend
ector_eolian_files_gl = \
	gl/ector_gl_surface.eo \
	gl/ector_gl_buffer.eo \
	gl/ector_gl_buffer_base.eo \
	gl/ector_renderer_gl.eo \
	gl/ector_renderer_gl_shape.eo \
	gl/ector_renderer_gl_gradient_radial.eo \
	gl/ector_renderer_gl_gradient_linear.eo
ector_eolian_gl_h = $(ector_eolian_files_gl:%.eo=%.eo.h)

ector_eolian_type_files = ector_types.eot

ector_eolian_files = $(ector_eolian_files_generic) \
                     $(ector_eolian_files_cairo) \
                     $(ector_eolian_files_software) \
                     $(ector_eolian_files_gl)

ector_eolian_c = $(ector_eolian_files:%.eo=%.eo.c)
ector_eolian_h = $(ector_eolian_files:%.eo=%.eo.h) \
                 $(ector_eolian_type_files:%.eot=%.eot.h)

BUILT_SOURCES = \
	$(ector_eolian_c) \
	$(ector_eolian_h)

ectoreolianfilesdir = $(datadir)/eolian/include/ector-@VMAJ@
ectoreolianfiles_DATA = $(ector_eolian_files) $(ector_eolian_type_files)

lib_LTLIBRARIES = libector.la

# And the generic implementation

libector_la_SOURCES = \
ector_main.c \
ector_gl_internal.h \
ector_buffer.c \
ector_renderer_shape.c \
ector_renderer.c \
ector_renderer_buffer.c \
ector_renderer_gradient.c \
ector_renderer_gradient_radial.c \
ector_renderer_gradient_linear.c

# And now the cairo backend
libector_la_SOURCES += \
cairo/ector_renderer_cairo_gradient_linear.c \
cairo/ector_renderer_cairo_gradient_radial.c \
cairo/ector_renderer_cairo_shape.c \
cairo/ector_renderer_cairo.c \
cairo/ector_cairo_software_surface.c \
cairo/ector_cairo_surface.c

# SW draw static lib
draw_sources = \
../../static_libs/draw/draw_main_sse2.c \
../../static_libs/draw/draw_main.c \
../../static_libs/draw/draw_main_neon.c \
static_libs/rg_etc/rg_etc1.c \
static_libs/rg_etc/rg_etc2.c \
static_libs/rg_etc/rg_etc1.h \
static_libs/rg_etc/etc2_encoder.c \
$(NULL)

# And the default software backend
libector_la_SOURCES += \
software/ector_renderer_software_gradient_linear.c \
software/ector_renderer_software_gradient_radial.c \
software/ector_renderer_software_shape.c \
software/ector_renderer_software_buffer.c \
software/ector_software_gradient.c \
software/ector_software_rasterizer.c \
software/ector_software_surface.c \
software/ector_software_buffer.c \
../../static_libs/freetype/sw_ft_math.c \
../../static_libs/freetype/sw_ft_raster.c \
../../static_libs/freetype/sw_ft_stroker.c \
$(draw_sources)

# And now the gl backend
libector_la_SOURCES += \
gl/ector_renderer_gl_gradient_linear.c \
gl/ector_renderer_gl_gradient_radial.c \
gl/ector_renderer_gl_shape.c \
gl/ector_renderer_gl.c \
gl/ector_gl_buffer.c \
gl/ector_gl_buffer_base.c \
gl/ector_gl_surface.c \
gl/ector_gl_private.h \
gl/shader/ector_gl_shaders.x \
gl/ector_gl_shader.c

ECTOR_GL_SHADERS_GEN = \
gl/shader/gen_shaders.sh \
gl/shader/fragment.glsl \
gl/shader/vertex.glsl \
gl/shader/include.glsl

gl/shader/ector_gl_shaders.x: $(ECTOR_GL_SHADERS_GEN)
	@echo "  SHADERS  $@"
	@bash $(srcdir)/gl/shader/gen_shaders.sh

libector_la_CPPFLAGS = -I$(top_builddir)/src/lib/efl \
-I$(top_builddir)/src/lib/ector \
-I$(top_builddir)/src/lib/ector/cairo \
-I$(top_builddir)/src/lib/ector/software \
-I$(top_builddir)/src/lib/ector/gl \
-I$(top_srcdir)/src/static_libs/freetype \
-I$(top_srcdir)/src/static_libs/draw \
@ECTOR_CFLAGS@ \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/ector\" \
@VALGRIND_CFLAGS@ \
@SSE3_CFLAGS@

libector_la_LIBADD = @ECTOR_SUBBUILD_LIBS@
libector_la_DEPENDENCIES = @ECTOR_SUBBUILD_INTERNAL_LIBS@
libector_la_LDFLAGS = @EFL_LTLIBRARY_FLAGS@
