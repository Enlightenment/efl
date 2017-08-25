#ifndef EVAS_GL_THREAD_H
#define EVAS_GL_THREAD_H


#include "evas_gl_common.h"

typedef struct
{
   int (*evas_gl_thread_enabled)(int thread_type);
   void (*evas_gl_thread_finish)(void);

   void *(*evas_gl_thread_gl_func_get)(void);
   void *(*evas_gl_thread_egl_func_get)(void);
   void *(*evas_gl_thread_glx_func_get)(void);
} Evas_GL_Thread_Func;

/* Custom thread call may be performed (without default parameters) */
/* Such as GL_TH_FN(func)(param). See glTexSubImage2DEVAS */
#define GL_TH_FN(api)                evas_##api##_thread_cmd

/* Internal names of thread infrastructure */
#define GL_TH_FNTYPE(api)            PFN_EVTH_##api
#define GL_TH_ST(api)                Evas_Thread_Command_##api
#define GL_TH_CB(api)                _gl_thread_##api

/* Default parameters. Are passed by using GL_TH, GL_TH_CALL, EVGL_TH, EVGL_TH_CALL */
#define GL_TH_DP                     int thread_type, void *orig_func

/* Thread call for Evas GL backend. original function = api name */
#define GL_TH(api, ...)              GL_TH_FN(api)(EVAS_GL_THREAD_TYPE_GL, api, ##__VA_ARGS__)
/* Thread call for Evas GL backend. original function may be different such as function pointer */
#define GL_TH_CALL(api, orig, ...)   GL_TH_FN(api)(EVAS_GL_THREAD_TYPE_GL, orig, ##__VA_ARGS__)

/* Thread call for EvasGL. original function = api name */
#define EVGL_TH(api, ...)            GL_TH_FN(api)(EVAS_GL_THREAD_TYPE_EVGL, api, ##__VA_ARGS__)
/* Thread call for EvasGL. original function may be different such as function pointer */
#define EVGL_TH_CALL(api, orig, ...) GL_TH_FN(api)(EVAS_GL_THREAD_TYPE_EVGL, orig, ##__VA_ARGS__)


#ifdef EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC

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


EAPI const Evas_GL_Thread_Func *evas_gl_thread_func_get(void);

extern int evas_gl_thread_enabled(int thread_type);
extern int evas_gl_thread_force_finish(void);
extern void evas_gl_thread_finish(void);

extern void evas_gl_thread_init(void);
extern void evas_gl_thread_terminate(void);


#else /* ! EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */


extern const Evas_GL_Thread_Func *(*evas_gl_thread_func_get)(void);

extern int (*evas_gl_thread_enabled)(int thread_type);
extern void (*evas_gl_thread_finish)(void);

extern void evas_gl_thread_link_init(void);


#endif /* EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */


#include "evas_gl_thread_gl.h"
#ifdef GL_GLES
#include "evas_gl_thread_egl.h"
#else
#include "evas_gl_thread_glx.h"
#endif


#endif /* EVAS_GL_THREAD_H */
