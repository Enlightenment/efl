set(DESCRIPTION "Enlightened retained mode drawing library")

set(LIBRARIES
  eet
  efl
  emile
  m
  support-draw
  support-freetype
  support-rg_etc
  support-triangulator
  support-scenegraph
)

set(PUBLIC_LIBRARIES
  eina
  eo
)

set(PUBLIC_EO_FILES
  ector_buffer.eo
  ector_renderer.eo
  ector_renderer_buffer.eo
  ector_renderer_gradient.eo
  ector_renderer_gradient_linear.eo
  ector_renderer_gradient_radial.eo
  ector_renderer_shape.eo
  ector_surface.eo
  ector_types.eot

  cairo/ector_cairo_software_surface.eo
  cairo/ector_cairo_surface.eo
  cairo/ector_renderer_cairo.eo
  cairo/ector_renderer_cairo_gradient_linear.eo
  cairo/ector_renderer_cairo_gradient_radial.eo
  cairo/ector_renderer_cairo_shape.eo

  gl/ector_gl_buffer.eo
  gl/ector_gl_buffer_base.eo
  gl/ector_gl_surface.eo
  gl/ector_renderer_gl.eo
  gl/ector_renderer_gl_gradient_linear.eo
  gl/ector_renderer_gl_gradient_radial.eo
  gl/ector_renderer_gl_shape.eo

  software/ector_renderer_software.eo
  software/ector_renderer_software_buffer.eo
  software/ector_renderer_software_gradient_linear.eo
  software/ector_renderer_software_gradient_radial.eo
  software/ector_renderer_software_shape.eo
  software/ector_software_buffer.eo
  software/ector_software_buffer_base.eo
  software/ector_software_surface.eo
)

set(PUBLIC_HEADERS
  Ector.h
  ector_buffer.h
  ector_renderer.h
  ector_surface.h
  ector_util.h

  cairo/Ector_Cairo.h

  gl/Ector_GL.h

  software/Ector_Software.h
)

set(SOURCES
  ector_buffer.c
  ector_gl_internal.h
  ector_main.c
  ector_renderer.c
  ector_renderer_buffer.c
  ector_renderer_gradient.c
  ector_renderer_gradient_linear.c
  ector_renderer_gradient_radial.c
  ector_renderer_shape.c

  cairo/ector_cairo_software_surface.c
  cairo/ector_cairo_surface.c
  cairo/ector_renderer_cairo.c
  cairo/ector_renderer_cairo_gradient_linear.c
  cairo/ector_renderer_cairo_gradient_radial.c
  cairo/ector_renderer_cairo_shape.c

  gl/ector_gl_buffer.c
  gl/ector_gl_buffer_base.c
  gl/ector_gl_private.h
  gl/ector_gl_shader.c
  gl/ector_gl_surface.c
  gl/ector_renderer_gl.c
  gl/ector_renderer_gl_gradient_linear.c
  gl/ector_renderer_gl_gradient_radial.c
  gl/ector_renderer_gl_shape.c
  gl/shader/ector_gl_shaders.x

  software/ector_renderer_software_buffer.c
  software/ector_renderer_software_gradient_linear.c
  software/ector_renderer_software_gradient_radial.c
  software/ector_renderer_software_shape.c
  software/ector_software_buffer.c
  software/ector_software_gradient.c
  software/ector_software_rasterizer.c
  software/ector_software_surface.c
)

add_custom_target(ector-update-shaders
  COMMAND /bin/sh ${EFL_LIB_SOURCE_DIR}/gl/shader/gen_shaders.sh
  DEPENDS
  ${EFL_LIB_SOURCE_DIR}/gl/shader/gen_shaders.sh
  ${EFL_LIB_SOURCE_DIR}/gl/shader/fragment.glsl
  ${EFL_LIB_SOURCE_DIR}/gl/shader/vertex.glsl
  ${EFL_LIB_SOURCE_DIR}/gl/shader/include.glsl
  WORKING_DIRECTORY "${EFL_LIB_SOURCE_DIR}"
  COMMENT "SHADERS src/lib/ector/gl/shader/ector_gl_shaders.x (if needed)"
)

set(DEPENDENCIES ector-update-shaders)
