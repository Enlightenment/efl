#include "evas_gl_thread.h"


#ifdef GL_GLES

#define EVAS_TH_EGL_FN(ret, name, ...) \
 ret (*GL_TH_FN(name))(GL_TH_DP, ##__VA_ARGS__);
#define EVAS_TH_EGL_FN_ASYNC(ret, name, ...) \
 void *(*GL_TH_FN(name##_begin))(GL_TH_DP, ##__VA_ARGS__); \
 ret (*GL_TH_FN(name##_end))(void *ref);

typedef struct
{
   EVAS_TH_EGL_FN_LIST
} Evas_GL_Thread_EGL_Func;

#undef EVAS_TH_EGL_FN_ASYNC
#undef EVAS_TH_EGL_FN

#define EVAS_TH_EGL_FN(ret, name, ...) \
 typedef ret (*GL_TH_FNTYPE(name))(__VA_ARGS__);
#define EVAS_TH_EGL_FN_ASYNC(ret, name, ...) \
 typedef void *(*GL_TH_FNTYPE(name##_begin))(__VA_ARGS__); \
 typedef ret (*GL_TH_FNTYPE(name##_end))(void *ref);

EVAS_TH_EGL_FN_LIST

#undef EVAS_TH_EGL_FN_ASYNC
#undef EVAS_TH_EGL_FN


#ifdef EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC


typedef struct
{
   GL_TH_FNTYPE(eglGetError) orig_func;
   EGLint return_value;
} GL_TH_ST(eglGetError);

static void
GL_TH_CB(eglGetError)(void *data)
{
   GL_TH_ST(eglGetError) *thread_data = (*(void **)data);

   thread_data->return_value =
      thread_data->orig_func();
}

EGLint
GL_TH_FN(eglGetError)(GL_TH_DP)
{
   GL_TH_ST(eglGetError) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     return ((GL_TH_FNTYPE(eglGetError))orig_func)();

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(eglGetError) *), &thcmd_ref);
   *thread_data_ptr = thread_data;
   *thread_data_ptr = thread_data;

   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(eglGetError),
                              EVAS_GL_THREAD_MODE_FINISH);

   return thread_data->return_value;
}



typedef struct
{
   GL_TH_FNTYPE(eglBindAPI) orig_func;
   EGLBoolean return_value;
   EGLenum api;
} GL_TH_ST(eglBindAPI);

static void
GL_TH_CB(eglBindAPI)(void *data)
{
   GL_TH_ST(eglBindAPI) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->api);
}

EGLBoolean
GL_TH_FN(eglBindAPI)(GL_TH_DP, EGLenum api)
{
   GL_TH_ST(eglBindAPI) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     return ((GL_TH_FNTYPE(eglBindAPI))orig_func)(api);

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(eglBindAPI) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->api = api;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(eglBindAPI),
                              EVAS_GL_THREAD_MODE_FINISH);

   return thread_data->return_value;
}



typedef struct
{
   GL_TH_FNTYPE(eglQueryAPI) orig_func;
   EGLenum return_value;
} GL_TH_ST(eglQueryAPI);

static void
GL_TH_CB(eglQueryAPI)(void *data)
{
   GL_TH_ST(eglQueryAPI) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func();
}

EGLenum
GL_TH_FN(eglQueryAPI)(GL_TH_DP)
{
   GL_TH_ST(eglQueryAPI) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     return ((GL_TH_FNTYPE(eglQueryAPI))orig_func)();

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(eglQueryAPI) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(eglQueryAPI),
                              EVAS_GL_THREAD_MODE_FINISH);

   return thread_data->return_value;
}



typedef struct
{
   GL_TH_FNTYPE(eglMakeCurrent) orig_func;
   EGLBoolean return_value;
   EGLDisplay dpy;
   EGLSurface draw;
   EGLSurface read;
   EGLContext ctx;
} GL_TH_ST(eglMakeCurrent);

EGLDisplay current_thread_dpy  = EGL_NO_DISPLAY;
EGLSurface current_thread_draw = EGL_NO_SURFACE;
EGLSurface current_thread_read = EGL_NO_SURFACE;
EGLContext current_thread_ctx  = EGL_NO_CONTEXT;

static void
GL_TH_CB(eglMakeCurrent)(void *data)
{
   GL_TH_ST(eglMakeCurrent) *thread_data = *(void **)data;
   DBG("THREAD >> OTHER THREAD MAKECURRENT : (%p, %p, %p, %p)\n", thread_data->dpy,
         thread_data->draw, thread_data->read, thread_data->ctx);

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->draw,
                             thread_data->read,
                             thread_data->ctx);

   if (thread_data->return_value == EGL_TRUE)
     {
        current_thread_dpy = thread_data->dpy;
        current_thread_draw = thread_data->draw;
        current_thread_read = thread_data->read;
        current_thread_ctx = thread_data->ctx;
     }
}

EGLBoolean
GL_TH_FN(eglMakeCurrent)(GL_TH_DP, EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
   GL_TH_ST(eglMakeCurrent) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     return ((GL_TH_FNTYPE(eglMakeCurrent))orig_func)(dpy, draw, read, ctx);

   /* Skip for noop make-current */
   if (current_thread_dpy == dpy &&
       current_thread_draw == draw &&
       current_thread_read == read &&
       current_thread_ctx == ctx)
      return EGL_TRUE;

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(eglMakeCurrent) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->draw = draw;
   thread_data->read = read;
   thread_data->ctx = ctx;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(eglMakeCurrent),
                              EVAS_GL_THREAD_MODE_FINISH);

   return thread_data->return_value;
}



EGLContext
GL_TH_FN(eglGetCurrentContext)(GL_TH_DP)
{
   if (!evas_gl_thread_enabled(thread_type))
     return ((GL_TH_FNTYPE(eglGetCurrentContext))orig_func)();

   /* Current context is completely tracked by this variable */
   return current_thread_ctx;
}


EGLSurface
GL_TH_FN(eglGetCurrentSurface)(GL_TH_DP, EGLint readdraw)
{
   if (!evas_gl_thread_enabled(thread_type))
     return ((GL_TH_FNTYPE(eglGetCurrentSurface))orig_func)(readdraw);

   /* Current surfaces are completely tracked by this variable */
   if (readdraw == EGL_READ)
      return current_thread_read;
   if (readdraw == EGL_DRAW)
      return current_thread_draw;
   return NULL;
}


EGLDisplay
GL_TH_FN(eglGetCurrentDisplay)(GL_TH_DP)
{
   if (!evas_gl_thread_enabled(thread_type))
     return ((GL_TH_FNTYPE(eglGetCurrentDisplay))orig_func)();

   /* Current display is completely tracked by this variable */
   return current_thread_dpy;
}

typedef struct
{
   GL_TH_FNTYPE(eglReleaseThread) orig_func;
   EGLBoolean return_value;
} GL_TH_ST(eglReleaseThread);

static void
GL_TH_CB(eglReleaseThread)(void *data)
{
   GL_TH_ST(eglReleaseThread) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func();
}

EGLBoolean
GL_TH_FN(eglReleaseThread)(GL_TH_DP)
{
   GL_TH_ST(eglReleaseThread) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     return ((GL_TH_FNTYPE(eglReleaseThread))orig_func)();

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(eglReleaseThread) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(eglReleaseThread),
                              EVAS_GL_THREAD_MODE_FINISH);

   return thread_data->return_value;
}



typedef struct
{
   GL_TH_FNTYPE(eglQuerySurface) orig_func;
   EGLBoolean return_value;
   EGLDisplay dpy;
   EGLSurface surface;
   EGLint attribute;
   EGLint *value;
} GL_TH_ST(eglQuerySurface);

static void
GL_TH_CB(eglQuerySurface)(void *data)
{
   GL_TH_ST(eglQuerySurface) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->surface,
                             thread_data->attribute,
                             thread_data->value);
}

EGLBoolean
GL_TH_FN(eglQuerySurface)(GL_TH_DP, EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value)
{
   GL_TH_ST(eglQuerySurface) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     return ((GL_TH_FNTYPE(eglQuerySurface))orig_func)(dpy, surface, attribute, value);

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(eglQuerySurface) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->surface = surface;
   thread_data->attribute = attribute;
   thread_data->value =value;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(eglQuerySurface),
                              EVAS_GL_THREAD_MODE_FINISH);

   return thread_data->return_value;
}



typedef struct
{
   GL_TH_FNTYPE(eglSwapInterval) orig_func;
   EGLBoolean return_value;
   EGLDisplay dpy;
   EGLint interval;
} GL_TH_ST(eglSwapInterval);

static void
GL_TH_CB(eglSwapInterval)(void *data)
{
   GL_TH_ST(eglSwapInterval) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->interval);
}

EGLBoolean
GL_TH_FN(eglSwapInterval)(GL_TH_DP, EGLDisplay dpy, EGLint interval)
{
   GL_TH_ST(eglSwapInterval) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     return ((GL_TH_FNTYPE(eglSwapInterval))orig_func)(dpy, interval);

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(eglSwapInterval) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->interval = interval;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(eglSwapInterval),
                              EVAS_GL_THREAD_MODE_FINISH);

   return thread_data->return_value;
}



typedef struct
{
   GL_TH_FNTYPE(eglWaitGL) orig_func;
   EGLBoolean return_value;
} GL_TH_ST(eglWaitGL);

static void
GL_TH_CB(eglWaitGL)(void *data)
{
   GL_TH_ST(eglWaitGL) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func();
}

EGLBoolean
GL_TH_FN(eglWaitGL)(GL_TH_DP)
{
   GL_TH_ST(eglWaitGL) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     return ((GL_TH_FNTYPE(eglWaitGL))orig_func)();

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(eglWaitGL) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(eglWaitGL),
                              EVAS_GL_THREAD_MODE_FINISH);

   return thread_data->return_value;
}



typedef struct
{
   GL_TH_FNTYPE(eglSwapBuffers) orig_func;
   EGLBoolean return_value;
   EGLDisplay dpy;
   EGLSurface surface;
} GL_TH_ST(eglSwapBuffers);

static void
GL_TH_CB(eglSwapBuffers)(void *data)
{
   GL_TH_ST(eglSwapBuffers) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->surface);
}

EGLBoolean
GL_TH_FN(eglSwapBuffers)(GL_TH_DP, EGLDisplay dpy, EGLSurface surface)
{
   GL_TH_ST(eglSwapBuffers) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     return ((GL_TH_FNTYPE(eglSwapBuffers))orig_func)(dpy, surface);

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(eglSwapBuffers) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->surface = surface;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(eglSwapBuffers),
                              EVAS_GL_THREAD_MODE_FINISH);

   return thread_data->return_value;
}



typedef struct
{
   GL_TH_FNTYPE(eglSwapBuffersWithDamage) orig_func;
   EGLBoolean return_value;
   EGLDisplay dpy;
   EGLSurface surface;
   EGLint *rects;
   EGLint n_rects;
} GL_TH_ST(eglSwapBuffersWithDamage);

static void
GL_TH_CB(eglSwapBuffersWithDamage)(void *data)
{
   GL_TH_ST(eglSwapBuffersWithDamage) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->surface,
                             thread_data->rects,
                             thread_data->n_rects);
}

EGLBoolean
GL_TH_FN(eglSwapBuffersWithDamage)(GL_TH_DP, EGLDisplay dpy, EGLSurface surface, EGLint *rects, EGLint n_rects)
{
   GL_TH_ST(eglSwapBuffersWithDamage) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     return ((GL_TH_FNTYPE(eglSwapBuffersWithDamage))orig_func)(dpy, surface, rects, n_rects);

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(eglSwapBuffersWithDamage) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->surface = surface;
   thread_data->rects = rects;
   thread_data->n_rects = n_rects;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(eglSwapBuffersWithDamage),
                              EVAS_GL_THREAD_MODE_FINISH);

   return thread_data->return_value;
}



typedef struct
{
   GL_TH_FNTYPE(eglSetDamageRegion) orig_func;
   EGLBoolean return_value;
   EGLDisplay dpy;
   EGLSurface surface;
   EGLint *rects;
   EGLint n_rects;
} GL_TH_ST(eglSetDamageRegion);

static void
GL_TH_CB(eglSetDamageRegion)(void *data)
{
   GL_TH_ST(eglSetDamageRegion) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->surface,
                             thread_data->rects,
                             thread_data->n_rects);
}

EGLBoolean
GL_TH_FN(eglSetDamageRegion)(GL_TH_DP, EGLDisplay dpy, EGLSurface surface, EGLint *rects, EGLint n_rects)
{
   GL_TH_ST(eglSetDamageRegion) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     return ((GL_TH_FNTYPE(eglSetDamageRegion))orig_func)(dpy, surface, rects, n_rects);

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(eglSetDamageRegion) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->surface = surface;
   thread_data->rects = rects;
   thread_data->n_rects = n_rects;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(eglSetDamageRegion),
                              EVAS_GL_THREAD_MODE_FINISH);

   return thread_data->return_value;
}



typedef struct
{
   GL_TH_FNTYPE(eglQueryWaylandBuffer) orig_func;
   EGLBoolean return_value;
   EGLDisplay dpy;
   void *buffer;
   EGLint attribute;
   EGLint *value;
} GL_TH_ST(eglQueryWaylandBuffer);

static void
GL_TH_CB(eglQueryWaylandBuffer)(void *data)
{
   GL_TH_ST(eglQueryWaylandBuffer) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->buffer,
                             thread_data->attribute,
                             thread_data->value);
}

EGLBoolean
GL_TH_FN(eglQueryWaylandBuffer)(GL_TH_DP, EGLDisplay dpy, void *buffer, EGLint attribute, EGLint *value)
{
   GL_TH_ST(eglQueryWaylandBuffer) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     return ((GL_TH_FNTYPE(eglQueryWaylandBuffer))orig_func)(dpy, buffer, attribute, value);

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(eglQueryWaylandBuffer) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->buffer = buffer;
   thread_data->attribute = attribute;
   thread_data->value =value;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(eglQueryWaylandBuffer),
                              EVAS_GL_THREAD_MODE_FINISH);

   return thread_data->return_value;
}


/*****************************************************************************/


static Evas_GL_Thread_EGL_Func th_egl_func;
Eina_Bool th_egl_func_initialized = EINA_FALSE;

void *
evas_gl_thread_egl_func_get()
{
   if (!th_egl_func_initialized)
     {
#define THREAD_FUNCTION_ASSIGN(func) th_egl_func.func = func;

#define EVAS_TH_EGL_FN(ret, name, ...) \
 THREAD_FUNCTION_ASSIGN(GL_TH_FN(name));
#define EVAS_TH_EGL_FN_ASYNC(ret, name, ...) \
 THREAD_FUNCTION_ASSIGN(GL_TH_FN(name##_begin)); \
 THREAD_FUNCTION_ASSIGN(GL_TH_FN(name##_end));

   EVAS_TH_EGL_FN_LIST

#undef EVAS_TH_EGL_FN_ASYNC
#undef EVAS_TH_EGL_FN

#undef THREAD_FUNCTION_ASSIGN

        th_egl_func_initialized = EINA_TRUE;
     }

   return &th_egl_func;
}


#else /* ! EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */
      /* compiled for GL backend */


#include <dlfcn.h>

#define EVAS_TH_EGL_FN(ret, name, ...) \
 ret (*GL_TH_FN(name))(GL_TH_DP, ##__VA_ARGS__);
#define EVAS_TH_EGL_FN_ASYNC(ret, name, ...) \
 void *(*GL_TH_FN(name##_begin))(GL_TH_DP, ##__VA_ARGS__); \
 ret (*GL_TH_FN(name##_end))(GL_TH_DP, void *ref);

   EVAS_TH_EGL_FN_LIST

#undef EVAS_TH_EGL_FN_ASYNC
#undef EVAS_TH_EGL_FN

void
_egl_thread_link_init(void *func_ptr)
{
   const Evas_GL_Thread_EGL_Func *th_egl_func = func_ptr;

   if (!th_egl_func)
     {
        ERR("Thread functions (EGL BASE) are not exist");
        return;
     }

#define THREAD_FUNCTION_ASSIGN(func) func = th_egl_func->func;

#define EVAS_TH_EGL_FN(ret, name, ...) \
 THREAD_FUNCTION_ASSIGN(GL_TH_FN(name));
#define EVAS_TH_EGL_FN_ASYNC(ret, name, ...) \
 THREAD_FUNCTION_ASSIGN(GL_TH_FN(name##_begin)); \
 THREAD_FUNCTION_ASSIGN(GL_TH_FN(name##_end));

   EVAS_TH_EGL_FN_LIST

#undef EVAS_TH_EGL_FN_ASYNC
#undef EVAS_TH_EGL_FN

#undef THREAD_FUNCTION_ASSIGN
}

#endif /* EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */

#endif /* ! GL_GLES */
