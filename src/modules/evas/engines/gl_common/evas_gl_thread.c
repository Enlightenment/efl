#include "evas_gl_common.h"

/* Main Thread id */
static Eina_Thread    _main_thread_id = 0;

#ifdef EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC

/* Memory Pool */
Eina_Mempool    *_mp_default       = NULL;
Eina_Mempool    *_mp_command       = NULL;
Eina_Mempool    *_mp_uniform       = NULL;
Eina_Mempool    *_mp_delete_object = NULL;
Eina_Mempool    *_mp_texture       = NULL;

unsigned int    _mp_default_memory_size       = 128;
unsigned int    _mp_command_memory_size       = 256;
unsigned int    _mp_uniform_memory_size       = 128;
unsigned int    _mp_delete_object_memory_size = 128;
unsigned int    _mp_texture_memory_size       = 128 * 1024;

unsigned int    _mp_default_pool_size       = 64;
unsigned int    _mp_command_pool_size       = 64;
unsigned int    _mp_uniform_pool_size       = 32;
unsigned int    _mp_delete_object_pool_size = 32;
unsigned int    _mp_texture_pool_size       =  8;


/* Thread Environment */
static int env_evas_gl_render_thread    = -1;
static int env_evas_gl_force_finish     = -1;

int
evas_gl_thread_enabled(int thread_type)
{
   Eina_Thread tid = eina_thread_self();
   if (UNLIKELY(env_evas_gl_render_thread != 0))
     {
        if (UNLIKELY(tid != _main_thread_id))
          {
             if (thread_type == EVAS_GL_THREAD_TYPE_EVGL) return 0;
          }
        if (UNLIKELY(tid == evas_gl_thread_get(EVAS_GL_THREAD_TYPE_GL))) return 0;
        return 1;
     }
   return 0;
}

int
evas_gl_thread_force_finish(void)
{
   if (env_evas_gl_force_finish == -1)
     {
        char *env_thread = getenv("EVAS_GL_RENDER_THREAD_FORCE_FINISH");
        int env_thread_value = 0;
        if (env_thread)
           env_thread_value = atoi(env_thread);

        if (env_thread_value)
           env_evas_gl_force_finish = 1;
        else
           env_evas_gl_force_finish = 0;
     }
   return env_evas_gl_force_finish;
}

static Evas_GL_Thread_Func th_func;
Eina_Bool th_func_initialized = EINA_FALSE;

EAPI const Evas_GL_Thread_Func *
evas_gl_thread_func_get(void)
{
   if (!th_func_initialized)
     {
#define THREAD_FUNCTION_ASSIGN(func) th_func.func = func;

        THREAD_FUNCTION_ASSIGN(evas_gl_thread_enabled);
        THREAD_FUNCTION_ASSIGN(evas_gl_thread_finish);

#ifdef GL_GLES
        THREAD_FUNCTION_ASSIGN(evas_gl_thread_egl_func_get);
#else
        THREAD_FUNCTION_ASSIGN(evas_gl_thread_glx_func_get);
#endif

        THREAD_FUNCTION_ASSIGN(evas_gl_thread_gl_func_get);

#undef THREAD_FUNCTION_ASSIGN

        th_func_initialized = EINA_TRUE;
     }

   return &th_func;
}

void
evas_gl_thread_init(void)
{
#define BUILD_MEMPOOL(name) \
   name = eina_mempool_add("chained_mempool", #name, NULL, \
                           name##_memory_size, name##_pool_size); \
   if (name == NULL) \
   EINA_LOG_ERR("eina_mempool_add() failed");

   char *env_thread = getenv("EVAS_GL_RENDER_THREAD");
   int env_thread_value = 1;
   if (env_thread)
      env_thread_value = atoi(env_thread);

   if (env_thread_value == 1)
      env_evas_gl_render_thread = 1;
   else
      env_evas_gl_render_thread = 0;

   BUILD_MEMPOOL(_mp_default);
   BUILD_MEMPOOL(_mp_command);
   BUILD_MEMPOOL(_mp_uniform);
   BUILD_MEMPOOL(_mp_delete_object);
   BUILD_MEMPOOL(_mp_texture);

   _main_thread_id = eina_thread_self();
}

void
evas_gl_thread_terminate(void)
{
   eina_mempool_del(_mp_default);
   eina_mempool_del(_mp_command);
   eina_mempool_del(_mp_uniform);
   eina_mempool_del(_mp_delete_object);
   eina_mempool_del(_mp_texture);
}

static void
GL_TH_CB(finish)(void *data)
{
   data = data;
}

void
evas_gl_thread_finish(void)
{
   if (evas_gl_thread_enabled(EVAS_GL_THREAD_TYPE_GL))
   {
      void *thcmd_ref;
      evas_gl_thread_cmd_create(EVAS_GL_THREAD_TYPE_GL, 0, &thcmd_ref);
      evas_gl_thread_cmd_enqueue(thcmd_ref,
                                 GL_TH_CB(finish),
                                 EVAS_GL_THREAD_MODE_FINISH);
   }
}


#else  /* ! EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */


#include <dlfcn.h>

const Evas_GL_Thread_Func *(*evas_gl_thread_func_get)(void);

int (*evas_gl_thread_enabled)(int thread_type);
void (*evas_gl_thread_finish)(void);

void
evas_gl_thread_link_init(void)
{
   const Evas_GL_Thread_Func *th_func = NULL;

#define LINK2GENERIC(sym) \
   sym = dlsym(RTLD_DEFAULT, #sym); \
   if (!sym) ERR("Could not find function '%s'", #sym);

   LINK2GENERIC(evas_gl_thread_func_get);

   th_func = evas_gl_thread_func_get();

   if (!th_func)
     {
        ERR("Thread functions (BASE) are not exist");
        return;
     }

#define THREAD_FUNCTION_ASSIGN(func) func = th_func->func;

   THREAD_FUNCTION_ASSIGN(evas_gl_thread_enabled);
   THREAD_FUNCTION_ASSIGN(evas_gl_thread_finish);

#undef THREAD_FUNCTION_ASSIGN

#ifdef GL_GLES
   if (th_func->evas_gl_thread_egl_func_get)
      _egl_thread_link_init(th_func->evas_gl_thread_egl_func_get());
   else
      ERR("Thread functions (EGL) are not exist");
#else
   if (th_func->evas_gl_thread_glx_func_get)
      _glx_thread_link_init(th_func->evas_gl_thread_glx_func_get());
   else
      ERR("Thread functions (GLX) are not exist");
#endif

   if (th_func->evas_gl_thread_gl_func_get)
      _gl_thread_link_init(th_func->evas_gl_thread_gl_func_get());
   else
      ERR("Thread functions (GL) are not exist");

   _main_thread_id = eina_thread_self();
}


#endif /* ! EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */
