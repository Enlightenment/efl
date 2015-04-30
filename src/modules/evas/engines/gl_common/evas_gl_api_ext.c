
#include "evas_gl_api_ext.h"

#include <dlfcn.h>

// list of egl extensions
#ifdef GL_GLES
static char *_egl_ext_string = NULL;
#endif
// list of gles 2.0 exts by official name
static char *_gl_ext_string = NULL;
// list of exts by official name only like "GL_EXT_discard_framebuffer GL_EXT_multi_draw_arrays"
static char *_gl_ext_string_official = NULL;
// list of gles 1.1 exts by official name
static char *_gles1_ext_string = NULL;
static char *_gles1_ext_string_official = NULL;
// list of gles 3.1 exts by official name
static char *_gles3_ext_string = NULL;
static char *_gles3_ext_string_official = NULL;

typedef void (*_getproc_fn) (void);
typedef _getproc_fn (*fp_getproc)(const char *);

#ifndef EGL_NATIVE_PIXMAP_KHR
# define EGL_NATIVE_PIXMAP_KHR 0x30b0
#endif

#ifndef EGL_WL_bind_wayland_display
struct wl_display;
struct wl_resource;
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Extension HEADER
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()
#define _EVASGL_EXT_BEGIN(name)
#define _EVASGL_EXT_END()
#define _EVASGL_EXT_DRVNAME(name)
#define _EVASGL_EXT_DRVNAME_PRIVATE(name)
#define _EVASGL_EXT_DRVNAME_DESKTOP(deskname)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param) \
   ret (*egl_ext_sym_##name) param = NULL; \
   ret (*gl_ext_sym_##name) param = NULL; \
   ret (*gles1_ext_sym_##name) param = NULL; \
   ret (*gles3_ext_sym_##name) param = NULL;
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)
#define _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR(name)

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_DRVNAME_PRIVATE
#undef _EVASGL_EXT_DRVNAME_DESKTOP
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#undef _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR
/////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Extension HEADER
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()
#define _EVASGL_EXT_BEGIN(name) \
   int _egl_ext_support_##name = 0; \
   int _gl_ext_support_##name = 0; \
   int _gles1_ext_support_##name = 0; \
   int _gles3_ext_support_##name = 0;
#define _EVASGL_EXT_END()
#define _EVASGL_EXT_DRVNAME(name)
#define _EVASGL_EXT_DRVNAME_PRIVATE(name) \
   int _egl_ext_support_func_##name = 0; \
   int _gl_ext_support_func_##name = 0; \
   int _gles1_ext_support_func_##name = 0; \
   int _gles3_ext_support_func_##name = 0;
#define _EVASGL_EXT_DRVNAME_DESKTOP(deskname)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param)
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)
#define _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR(name)

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_ENABLE_EGL
#undef _EVASGL_EXT_ENABLE_GL_GLES
#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_DRVNAME_PRIVATE
#undef _EVASGL_EXT_DRVNAME_DESKTOP
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#undef _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR
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
        evas_gl_common_error_set(NULL, EVAS_GL_NOT_INITIALIZED);
        return EGL_NO_DISPLAY;
     }

   if (!rsc->current_eng)
     {
        ERR("%s: no current engine set; ensure you've called evas_gl_make_current()", function);
        evas_gl_common_error_set(NULL, EVAS_GL_NOT_INITIALIZED);
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
        evas_gl_common_error_set(NULL, EVAS_GL_BAD_DISPLAY);
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

   return EXT_FUNC_EGL(eglCreateImage)(dpy, ctx, target, buffer, attribs);
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

   ctx = _evgl_native_context_get(evasctx);
   return _evgl_eglCreateImageKHR(dpy, ctx, target, buffer, attrib_list);
}

static void
evgl_evasglDestroyImage(EvasGLImage image)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return;
   EXT_FUNC_EGL(eglDestroyImage)(dpy, image);
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
   return EXT_FUNC_EGL(eglCreateSyncKHR)(dpy, type, attrib_list);
}

static Eina_Bool
evgl_evasglDestroySync(Evas_GL *evas_gl EINA_UNUSED, EvasGLSync sync)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return EINA_FALSE;
   return EXT_FUNC_EGL(eglDestroySyncKHR)(dpy, sync);
}

static int
evgl_evasglClientWaitSync(Evas_GL *evas_gl EINA_UNUSED,
                          EvasGLSync sync, int flags, EvasGLTime timeout)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return EINA_FALSE;
   return EXT_FUNC_EGL(eglClientWaitSyncKHR)(dpy, sync, flags, timeout);
}

static Eina_Bool
evgl_evasglSignalSync(Evas_GL *evas_gl EINA_UNUSED,
                      EvasGLSync sync, unsigned mode)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return EINA_FALSE;
   return EXT_FUNC_EGL(eglSignalSyncKHR)(dpy, sync, mode);
}

static Eina_Bool
evgl_evasglGetSyncAttrib(Evas_GL *evas_gl EINA_UNUSED,
                         EvasGLSync sync, int attribute, int *value)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return EINA_FALSE;
   return EXT_FUNC_EGL(eglGetSyncAttribKHR)(dpy, sync, attribute, value);
}

static int
evgl_evasglWaitSync(Evas_GL *evas_gl EINA_UNUSED,
                    EvasGLSync sync, int flags)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return EINA_FALSE;
   return EXT_FUNC_EGL(eglWaitSyncKHR)(dpy, sync, flags);
}

static Eina_Bool
evgl_evasglBindWaylandDisplay(Evas_GL *evas_gl EINA_UNUSED,
                              void *wl_display)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return EINA_FALSE;
   return EXT_FUNC_EGL(eglBindWaylandDisplayWL)(dpy, wl_display);
}

static Eina_Bool
evgl_evasglUnbindWaylandDisplay(Evas_GL *evas_gl EINA_UNUSED,
                                void *wl_display)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return EINA_FALSE;
   return EXT_FUNC_EGL(eglUnbindWaylandDisplayWL)(dpy, wl_display);
}

static Eina_Bool
evgl_evasglQueryWaylandBuffer(Evas_GL *evas_gl EINA_UNUSED,
                              void *buffer, int attribute, int *value)
{
   EGLDisplay dpy = EGLDISPLAY_GET();
   if (!dpy) return EINA_FALSE;
   return EXT_FUNC_EGL(eglQueryWaylandBufferWL)(dpy, buffer, attribute, value);
}


#else
#endif

//  0: not initialized,
//  1: GLESv2 initialized,
//  3: GLESv1 and GLESv2 initialized,
//  5: GLESv3 and GLESv2 initialized,
//  7: GLESv3 + GLESv2  + GLESv1 all initialized.
static int _evgl_api_ext_status = 0;
#define EVASGL_API_GLES2_EXT_INITIALIZED 0x1
#define EVASGL_API_GLES1_EXT_INITIALIZED 0x2
#define EVASGL_API_GLES3_EXT_INITIALIZED 0x4
#define EVASGL_API_EGL_EXT_INITIALIZED   0x8

#ifdef GL_GLES
Eina_Bool
evgl_api_egl_ext_init(void *getproc, const char *glueexts)
{
   fp_getproc gp = (fp_getproc)getproc;
   int _curext_supported = 0;
   Eina_Strbuf *sb = NULL;

   if (_evgl_api_ext_status & EVASGL_API_EGL_EXT_INITIALIZED)
      return EINA_TRUE;

   sb = eina_strbuf_new();

   /////////////////////////////////////////////////////////////////////////////////////////////////////
   // Extension HEADER
   /////////////////////////////////////////////////////////////////////////////////////////////////////
#define GETPROCADDR(sym) \
   (((!(*drvfunc)) && (gp)) ? (__typeof__((*drvfunc)))gp(sym) : (__typeof__((*drvfunc)))dlsym(RTLD_DEFAULT, sym))

#define _EVASGL_EXT_BEGIN(name) \
     { \
        int *ext_support = &_egl_ext_support_##name; \
        *ext_support = 0;

#define _EVASGL_EXT_END() \
     }

#define _EVASGL_EXT_CHECK_SUPPORT(name) \
   (strstr(glueexts, name) != NULL)

#define _EVASGL_EXT_DISCARD_SUPPORT() \
   *ext_support = 0;

#define _EVASGL_EXT_DRVNAME(name) \
   if (_EVASGL_EXT_CHECK_SUPPORT(#name)) *ext_support = 1;

#define _EVASGL_EXT_DRVNAME_PRIVATE(name) \
   if (_EVASGL_EXT_CHECK_SUPPORT(#name)) { *ext_support = 1; _egl_ext_support_func_##name = 1; }

#define _EVASGL_EXT_DRVNAME_DESKTOP(deskname) \
   if (_EVASGL_EXT_CHECK_SUPPORT(deskname)) *ext_support = 1;

#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param) \
     { \
        ret (**drvfunc)param = &egl_ext_sym_##name; \
        if (*ext_support == 1) \
          {

#define _EVASGL_EXT_FUNCTION_END() \
          } \
        if ((*drvfunc) == NULL) _EVASGL_EXT_DISCARD_SUPPORT(); \
     }
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name) \
   if ((*drvfunc) == NULL) *drvfunc = name;

// This adds all the function names to the "safe" list but only one pointer
// will be stored in the hash table.
#define _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR(name) \
   if ((*drvfunc) == NULL) \
     { \
        *drvfunc = GETPROCADDR(name); \
        evgl_safe_extension_add(name, (void *) (*drvfunc)); \
     } \
   else evgl_safe_extension_add(name, NULL);

#ifdef _EVASGL_EXT_FUNCTION_WHITELIST
# undef _EVASGL_EXT_FUNCTION_WHITELIST
#endif
#define _EVASGL_EXT_FUNCTION_WHITELIST(name) evgl_safe_extension_add(name, NULL);

#define _EVASGL_EXT_ENABLE_GL_GLES 0
#define _EVASGL_EXT_ENABLE_EGL 1

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_ENABLE_EGL
#undef _EVASGL_EXT_ENABLE_GL_GLES
#undef _EVASGL_EXT_FUNCTION_WHITELIST
#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_DRVNAME_PRIVATE
#undef _EVASGL_EXT_DRVNAME_DESKTOP
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#undef _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR

#undef GETPROCADDR
   /////////////////////////////////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////////////////////////////////
   // Extension HEADER
   /////////////////////////////////////////////////////////////////////////////////////////////////////
#define _EVASGL_EXT_BEGIN(name) \
     if (_egl_ext_support_##name != 0) \
       { \
          eina_strbuf_append(sb, #name" "); \
          _curext_supported = 1; \
       } \
     else _curext_supported = 0;

#define _EVASGL_EXT_END()
#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()
#define _EVASGL_EXT_DRVNAME_PRINT(name) \
       { \
          if ((strncmp(name, "EGL_", 4) == 0) && (strstr(eina_strbuf_string_get(sb), name) == NULL)) \
            eina_strbuf_append(sb, name" "); \
       }
#define _EVASGL_EXT_DRVNAME(name) \
     if (_curext_supported) \
       _EVASGL_EXT_DRVNAME_PRINT(#name)
#define _EVASGL_EXT_DRVNAME_PRIVATE(name) \
     if (_curext_supported && _egl_ext_support_func_##name) \
       _EVASGL_EXT_DRVNAME_PRINT(#name)
#define _EVASGL_EXT_DRVNAME_DESKTOP(deskname)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param)
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)
#define _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR(name)

#define _EVASGL_EXT_ENABLE_GL_GLES 0
#define _EVASGL_EXT_ENABLE_EGL 1

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_ENABLE_EGL
#undef _EVASGL_EXT_ENABLE_GL_GLES
#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME_PRINT
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_DRVNAME_DESKTOP
#undef _EVASGL_EXT_DRVNAME_PRIVATE
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#undef _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR
   /////////////////////////////////////////////////////////////////////////////////////////////////////

   if (_egl_ext_string) free(_egl_ext_string);
   _egl_ext_string = eina_strbuf_string_steal(sb);
   eina_strbuf_free(sb);

  ERR("EGL extension string: %s", _egl_ext_string);

  _evgl_api_ext_status |= EVASGL_API_EGL_EXT_INITIALIZED;
   return EINA_TRUE;
}
#endif

Eina_Bool
_evgl_api_gles2_ext_init(void *getproc, const char *glueexts)
{
   const char *glexts;
   fp_getproc gp = (fp_getproc)getproc;
   int _curext_supported = 0;
   Eina_Strbuf *sb = eina_strbuf_new();
   Eina_Strbuf *sboff = eina_strbuf_new();

#ifndef GL_GLES
   /* Add some extension strings that are always working on desktop GL */
   static const char *desktop_exts =
         "GL_EXT_read_format_bgra "
         "GL_EXT_texture_format_BGRA8888 "
         "GL_EXT_texture_type_2_10_10_10_REV ";
   eina_strbuf_append(sb, desktop_exts);
   eina_strbuf_append(sboff, desktop_exts);
#endif

   // GLES Extensions
   glexts = (const char*)glGetString(GL_EXTENSIONS);
   if (!glexts)
     {
        ERR("glGetString returned NULL! Something is very wrong...");
        return EINA_FALSE;
     }

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

#define _EVASGL_EXT_DRVNAME_PRIVATE(name) \
   if (_EVASGL_EXT_CHECK_SUPPORT(#name)) { *ext_support = 1; _gl_ext_support_func_##name = 1; }

#define _EVASGL_EXT_DRVNAME_DESKTOP(deskname) \
   if (_EVASGL_EXT_CHECK_SUPPORT(deskname)) *ext_support = 1;

#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param) \
     { \
        ret (**drvfunc)param = &gl_ext_sym_##name; \
        if (*ext_support == 1) \
          {

#define _EVASGL_EXT_FUNCTION_END() \
          } \
        if ((*drvfunc) == NULL) _EVASGL_EXT_DISCARD_SUPPORT(); \
     }
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name) \
   if ((*drvfunc) == NULL) *drvfunc = name;

// This adds all the function names to the "safe" list but only one pointer
// will be stored in the hash table.
#define _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR(name) \
   if ((*drvfunc) == NULL) \
     { \
        *drvfunc = GETPROCADDR(name); \
        evgl_safe_extension_add(name, (void *) (*drvfunc)); \
     } \
   else evgl_safe_extension_add(name, NULL);

#ifdef _EVASGL_EXT_FUNCTION_WHITELIST
# undef _EVASGL_EXT_FUNCTION_WHITELIST
#endif
#define _EVASGL_EXT_FUNCTION_WHITELIST(name) evgl_safe_extension_add(name, NULL);

#define _EVASGL_EXT_ENABLE_GL_GLES 1
#define _EVASGL_EXT_ENABLE_EGL 1

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_ENABLE_EGL
#undef _EVASGL_EXT_ENABLE_GL_GLES
#undef _EVASGL_EXT_FUNCTION_WHITELIST
#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_DRVNAME_PRIVATE
#undef _EVASGL_EXT_DRVNAME_DESKTOP
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#undef _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR

#undef GETPROCADDR
   /////////////////////////////////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////////////////////////////////
   // Extension HEADER
   /////////////////////////////////////////////////////////////////////////////////////////////////////
#define _EVASGL_EXT_BEGIN(name) \
     if (_gl_ext_support_##name != 0) \
       { \
          eina_strbuf_append(sb, #name" "); \
          _curext_supported = 1; \
       } \
     else _curext_supported = 0;

#define _EVASGL_EXT_END()
#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()
#define _EVASGL_EXT_DRVNAME_PRINT(name) \
       { \
          eina_strbuf_append(sb, name" "); \
          if ((strncmp(name, "GL_", 3) == 0) && (strstr(eina_strbuf_string_get(sboff), name) == NULL)) \
            eina_strbuf_append(sboff, name" "); \
       }
#define _EVASGL_EXT_DRVNAME(name) \
     if (_curext_supported) \
       _EVASGL_EXT_DRVNAME_PRINT(#name)
#define _EVASGL_EXT_DRVNAME_PRIVATE(name) \
     if (_curext_supported && _gl_ext_support_func_##name) \
       _EVASGL_EXT_DRVNAME_PRINT(#name)
#define _EVASGL_EXT_DRVNAME_DESKTOP(deskname)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param)
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)
#define _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR(name)
#define _EVASGL_EXT_ENABLE_GL_GLES 1
#define _EVASGL_EXT_ENABLE_EGL 1

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_ENABLE_EGL
#undef _EVASGL_EXT_ENABLE_GL_GLES
#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME_PRINT
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_DRVNAME_DESKTOP
#undef _EVASGL_EXT_DRVNAME_PRIVATE
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#undef _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR
   /////////////////////////////////////////////////////////////////////////////////////////////////////

   if (_gl_ext_string) free(_gl_ext_string);
   if (_gl_ext_string_official) free(_gl_ext_string_official);
   _gl_ext_string = eina_strbuf_string_steal(sb);
   _gl_ext_string_official = eina_strbuf_string_steal(sboff);
   eina_strbuf_free(sb);
   eina_strbuf_free(sboff);

  _evgl_api_ext_status |= EVASGL_API_GLES2_EXT_INITIALIZED;
   return EINA_TRUE;
}

void
evgl_api_gles2_ext_get(Evas_GL_API *gl_funcs, void *getproc, const char *glueexts)
{
   if (!(_evgl_api_ext_status & EVASGL_API_GLES2_EXT_INITIALIZED))
     {
        DBG("Initializing GLESv2 extensions...");
        if (!_evgl_api_gles2_ext_init(getproc, glueexts))
          {
             ERR("GLESv2 extensions initialization failed");
             return;
          }
     }
#define ORD(f) EVAS_API_OVERRIDE(f, gl_funcs, gl_ext_sym_)

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
#define _EVASGL_EXT_DRVNAME_PRIVATE(name)
#define _EVASGL_EXT_DRVNAME_DESKTOP(deskname)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param) \
   ORD(name);
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END()
#define _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(ret, name, param)
#define _EVASGL_EXT_FUNCTION_PRIVATE_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)
#define _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR(name)

#undef _EVASGL_EXT_WHITELIST_ONLY
#define _EVASGL_EXT_WHITELIST_ONLY 0

#define _EVASGL_EXT_ENABLE_GL_GLES 1
#define _EVASGL_EXT_ENABLE_EGL 1

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_ENABLE_EGL
#undef _EVASGL_EXT_ENABLE_GL_GLES
#undef _EVASGL_EXT_WHITELIST_ONLY
#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_DRVNAME_PRIVATE
#undef _EVASGL_EXT_DRVNAME_DESKTOP
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END
#undef _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN
#undef _EVASGL_EXT_FUNCTION_PRIVATE_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#undef _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR
   /////////////////////////////////////////////////////////////////////////////////////////////////////
#undef ORD

}

Eina_Bool
_evgl_api_gles1_ext_init(void *getproc, const char *glueexts)
{
   const char *glexts;
   fp_getproc gp = (fp_getproc)getproc;
   int _curext_supported = 0;
   Evas_GL_API *gles1_funcs;
   Eina_Strbuf *sb = eina_strbuf_new();
   Eina_Strbuf *sboff = eina_strbuf_new();

#ifdef GL_GLES
   EVGL_Resource *rsc;
   EGLint context_version;
   EGLDisplay dpy = EGLDISPLAY_GET();

   /* glGetString returns the information for the currently bound context
    * So, update glexts only if GLES1 context is currently bound.
    * Check here if GLESv1 is current
    */
   if (!(rsc=_evgl_tls_resource_get()))
     {
        ERR("Unable to initialize GLES1 extensions. Error retrieving tls");
        return EINA_FALSE;
     }

   if ((dpy == EGL_NO_DISPLAY) || !rsc->current_ctx)
     {
        DBG("Unable to initialize GLES1 extensions. Engine not initialized");
        return EINA_FALSE;
     }

   if (!eglQueryContext(dpy, rsc->current_ctx->context, EGL_CONTEXT_CLIENT_VERSION, &context_version))
     {
        ERR("Unable to initialize GLES1 extensions. eglQueryContext failed 0x%x", eglGetError());
        return EINA_FALSE;
     }

   if (context_version != EVAS_GL_GLES_1_X)
     {
        DBG("GLESv1 context not bound");
        return EINA_FALSE;
     }
#endif

   gles1_funcs = _evgl_api_gles1_internal_get();
   if (!gles1_funcs || !gles1_funcs->glGetString)
     {
        ERR("Could not get address of glGetString in GLESv1 library!");
        return EINA_FALSE;
     }

   glexts = (const char *) gles1_funcs->glGetString(GL_EXTENSIONS);
   if (!glexts)
     {
        ERR("GLESv1:glGetString(GL_EXTENSIONS) returned NULL!");
        return EINA_FALSE;
     }

   /////////////////////////////////////////////////////////////////////////////////////////////////////
   // Scanning supported extensions, sets the variables
   /////////////////////////////////////////////////////////////////////////////////////////////////////

   // Preparing all the magic macros
#define GETPROCADDR(sym) \
   (((!(*drvfunc)) && (gp)) ? (__typeof__((*drvfunc)))gp(sym) : (__typeof__((*drvfunc)))dlsym(RTLD_DEFAULT, sym))

#define _EVASGL_EXT_BEGIN(name) \
   { \
      int *ext_support = &_gles1_ext_support_##name; \
      *ext_support = 0;

#define _EVASGL_EXT_END() \
   }

#define _EVASGL_EXT_CHECK_SUPPORT(name) \
   ((strstr(glexts, name) != NULL) || (strstr(glueexts, name) != NULL))

#define _EVASGL_EXT_DISCARD_SUPPORT() \
   *ext_support = 0;

#define _EVASGL_EXT_DRVNAME(name) \
   if (_EVASGL_EXT_CHECK_SUPPORT(#name)) *ext_support = 1;

#define _EVASGL_EXT_DRVNAME_PRIVATE(name) \
   if (_EVASGL_EXT_CHECK_SUPPORT(#name)) { *ext_support = 1; _gles1_ext_support_func_##name = 1; }
#define _EVASGL_EXT_DRVNAME_DESKTOP(deskname) \
   if (_EVASGL_EXT_CHECK_SUPPORT(deskname)) *ext_support = 1;

#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param) \
     { \
        ret (**drvfunc)param = &gles1_ext_sym_##name; \
        if (*ext_support == 1) \
          {

#define _EVASGL_EXT_FUNCTION_END() \
          } \
        if ((*drvfunc) == NULL) _EVASGL_EXT_DISCARD_SUPPORT(); \
     }
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN() \
   if (EINA_FALSE) \
     {
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END() \
     }
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name) \
   if ((*drvfunc) == NULL) *drvfunc = name;

#define _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR(name) \
   if ((*drvfunc) == NULL) \
     { \
        *drvfunc = GETPROCADDR(name); \
        evgl_safe_extension_add(name, (void *) (*drvfunc)); \
     } \
   else evgl_safe_extension_add(name, NULL);

#ifdef _EVASGL_EXT_FUNCTION_WHITELIST
# undef _EVASGL_EXT_FUNCTION_WHITELIST
#endif
#define _EVASGL_EXT_FUNCTION_WHITELIST(name) evgl_safe_extension_add(name, NULL);

#define _EVASGL_EXT_ENABLE_GL_GLES 1
#define _EVASGL_EXT_ENABLE_EGL 1
#define _EVASGL_EXT_GLES1_ONLY 1

   // Okay, now we are ready to scan.
#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_GLES1_ONLY
#undef _EVASGL_EXT_ENABLE_EGL
#undef _EVASGL_EXT_ENABLE_GL_GLES
#undef _EVASGL_EXT_FUNCTION_WHITELIST
#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_DRVNAME_PRIVATE
#undef _EVASGL_EXT_DRVNAME_DESKTOP
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#undef _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR
#undef GETPROCADDR

#define _EVASGL_EXT_BEGIN(name) \
     if (_gles1_ext_support_##name != 0) \
       { \
          eina_strbuf_append(sb, #name" "); \
          _curext_supported = 1; \
       } \
     else _curext_supported = 0;


   /////////////////////////////////////////////////////////////////////////////////////////////////////
   // Scanning again to add to the gles1 ext string list
   /////////////////////////////////////////////////////////////////////////////////////////////////////

#define _EVASGL_EXT_END()
#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()
#define _EVASGL_EXT_DRVNAME_PRINT(name) \
     { \
        eina_strbuf_append(sb, name" "); \
        if ((strncmp(name, "GL_", 3) == 0) && (strstr(eina_strbuf_string_get(sboff), name) == NULL)) \
          eina_strbuf_append(sboff, name" "); \
     }
#define _EVASGL_EXT_DRVNAME(name) \
   if (_curext_supported) \
      _EVASGL_EXT_DRVNAME_PRINT(#name)
#define _EVASGL_EXT_DRVNAME_PRIVATE(name) \
   if (_curext_supported && _gles1_ext_support_func_##name) \
      _EVASGL_EXT_DRVNAME_PRINT(#name)
#define _EVASGL_EXT_DRVNAME_DESKTOP(deskname)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param)
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)
#define _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR(name)
#define _EVASGL_EXT_ENABLE_GL_GLES 1
#define _EVASGL_EXT_ENABLE_EGL 1

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_ENABLE_EGL
#undef _EVASGL_EXT_ENABLE_GL_GLES
#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME_PRINT
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_DRVNAME_PRIVATE
#undef _EVASGL_EXT_DRVNAME_DESKTOP
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#undef _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR

   if (_gles1_ext_string) free(_gles1_ext_string);
   if (_gles1_ext_string_official) free(_gles1_ext_string_official);
   _gles1_ext_string = eina_strbuf_string_steal(sb);
   _gles1_ext_string_official = eina_strbuf_string_steal(sboff);
   eina_strbuf_free(sb);
   eina_strbuf_free(sboff);

   if (evgl_engine->api_debug_mode)
     DBG("GLES1: List of supported extensions:\n%s", _gles1_ext_string);

   // GLESv1 version has been initialized!
   _evgl_api_ext_status |= EVASGL_API_GLES1_EXT_INITIALIZED;
   return EINA_TRUE;
}

void
evgl_api_gles1_ext_get(Evas_GL_API *gl_funcs, void *getproc, const char *glueexts)
{
   if (!(_evgl_api_ext_status & EVASGL_API_GLES1_EXT_INITIALIZED))
     {
        DBG("Initializing GLESv1 extensions...");
        if (!_evgl_api_gles1_ext_init(getproc, glueexts))
          {
             ERR("GLESv1 extensions initialization failed");
             return;
          }
     }

#define ORD(f) EVAS_API_OVERRIDE(f, gl_funcs, gles1_ext_sym_)

   /////////////////////////////////////////////////////////////////////////////////////////////////////
   // Extension HEADER
   /////////////////////////////////////////////////////////////////////////////////////////////////////
#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()
#define _EVASGL_EXT_BEGIN(name) \
   if (_gles1_ext_support_##name != 0) \
     {
#define _EVASGL_EXT_END() \
     }
#define _EVASGL_EXT_DRVNAME(name)
#define _EVASGL_EXT_DRVNAME_PRIVATE(name)
#define _EVASGL_EXT_DRVNAME_DESKTOP(deskname)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param) \
   ORD(name);
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END()
#define _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(ret, name, param)
#define _EVASGL_EXT_FUNCTION_PRIVATE_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)
#define _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR(name)

#undef _EVASGL_EXT_WHITELIST_ONLY
#define _EVASGL_EXT_WHITELIST_ONLY 0
#define _EVASGL_EXT_ENABLE_GL_GLES 1
#define _EVASGL_EXT_ENABLE_EGL 1

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_ENABLE_EGL
#undef _EVASGL_EXT_ENABLE_GL_GLES
#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_DRVNAME_PRIVATE
#undef _EVASGL_EXT_DRVNAME_DESKTOP
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END
#undef _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN
#undef _EVASGL_EXT_FUNCTION_PRIVATE_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#undef _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR
   /////////////////////////////////////////////////////////////////////////////////////////////////////
#undef ORD

}

Eina_Bool
_evgl_api_gles3_ext_init(void *getproc, const char *glueexts)
{
   const char *glexts;
   fp_getproc gp = (fp_getproc)getproc;
   int _curext_supported = 0;
   Evas_GL_API *gles3_funcs;
   Eina_Strbuf *sb = eina_strbuf_new();
   Eina_Strbuf *sboff = eina_strbuf_new();

#ifdef GL_GLES
   EVGL_Resource *rsc;
   EGLint context_version;
   EGLDisplay dpy = EGLDISPLAY_GET();

   /* glGetString returns the information for the currently bound context
    * So, update gles3_exts only if GLES3 context is currently bound.
    * Check here if GLESv3 is current
    */
   if (!(rsc=_evgl_tls_resource_get()))
     {
        ERR("Unable to initialize GLES3 extensions. Error retrieving tls");
        return EINA_FALSE;
     }

   if ((dpy == EGL_NO_DISPLAY) || !rsc->current_ctx)
     {
        DBG("Unable to initialize GLES3 extensions. Engine not initialized");
        return EINA_FALSE;
     }

   if (!eglQueryContext(dpy, rsc->current_ctx->context, EGL_CONTEXT_CLIENT_VERSION, &context_version))
     {
        ERR("Unable to initialize GLES3 extensions. eglQueryContext failed 0x%x", eglGetError());
        return EINA_FALSE;
     }

   if (context_version != EVAS_GL_GLES_3_X)
     {
        DBG("GLESv3 context not bound");
        return EINA_FALSE;
     }
#endif

   gles3_funcs = _evgl_api_gles3_internal_get();
   if (!gles3_funcs || !gles3_funcs->glGetString)
     {
        ERR("Could not get address of glGetString in GLESv3 library!");
        return EINA_FALSE;
     }

   glexts = (const char *) gles3_funcs->glGetString(GL_EXTENSIONS);
   if (!glexts)
     {
        ERR("GLESv3:glGetString(GL_EXTENSIONS) returned NULL!");
        return EINA_FALSE;
     }

   /////////////////////////////////////////////////////////////////////////////////////////////////////
   // Scanning supported extensions, sets the variables
   /////////////////////////////////////////////////////////////////////////////////////////////////////

   // Preparing all the magic macros
#define GETPROCADDR(sym) \
      (((!(*drvfunc)) && (gp)) ? (__typeof__((*drvfunc)))gp(sym) : (__typeof__((*drvfunc)))dlsym(RTLD_DEFAULT, sym))

#define _EVASGL_EXT_BEGIN(name) \
   { \
      int *ext_support = &_gles3_ext_support_##name; \
      *ext_support = 0;

#define _EVASGL_EXT_END() \
   }

#define _EVASGL_EXT_CHECK_SUPPORT(name) \
   ((strstr(glexts, name) != NULL) || (strstr(glueexts, name) != NULL))

#define _EVASGL_EXT_DISCARD_SUPPORT() \
   *ext_support = 0;

#define _EVASGL_EXT_DRVNAME(name) \
   if (_EVASGL_EXT_CHECK_SUPPORT(#name)) *ext_support = 1;

#define _EVASGL_EXT_DRVNAME_PRIVATE(name) \
   if (_EVASGL_EXT_CHECK_SUPPORT(#name)) { *ext_support = 1; _gles3_ext_support_func_##name = 1; }
#define _EVASGL_EXT_DRVNAME_DESKTOP(deskname) \
   if (_EVASGL_EXT_CHECK_SUPPORT(deskname)) *ext_support = 1;

#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param) \
     { \
        ret (**drvfunc)param = &gles3_ext_sym_##name; \
        if (*ext_support == 1) \
          {

#define _EVASGL_EXT_FUNCTION_END() \
          } \
        if ((*drvfunc) == NULL) _EVASGL_EXT_DISCARD_SUPPORT(); \
     }
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name) \
   if ((*drvfunc) == NULL) *drvfunc = name;

#define _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR(name) \
   if ((*drvfunc) == NULL) \
     { \
        *drvfunc = GETPROCADDR(name); \
        evgl_safe_extension_add(name, (void *) (*drvfunc)); \
     } \
   else evgl_safe_extension_add(name, NULL);

#ifdef _EVASGL_EXT_FUNCTION_WHITELIST
# undef _EVASGL_EXT_FUNCTION_WHITELIST
#endif
#define _EVASGL_EXT_FUNCTION_WHITELIST(name) evgl_safe_extension_add(name, NULL);

#define _EVASGL_EXT_ENABLE_GL_GLES 1
#define _EVASGL_EXT_ENABLE_EGL 1

   // Okay, now we are ready to scan.
#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_ENABLE_EGL
#undef _EVASGL_EXT_ENABLE_GL_GLES
#undef _EVASGL_EXT_FUNCTION_WHITELIST
#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_DRVNAME_PRIVATE
#undef _EVASGL_EXT_DRVNAME_DESKTOP
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#undef _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR
#undef GETPROCADDR

#define _EVASGL_EXT_BEGIN(name) \
     if (_gles3_ext_support_##name != 0) \
       { \
          eina_strbuf_append(sb, #name" "); \
          _curext_supported = 1; \
       } \
     else _curext_supported = 0;


   /////////////////////////////////////////////////////////////////////////////////////////////////////
   // Scanning again to add to the gles3 ext string list
   /////////////////////////////////////////////////////////////////////////////////////////////////////

#define _EVASGL_EXT_END()
#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()
#define _EVASGL_EXT_DRVNAME_PRINT(name) \
     { \
        eina_strbuf_append(sb, name" "); \
        if ((strncmp(name, "GL_", 3) == 0) && (strstr(eina_strbuf_string_get(sboff), name) == NULL)) \
          eina_strbuf_append(sboff, name" "); \
     }
#define _EVASGL_EXT_DRVNAME(name) \
   if (_curext_supported) \
      _EVASGL_EXT_DRVNAME_PRINT(#name)
#define _EVASGL_EXT_DRVNAME_PRIVATE(name) \
   if (_curext_supported && _gles3_ext_support_func_##name) \
      _EVASGL_EXT_DRVNAME_PRINT(#name)
#define _EVASGL_EXT_DRVNAME_DESKTOP(deskname)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param)
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)
#define _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR(name)

#define _EVASGL_EXT_ENABLE_GL_GLES 1
#define _EVASGL_EXT_ENABLE_EGL 1

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_ENABLE_EGL
#undef _EVASGL_EXT_ENABLE_GL_GLES
#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME_PRINT
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_DRVNAME_PRIVATE
#undef _EVASGL_EXT_DRVNAME_DESKTOP
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#undef _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR

   if (_gles3_ext_string) free(_gles3_ext_string);
   if (_gles3_ext_string_official) free(_gles3_ext_string_official);
   _gles3_ext_string = eina_strbuf_string_steal(sb);
   _gles3_ext_string_official = eina_strbuf_string_steal(sboff);
   eina_strbuf_free(sb);
   eina_strbuf_free(sboff);

   if (evgl_engine->api_debug_mode)
     DBG("GLES3: List of supported extensions:\n%s", _gles3_ext_string);

   // GLESv3 version has been initialized!
   _evgl_api_ext_status |= EVASGL_API_GLES3_EXT_INITIALIZED;
   return EINA_TRUE;
}

void
evgl_api_gles3_ext_get(Evas_GL_API *gl_funcs, void *getproc, const char *glueexts)
{
   if (!(_evgl_api_ext_status & EVASGL_API_GLES3_EXT_INITIALIZED))
     {
        DBG("Initializing GLESv3 extensions...");
        if (!_evgl_api_gles3_ext_init(getproc, glueexts))
          {
             ERR("GLESv3 extensions initialization failed");
             return;
          }
     }

#define ORD(f) EVAS_API_OVERRIDE(f, gl_funcs, gles3_ext_sym_)

   /////////////////////////////////////////////////////////////////////////////////////////////////////
   // Extension HEADER
   /////////////////////////////////////////////////////////////////////////////////////////////////////
#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()
#define _EVASGL_EXT_BEGIN(name) \
   if (_gles3_ext_support_##name != 0) \
     {
#define _EVASGL_EXT_END() \
     }
#define _EVASGL_EXT_DRVNAME(name)
#define _EVASGL_EXT_DRVNAME_PRIVATE(name)
#define _EVASGL_EXT_DRVNAME_DESKTOP(deskname)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param) \
   ORD(name);
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END()
#define _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(ret, name, param)
#define _EVASGL_EXT_FUNCTION_PRIVATE_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)
#define _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR(name)

#undef _EVASGL_EXT_WHITELIST_ONLY
#define _EVASGL_EXT_WHITELIST_ONLY 0

#define _EVASGL_EXT_ENABLE_GL_GLES 1
#define _EVASGL_EXT_ENABLE_EGL 1

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_ENABLE_EGL
#undef _EVASGL_EXT_ENABLE_GL_GLES
#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_DRVNAME_PRIVATE
#undef _EVASGL_EXT_DRVNAME_DESKTOP
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END
#undef _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN
#undef _EVASGL_EXT_FUNCTION_PRIVATE_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#undef _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR
   /////////////////////////////////////////////////////////////////////////////////////////////////////
#undef ORD

}

const char *
evgl_api_ext_egl_string_get(void)
{
#ifdef GL_GLES
   if (!(_evgl_api_ext_status & EVASGL_API_EGL_EXT_INITIALIZED))
     {
        ERR("EVGL extension for egl is not yet initialized.");
        return NULL;
     }

   return _egl_ext_string;
#else
   return "";
#endif
}

const char *
evgl_api_ext_string_get(Eina_Bool official, int version)
{
   if (_evgl_api_ext_status < 1)
     {
        ERR("EVGL extension is not yet initialized.");
        return NULL;
     }

   if (version == EVAS_GL_GLES_1_X)
     return (official?_gles1_ext_string_official:_gles1_ext_string);

   if (version == EVAS_GL_GLES_3_X)
     return (official?_gles3_ext_string_official:_gles3_ext_string);

   return (official?_gl_ext_string_official:_gl_ext_string);
}
