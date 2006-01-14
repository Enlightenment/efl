#ifndef _EVAS_GL_PRIVATE_H
#define _EVAS_GL_PRIVATE_H

#include "evas_gl_common.h"

#ifndef GL_TEXTURE_RECTANGLE_NV
#define GL_TEXTURE_RECTANGLE_NV 0x84f5
#endif

#if 0
#ifndef GL_WRITE_PIXEL_DATA_RANGE_NV
/* nvidia extensions */
extern void *glXAllocateMemoryNV(GLsizei size, GLfloat readfreq, GLfloat writefreq, GLfloat priority);
extern void glPixelDataRangeNV(GLenum target, GLsizei length, void *pointer);
extern void glFlushPixelDataRangeNV(GLenum target);
# define GL_WRITE_PIXEL_DATA_RANGE_NV 0x8878
#define GL_READ_PIXEL_DATA_RANGE_NV 0x8879
#endif

/* arb extensions */
void glBindBufferARB(GLenum target, uint buffer);
void glDeleteBuffersARB(GLsizei n, const uint *buffers);
void glGenBuffersARB(GLsizei n, GLuint *buffers);
GLboolean glIsBufferARB(GLuint buffer);

void glBufferDataARB(GLenum target, GLsizei size, const void *data,
		                        GLenum usage);
void glBufferSubDataARB(GLenum target, int offset, GLsizei size,
			                      const void *data);
void glGetBufferSubDataARB(GLenum target, int offset,
			                            GLsizei size, void *data);

void *glMapBufferARB(GLenum target, GLenum access);
GLboolean glUnmapBufferARB(GLenum target);

void glGetBufferParameterivARB(GLenum target, GLenum pname, int *params);
void glGetBufferPointervARB(GLenum target, GLenum pname, void **params);

#define GL_ARRAY_BUFFER_ARB                             0x8892
#define GL_ELEMENT_ARRAY_BUFFER_ARB                     0x8893
#define GL_ARRAY_BUFFER_BINDING_ARB                     0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB             0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING_ARB              0x8896
#define GL_BNORMAL_ARRAY_BUFFER_BINDING_ARB             0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING_ARB               0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING_ARB               0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB       0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB           0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB     0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB      0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB              0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB       0x889F
#define GL_STREAM_DRAW_ARB                              0x88E0
#define GL_STREAM_READ_ARB                              0x88E1
#define GL_STREAM_COPY_ARB                              0x88E2
#define GL_STATIC_DRAW_ARB                              0x88E4
#define GL_STATIC_READ_ARB                              0x88E5
#define GL_STATIC_COPY_ARB                              0x88E6
#define GL_DYNAMIC_DRAW_ARB                             0x88E8
#define GL_DYNAMIC_READ_ARB                             0x88E9
#define GL_DYNAMIC_COPY_ARB                             0x88EA
#define GL_READ_ONLY_ARB                                0x88B8
#define GL_WRITE_ONLY_ARB                               0x88B9
#define GL_READ_WRITE_ARB                               0x88BA
#define GL_BUFFER_SIZE_ARB                              0x8764
#define GL_BUFFER_USAGE_ARB                             0x8765
#define GL_BUFFER_ACCESS_ARB                            0x88BB
#define GL_BUFFER_MAPPED_ARB                            0x88BC
#define GL_BUFFER_MAP_POINTER_ARB                       0x88BD
#endif






/* evas ARGB pixel config */
#define NATIVE_PIX_FORMAT GL_BGRA

/* Big endian systems require the texture know the byte order is reversed */
#ifdef WORDS_BIGENDIAN
#define NATIVE_PIX_UNIT   GL_UNSIGNED_INT_8_8_8_8_REV
#else
/* fast on vidia */
/*#define NATIVE_PIX_UNIT   GL_UNSIGNED_INT_8_8_8_8_REV*/
/* fast on ati compared to GL_UNSIGNED_INT_8_8_8_8_REV */
#define NATIVE_PIX_UNIT   GL_UNSIGNED_BYTE
#endif

#endif
