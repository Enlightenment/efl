
#include "evas_gl_api_ext.h"

#include <dlfcn.h>


#define MAX_EXTENSION_STRING_BUFFER 10240

char _gl_ext_string[10240] = { 0 };

#ifndef EGL_NATIVE_PIXMAP_KHR
# define EGL_NATIVE_PIXMAP_KHR 0x30b0
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Extension HEADER
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()
#define _EVASGL_EXT_BEGIN(name)
#define _EVASGL_EXT_END()
#define _EVASGL_EXT_DRVNAME(name)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param) ret (*glextsym_##name) param = NULL;
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
/////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Extension HEADER
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()
#define _EVASGL_EXT_BEGIN(name) int _gl_ext_support_##name = 0;
#define _EVASGL_EXT_END()
#define _EVASGL_EXT_DRVNAME(name)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param)
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
/////////////////////////////////////////////////////////////////////////////////////////////////////


// Evas extensions from EGL extensions
#ifdef GL_GLES
#define EGLDISPLAY_GET() _evgl_egl_display_get(__FUNCTION__)
static EGLDisplay
_evgl_egl_display_get(const char *function)
{
   EGLDisplay dpy = EGL_NO_DISPLAY;
   EVGL_Resource *rsc;

   if (!(rsc=_evgl_tls_resource_get()))
     {
        ERR("%s: Unable to execute GL command. Error retrieving tls", function);
        _evgl_error_set(EVAS_GL_NOT_INITIALIZED);
        return EGL_NO_DISPLAY;
     }

   if (!rsc->current_eng)
     {
        ERR("%s: Unable to retrive Current Engine", function);
        _evgl_error_set(EVAS_GL_NOT_INITIALIZED);
        return EGL_NO_DISPLAY;
     }

   if ((evgl_engine) && (evgl_engine->funcs->display_get))
     {
        dpy = (EGLDisplay)evgl_engine->funcs->display_get(rsc->current_eng);
        return dpy;
     }
   else
     {
        ERR("%s: Invalid Engine... (Can't acccess EGL Display)\n", function);
        _evgl_error_set(EVAS_GL_BAD_DISPLAY);
        return EGL_NO_DISPLAY;
     }
}

static void *
_evgl_eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx,
                        int target, void* buffer, const int *attrib_list)
{
   int *attribs = NULL;

   /* Convert 0 terminator into a EGL_NONE terminator */
   if (attrib_list)
     {
        int cnt = 0;
        int *a;

        for (a = (int *) attrib_list; (*a) && (*a != EGL_NONE); a += 2)
          {
             /* TODO: Verify supported attributes */
             cnt += 2;
          }

        attribs = alloca(sizeof(int) * (cnt + 1));
        for (a = attribs; (*attrib_list) && (*attrib_list != EGL_NONE);
             a += 2, attrib_list += 2)
          {
             a[0] = attrib_list[0];
             a[1] = attrib_list[1];
          }
        *a = EGL_NONE;
     }

   return EXT_FUNC(eglCreateImage)(dpy, ctx, target, buffer, attribs);
}

static void *
evgl_evasglCreateImage(int target, void* buffer, const int *attrib_list)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   EGLContext ctx = EGL_NO_CONTEXT;

   if (!dpy) return NULL;

   /* EGL_NO_CONTEXT will always fail for TEXTURE_2D */
   if (target == EVAS_GL_TEXTURE_2D)
     {
        ctx = eglGetCurrentContext();
        INF("Creating EGL image based on the current context: %p", ctx);
     }

   return _evgl_eglCreateImageKHR(dpy, ctx, target, buffer, attrib_list);
}

static void *
evgl_evasglCreateImageForContext(Evas_GL *evasgl EINA_UNUSED, Evas_GL_Context *evasctx,
                                 int target, void* buffer, const int *attrib_list)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   EGLContext ctx = EGL_NO_CONTEXT;

   if (!evasgl || !dpy) return NULL;

   ctx = evgl_context_native_get(evasctx);
   return _evgl_eglCreateImageKHR(dpy, ctx, target, buffer, attrib_list);
}

static void
evgl_evasglDestroyImage(EvasGLImage image)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return;
   EXT_FUNC(eglDestroyImage)(dpy, image);
}

static void
evgl_glEvasGLImageTargetTexture2D(GLenum target, EvasGLImage image)
{
   EXT_FUNC(glEGLImageTargetTexture2DOES)(target, image);
}

static void
evgl_glEvasGLImageTargetRenderbufferStorage(GLenum target, EvasGLImage image)
{
   EXT_FUNC(glEGLImageTargetRenderbufferStorageOES)(target, image);
}

static EvasGLSync
evgl_evasglCreateSync(Evas_GL *evas_gl EINA_UNUSED,
                      unsigned int type, const int *attrib_list)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return NULL;
   return EXT_FUNC(eglCreateSyncKHR)(dpy, type, attrib_list);
}

static Eina_Bool
evgl_evasglDestroySync(Evas_GL *evas_gl EINA_UNUSED, EvasGLSync sync)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return EINA_FALSE;
   return EXT_FUNC(eglDestroySyncKHR)(dpy, sync);
}

static int
evgl_evasglClientWaitSync(Evas_GL *evas_gl EINA_UNUSED,
                          EvasGLSync sync, int flags, EvasGLTime timeout)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return EINA_FALSE;
   return EXT_FUNC(eglClientWaitSyncKHR)(dpy, sync, flags, timeout);
}

static Eina_Bool
evgl_evasglSignalSync(Evas_GL *evas_gl EINA_UNUSED,
                      EvasGLSync sync, unsigned mode)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return EINA_FALSE;
   return EXT_FUNC(eglSignalSyncKHR)(dpy, sync, mode);
}

static Eina_Bool
evgl_evasglGetSyncAttrib(Evas_GL *evas_gl EINA_UNUSED,
                         EvasGLSync sync, int attribute, int *value)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return EINA_FALSE;
   return EXT_FUNC(eglGetSyncAttribKHR)(dpy, sync, attribute, value);
}

static int
evgl_evasglWaitSync(Evas_GL *evas_gl EINA_UNUSED,
                    EvasGLSync sync, int flags)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return EINA_FALSE;
   return EXT_FUNC(eglWaitSyncKHR)(dpy, sync, flags);
}


#else
#endif

static int _evgl_api_ext_status = 0;

void
evgl_api_ext_init(void *getproc, const char *glueexts)
{
   const char *glexts;
   typedef void (*_getproc_fn) (void);
   typedef _getproc_fn (*fp_getproc)(const char *);

   fp_getproc gp = (fp_getproc)getproc;

   memset(_gl_ext_string, 0x00, MAX_EXTENSION_STRING_BUFFER);

#define FINDSYM(getproc, dst, sym) \
   if (getproc) { \
      if (!dst) dst = (__typeof__(dst))getproc(sym); \
   } else { \
      if (!dst) dst = (__typeof__(dst))dlsym(RTLD_DEFAULT, sym); \
   }

   // GLES Extensions
   glexts = (const char*)glGetString(GL_EXTENSIONS);
   /*
   // GLUE Extensions
#ifdef GL_GLES
getproc = &eglGetProcAddress;
glueexts = eglQueryString(re->win->egl_disp, EGL_EXTENSIONS);
#else
getproc = &glXGetProcAddress;
glueexts = glXQueryExtensionsString(re->info->info.display,
re->info->info.screen);
#endif
    */

   /////////////////////////////////////////////////////////////////////////////////////////////////////
   // Extension HEADER
   /////////////////////////////////////////////////////////////////////////////////////////////////////
#define GETPROCADDR(sym) \
   (((!(*drvfunc)) && (gp)) ? (__typeof__((*drvfunc)))gp(sym) : (__typeof__((*drvfunc)))dlsym(RTLD_DEFAULT, sym))

#define _EVASGL_EXT_BEGIN(name) \
     { \
        int *ext_support = &_gl_ext_support_##name; \
        *ext_support = 0;

#define _EVASGL_EXT_END() \
     }

#define _EVASGL_EXT_CHECK_SUPPORT(name) \
   (strstr(glexts, name) != NULL || strstr(glueexts, name) != NULL)

#define _EVASGL_EXT_DISCARD_SUPPORT() \
   *ext_support = 0;

#define _EVASGL_EXT_DRVNAME(name) \
   if (_EVASGL_EXT_CHECK_SUPPORT(#name)) *ext_support = 1;

#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param) \
     { \
        ret (**drvfunc)param = &glextsym_##name;

#define _EVASGL_EXT_FUNCTION_END() \
        if ((*drvfunc) == NULL) _EVASGL_EXT_DISCARD_SUPPORT(); \
     }

#define _EVASGL_EXT_FUNCTION_DRVFUNC(name) \
   if ((*drvfunc) == NULL) *drvfunc = name;

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC

#undef GETPROCADDR
   /////////////////////////////////////////////////////////////////////////////////////////////////////



	_gl_ext_string[0] = 0x00; //NULL;

   /////////////////////////////////////////////////////////////////////////////////////////////////////
   // Extension HEADER
   /////////////////////////////////////////////////////////////////////////////////////////////////////
#define _EVASGL_EXT_BEGIN(name) \
     if (_gl_ext_support_##name != 0) strcat(_gl_ext_string, #name" ");

#define _EVASGL_EXT_END()
#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()
#define _EVASGL_EXT_DRVNAME(name)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param)
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
   /////////////////////////////////////////////////////////////////////////////////////////////////////




   _evgl_api_ext_status = 1;
}

void
evgl_api_ext_get(Evas_GL_API *gl_funcs)
{
   if (_evgl_api_ext_status != 1)
     {
        ERR("EVGL extension is not yet initialized.");
        return;
     }

#define ORD(f) EVAS_API_OVERRIDE(f, gl_funcs, glextsym_)

   /////////////////////////////////////////////////////////////////////////////////////////////////////
   // Extension HEADER
   /////////////////////////////////////////////////////////////////////////////////////////////////////
#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()
#define _EVASGL_EXT_BEGIN(name) \
   if (_gl_ext_support_##name != 0) \
     {
#define _EVASGL_EXT_END() \
     }
#define _EVASGL_EXT_DRVNAME(name)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param) \
   ORD(name);
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(ret, name, param)
#define _EVASGL_EXT_FUNCTION_PRIVATE_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN
#undef _EVASGL_EXT_FUNCTION_PRIVATE_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
   /////////////////////////////////////////////////////////////////////////////////////////////////////
#undef ORD

}

const char *
evgl_api_ext_string_get()
{
   if (_evgl_api_ext_status != 1)
     {
        ERR("EVGL extension is not yet initialized.");
        return NULL;
     }

   return _gl_ext_string;
}

