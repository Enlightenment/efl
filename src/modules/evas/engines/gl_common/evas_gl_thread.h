#ifndef EVAS_GL_THREAD_H
#define EVAS_GL_THREAD_H


#include "evas_gl_common.h"

/* Memory Pool */
extern Eina_Mempool *_mp_default;
extern Eina_Mempool *_mp_command;
extern Eina_Mempool *_mp_uniform;
extern Eina_Mempool *_mp_delete_object;
extern Eina_Mempool *_mp_texture;

extern unsigned int _mp_default_memory_size;
extern unsigned int _mp_command_memory_size;
extern unsigned int _mp_uniform_memory_size;
extern unsigned int _mp_delete_object_memory_size;
extern unsigned int _mp_texture_memory_size;

extern unsigned int _mp_default_pool_size;
extern unsigned int _mp_command_pool_size;
extern unsigned int _mp_uniform_pool_size;
extern unsigned int _mp_delete_object_pool_size;
extern unsigned int _mp_texture_pool_size;


#ifdef EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC

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


EAPI int evas_gl_thread_enabled();
EAPI int evas_evgl_thread_enabled();
EAPI int evas_gl_thread_force_finish();

EAPI void evas_gl_thread_begin();
EAPI void evas_gl_thread_end();
EAPI void evas_gl_thread_finish();
EAPI void evas_evgl_thread_begin();
EAPI void evas_evgl_thread_end();
EAPI void evas_evgl_thread_finish();

extern void evas_gl_thread_init();
extern void evas_gl_thread_terminate();


#else /* ! EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */


extern int (*evas_gl_thread_enabled)();
extern int (*evas_evgl_thread_enabled)();

extern void (*evas_gl_thread_begin)();
extern void (*evas_gl_thread_end)();
extern void (*evas_gl_thread_finish)();
extern void (*evas_evgl_thread_begin)();
extern void (*evas_evgl_thread_end)();
extern void (*evas_evgl_thread_finish)();

extern void evas_gl_thread_link_init();


#endif /* EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */


#include "evas_gl_thread_gl.h"
#ifdef GL_GLES
#include "evas_gl_thread_egl.h"
#else
#include "evas_gl_thread_glx.h"
#endif


#endif /* EVAS_GL_THREAD_H */
