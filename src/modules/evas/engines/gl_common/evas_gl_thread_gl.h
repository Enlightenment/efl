#ifndef EVAS_GL_THREAD_GL_H
#define EVAS_GL_THREAD_GL_H

extern void *evas_gl_thread_gl_func_get(void);

/* Declare Evas GL Types */
typedef struct _Evas_GL               Evas_GL;
typedef struct _Evas_GL_Surface       Evas_GL_Surface;
typedef struct _Evas_GL_Context       Evas_GL_Context;
typedef struct _Evas_GL_Config        Evas_GL_Config;
typedef struct _Evas_GL_API           Evas_GL_API;
typedef void                         *Evas_GL_Func;
typedef void                         *EvasGLImage;

typedef void                          GLvoid;
typedef char                          GLchar;
typedef unsigned int                  GLenum;
typedef unsigned char                 GLboolean;
typedef unsigned int                  GLbitfield;
typedef signed char                   GLbyte;       // Changed khronos_int8_t
typedef short                         GLshort;
typedef int                           GLint;
typedef int                           GLsizei;
typedef unsigned char                 GLubyte;      // Changed khronos_uint8_t
typedef unsigned short                GLushort;
typedef unsigned int                  GLuint;
typedef float                         GLfloat;      // Changed khronos_float_t
typedef float                         GLclampf;     // Changed khronos_float_t
typedef signed int                    GLfixed;      // Changed khronos_int32_t
typedef ptrdiff_t                     GLintptr;     // Changed khronos_intptr_t
typedef ptrdiff_t                     GLsizeiptr;   // Changed khronos_ssize_t
typedef signed int                    GLclampx;     // Changed khronos_int32_t
typedef struct __GLsync*              GLsync;
typedef void *                        EvasGLSync;
typedef unsigned long long            EvasGLTime;

#ifndef TIZEN
/* We should clear platform dependencies for Evas_GL.h */
typedef int64_t                       EvasGLint64;
typedef uint64_t                      EvasGLuint64;
#endif

#ifdef EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC

# include "evas_gl_thread_gl_generated.h"

extern void GL_TH_FN(glTexSubImage2DEVAS)(int thread_push, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);

#else /* ! EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */
/* compiled for GL backend */


# include "evas_gl_thread_gl_link_generated.h"

extern void (*GL_TH_FN(glTexSubImage2DEVAS))(int thread_push, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);

extern void _gl_thread_link_init(void *func_ptr);

#endif /* EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */


#endif /* EVAS_GL_THREAD_GL_H */
