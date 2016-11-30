#ifndef EVAS_GL_THREAD_GL_H
#define EVAS_GL_THREAD_GL_H

#ifdef EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC

#ifndef TIZEN
/* We should clear platform dependencies for Evas_GL.h */
typedef int64_t EvasGLint64;
typedef uint64_t EvasGLuint64;
#endif

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVAS_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

# include "evas_gl_thread_gl_generated.h"
# include "evas_gl_thread_evgl_generated.h"
# include "evas_gl_thread_evgl_api_generated.h"

EAPI void
evas_glTexSubImage2DEVAS_th(int thread_push, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);

#else /* ! EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */
/* compiled for GL backend */


# include "evas_gl_thread_gl_link_generated.h"
# include "evas_gl_thread_evgl_link_generated.h"

extern void (*evas_glTexSubImage2DEVAS_th)(int thread_push, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);

extern void _gl_thread_link_init();

#endif /* EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */


#endif /* EVAS_GL_THREAD_GL_H */
