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
static int env_evas_evgl_render_thread  = -1;
static int env_evas_gl_force_finish     = -1;
static int evas_gl_enable_thread        =  0;
static int evas_evgl_enable_thread      =  0;

int
evas_gl_thread_enabled()
{
   if (env_evas_gl_render_thread == -1)
     {
        char *env_thread = getenv("EVAS_GL_RENDER_THREAD");
        int env_thread_value = 0;
        if (env_thread)
           env_thread_value = atoi(env_thread);

        /* Thread rendering is forced off because direct rendering explicitly on */
        char *env_direct_override = getenv("EVAS_GL_DIRECT_OVERRIDE");
        int env_direct_override_value = 0;
        if (env_direct_override)
            env_direct_override_value = atoi(env_direct_override);

        if (env_thread_value == 1 && env_direct_override_value == 0)
           env_evas_gl_render_thread = 1;
        else
           env_evas_gl_render_thread = 0;
     }
   return env_evas_gl_render_thread && (evas_gl_enable_thread == 0);
}

int
evas_evgl_thread_enabled()
{
   if (env_evas_evgl_render_thread == -1)
     {
        char *env_thread = getenv("EVAS_EVGL_RENDER_THREAD");
        int env_thread_value = 0;
        if (env_thread)
           env_thread_value = atoi(env_thread);

        if (env_thread_value)
           env_evas_evgl_render_thread = 1;
        else
           env_evas_evgl_render_thread = 0;
     }
   return env_evas_gl_render_thread && env_evas_evgl_render_thread &&
      (evas_evgl_enable_thread == 0) && (_main_thread_id == eina_thread_self());
}

int
evas_gl_thread_force_finish()
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


void
evas_gl_thread_init()
{
#define BUILD_MEMPOOL(name) \
   name = eina_mempool_add("chained_mempool", #name, NULL, \
                           name##_memory_size, name##_pool_size); \
   if (name == NULL) \
   EINA_LOG_ERR("eina_mempool_add() failed");

   BUILD_MEMPOOL(_mp_default);
   BUILD_MEMPOOL(_mp_command);
   BUILD_MEMPOOL(_mp_uniform);
   BUILD_MEMPOOL(_mp_delete_object);
   BUILD_MEMPOOL(_mp_texture);

   _main_thread_id = eina_thread_self();
}

void
evas_gl_thread_terminate()
{
   eina_mempool_del(_mp_default);
   eina_mempool_del(_mp_command);
   eina_mempool_del(_mp_uniform);
   eina_mempool_del(_mp_delete_object);
   eina_mempool_del(_mp_texture);
}

void
evas_gl_thread_begin()
{
   evas_gl_enable_thread++;
}

void
evas_gl_thread_end()
{
   evas_gl_enable_thread--;
}

static void
_gl_thread_finish(void *data)
{
   data = data;
}

void
evas_gl_thread_finish()
{
   if (evas_gl_thread_enabled())
   {
      evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                                 _gl_thread_finish,
                                 NULL,
                                 EVAS_GL_THREAD_MODE_FINISH);
   }
}

void
evas_evgl_thread_begin()
{
   evas_evgl_enable_thread++;
}

void
evas_evgl_thread_end()
{
   evas_evgl_enable_thread--;
}

static void
_evgl_thread_finish(void *data)
{
    data = data;
}

void
evas_evgl_thread_finish()
{
   if (evas_gl_thread_enabled())
   {
      evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                                 _evgl_thread_finish,
                                 NULL,
                                 EVAS_GL_THREAD_MODE_FINISH);
   }
}


#else  /* ! EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */


#include <dlfcn.h>

int (*evas_gl_thread_enabled)();
int (*evas_evgl_thread_enabled)();

void (*evas_gl_thread_begin)();
void (*evas_gl_thread_end)();
void (*evas_gl_thread_finish)();
void (*evas_evgl_thread_begin)();
void (*evas_evgl_thread_end)();
void (*evas_evgl_thread_finish)();

void
evas_gl_thread_link_init()
{
#define LINK2GENERIC(sym) \
   sym = dlsym(RTLD_DEFAULT, #sym); \
   if (!sym) ERR("Could not find function '%s'", #sym);

   LINK2GENERIC(evas_gl_thread_enabled);
   LINK2GENERIC(evas_evgl_thread_enabled);

   LINK2GENERIC(evas_gl_thread_begin);
   LINK2GENERIC(evas_gl_thread_end);
   LINK2GENERIC(evas_gl_thread_finish);
   LINK2GENERIC(evas_evgl_thread_begin);
   LINK2GENERIC(evas_evgl_thread_end);
   LINK2GENERIC(evas_evgl_thread_finish);

#ifdef GL_GLES
   _egl_thread_link_init();
#else
   _glx_thread_link_init();
#endif
   _gl_thread_link_init();

   _main_thread_id = eina_thread_self();
}


#endif /* ! EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */
