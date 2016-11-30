#ifndef EVAS_GL_THREAD_GLX_H
#define EVAS_GL_THREAD_GLX_H

#ifndef GL_GLES

# include <GL/glx.h>

#ifdef EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC

#define GL_GLEXT_PROTOTYPES

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


EAPI void         glXBindTexImage_orig_evas_set(void *func);
EAPI void        *glXBindTexImage_orig_evas_get(void);
EAPI void         evas_glXBindTexImage_th(Display *dpy, GLXDrawable drawable, int buffer, const int *attrib_list);
EAPI GLXFBConfig *evas_glXChooseFBConfig_th(Display *dpy, int screen, const int *attribList, int *nitems);
EAPI GLXContext   evas_glXCreateContext_th(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct);
EAPI GLXContext   evas_glXCreateNewContext_th(Display *dpy, GLXFBConfig config, int renderType, GLXContext shareList, Bool direct);
EAPI GLXPbuffer   evas_glXCreatePbuffer_th(Display *dpy, GLXFBConfig config, const int *attribList);
EAPI void         glXCreatePixmap_orig_evas_set(void *func);
EAPI void        *glXCreatePixmap_orig_evas_get(void);
EAPI GLXPixmap    evas_glXCreatePixmap_th(Display *dpy, GLXFBConfig config, Pixmap pixmap, const int *attribList);
EAPI GLXWindow    evas_glXCreateWindow_th(Display *dpy, GLXFBConfig config, Window win, const int *attribList);
EAPI void         evas_glXDestroyContext_th(Display *dpy, GLXContext ctx);
EAPI void         evas_glXDestroyPbuffer_th(Display *dpy, GLXPbuffer pbuf);
EAPI void         glXDestoyPixmap_orig_evas_set(void *func);
EAPI void        *glXDestoyPixmap_orig_evas_get(void);
EAPI void         evas_glXDestroyPixmap_th(Display *dpy, GLXPixmap pixmap);
EAPI void         evas_glXDestroyWindow_th(Display *dpy, GLXWindow window);
EAPI int          evas_glXGetConfig_th(Display *dpy, XVisualInfo *visual, int attrib, int *value);
EAPI GLXContext   evas_glXGetCurrentContext_th(void);
EAPI int          evas_glXGetFBConfigAttrib_th(Display *dpy, GLXFBConfig config, int attribute, int *value);

EAPI void         glXGetVideoSync_orig_evas_set(void *func);
EAPI void        *glXGetVideoSync_orig_evas_get(void);
EAPI int          evas_glXGetVideoSync_th(unsigned int *count);
EAPI XVisualInfo *evas_glXGetVisualFromFBConfig_th(Display *dpy, GLXFBConfig config);
EAPI Bool         evas_glXMakeContextCurrent_th(Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx);
EAPI void         glXQueryDrawable_orig_evas_set(void *func);
EAPI void        *glXQueryDrawable_orig_evas_get(void);
EAPI void         evas_glXQueryDrawable_th(Display *dpy, GLXDrawable draw, int attribute, unsigned int *value);
EAPI Bool         evas_glXQueryExtension_th(Display *dpy, int *errorb, int *event);
EAPI const char  *evas_glXQueryExtensionsString_th(Display *dpy, int screen);

EAPI void         glXReleaseBuffersMESA_orig_evas_set(void *func);
EAPI void        *glXReleaseBuffersMESA_orig_evas_get(void);
EAPI Bool         evas_glXReleaseBuffersMESA_th(Display *dpy, GLXDrawable drawable);

EAPI void         glXReleaseTexImageEXT_orig_evas_set(void *func);
EAPI void        *glXReleaseTexImageEXT_orig_evas_get(void);
EAPI void         evas_glXReleaseTexImageEXT_th(Display *dpy, GLXDrawable drawable, int buffer);
EAPI void         evas_glXSwapBuffers_th(Display *dpy, GLXDrawable drawable);

EAPI void         glXSwapIntervalEXT_orig_evas_set(void *func);
EAPI void        *glXSwapIntervalEXT_orig_evas_get(void);
EAPI void         evas_glXSwapIntervalEXT_th(Display *dpy, GLXDrawable drawable, int interval);

EAPI void         glXSwapIntervalSGI_orig_evas_set(void *func);
EAPI void        *glXSwapIntervalSGI_orig_evas_get(void);
EAPI int          evas_glXSwapIntervalSGI_th(int interval);

EAPI void         glXWaitVideoSync_orig_evas_set(void *func);
EAPI void        *glXWaitVideoSync_orig_evas_get(void);
EAPI int          evas_glXWaitVideoSync_th(int divisor, int remainder, unsigned int *count);

/****** EVAS GL ******/
EAPI void         glXBindTexImage_orig_evgl_set(void *func);
EAPI void        *glXBindTexImage_orig_evgl_get(void);
EAPI void         evas_glXBindTexImage_evgl_th(Display *dpy, GLXDrawable drawable, int buffer, const int *attrib_list);
EAPI GLXFBConfig *evas_glXChooseFBConfig_evgl_th(Display *dpy, int screen, const int *attribList, int *nitems);
EAPI GLXContext   evas_glXCreateContext_evgl_th(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct);
EAPI GLXContext   evas_glXCreateNewContext_evgl_th(Display *dpy, GLXFBConfig config, int renderType, GLXContext shareList, Bool direct);
EAPI GLXPbuffer   evas_glXCreatePbuffer_evgl_th(Display *dpy, GLXFBConfig config, const int *attribList);

EAPI void         glXCreatePixmap_orig_evgl_set(void *func);
EAPI void        *glXCreatePixmap_orig_evgl_get(void);
EAPI GLXPixmap    evas_glXCreatePixmap_evgl_th(Display *dpy, GLXFBConfig config, Pixmap pixmap, const int *attribList);
EAPI GLXWindow    evas_glXCreateWindow_evgl_th(Display *dpy, GLXFBConfig config, Window win, const int *attribList);
EAPI void         evas_glXDestroyContext_evgl_th(Display *dpy, GLXContext ctx);
EAPI void         evas_glXDestroyPbuffer_evgl_th(Display *dpy, GLXPbuffer pbuf);

EAPI void         glXDestroyPixmap_orig_evgl_set(void *func);
EAPI void        *glXDestroyPixmap_orig_evgl_get(void);
EAPI void         evas_glXDestroyPixmap_evgl_th(Display *dpy, GLXPixmap pixmap);
EAPI void         evas_glXDestroyWindow_evgl_th(Display *dpy, GLXWindow window);
EAPI int          evas_glXGetConfig_evgl_th(Display *dpy, XVisualInfo *visual, int attrib, int *value);
EAPI GLXContext   evas_glXGetCurrentContext_evgl_th(void);
EAPI int          evas_glXGetFBConfigAttrib_evgl_th(Display *dpy, GLXFBConfig config, int attribute, int *value);

EAPI void         glXGetVideoSync_orig_evgl_set(void *func);
EAPI void        *glXGetVideoSync_orig_evgl_get(void);
EAPI int          evas_glXGetVideoSync_evgl_th(unsigned int *count);
EAPI XVisualInfo *evas_glXGetVisualFromFBConfig_evgl_th(Display *dpy, GLXFBConfig config);
EAPI Bool         evas_glXMakeContextCurrent_evgl_th(Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx);

EAPI void         glXQueryDrawable_orig_evgl_set(void *func);
EAPI void        *glXQueryDrawable_orig_evgl_get(void);
EAPI void         evas_glXQueryDrawable_evgl_th(Display *dpy, GLXDrawable draw, int attribute, unsigned int *value);
EAPI Bool         evas_glXQueryExtension_evgl_th(Display *dpy, int *errorb, int *event);
EAPI const char  *evas_glXQueryExtensionsString_evgl_th(Display *dpy, int screen);

EAPI void         glXReleaseBuffersMESA_orig_evgl_set(void *func);
EAPI void        *glXReleaseBuffersMESA_orig_evgl_get(void);
EAPI Bool         evas_glXReleaseBuffersMESA_evgl_th(Display *dpy, GLXDrawable drawable);

EAPI void         glXReleaseTexImage_orig_evgl_set(void *func);
EAPI void        *glXReleaseTexImage_orig_evgl_get(void);
EAPI void         evas_glXReleaseTexImage_evgl_th(Display *dpy, GLXDrawable drawable, int buffer);
EAPI void         evas_glXSwapBuffers_evgl_th(Display *dpy, GLXDrawable drawable);

EAPI void         glXSwapIntervalEXT_orig_evgl_set(void *func);
EAPI void        *glXSwapIntervalEXT_orig_evgl_get(void);
EAPI void         evas_glXSwapIntervalEXT_evgl_th(Display *dpy, GLXDrawable drawable, int interval);

EAPI void         glXSwapIntervalSGI_orig_evgl_set(void *func);
EAPI void        *glXSwapIntervalSGI_orig_evgl_get(void);
EAPI int          evas_glXSwapIntervalSGI_evgl_th(int interval);

EAPI void         glXWaitVideoSync_orig_evgl_set(void *func);
EAPI void        *glXWaitVideoSync_orig_evgl_get(void);
EAPI int          evas_glXWaitVideoSync_evgl_th(int divisor, int remainder, unsigned int *count);

#else /* ! EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */


extern void         (*glXBindTexImage_orig_evas_set)(void *func);
extern void        *(*glXBindTexImage_orig_evas_get)(void);
extern void         (*evas_glXBindTexImage_th)(Display *dpy, GLXDrawable drawable, int buffer, const int *attrib_list);
extern GLXFBConfig *(*evas_glXChooseFBConfig_th)(Display *dpy, int screen, const int *attribList, int *nitems);
extern GLXContext   (*evas_glXCreateContext_th)(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct);
extern GLXContext   (*evas_glXCreateNewContext_th)(Display *dpy, GLXFBConfig config, int renderType, GLXContext shareList, Bool direct);
extern GLXPbuffer   (*evas_glXCreatePbuffer_th)(Display *dpy, GLXFBConfig config, const int *attribList);
extern void         (*glXCreatePixmap_orig_evas_set)(void *func);
extern void        *(*glXCreatePixmap_orig_evas_get)(void);
extern GLXPixmap    (*evas_glXCreatePixmap_th)(Display *dpy, GLXFBConfig config, Pixmap pixmap, const int *attribList);
extern GLXWindow    (*evas_glXCreateWindow_th)(Display *dpy, GLXFBConfig config, Window win, const int *attribList);
extern void         (*evas_glXDestroyContext_th)(Display *dpy, GLXContext ctx);
extern void         (*evas_glXDestroyPbuffer_th)(Display *dpy, GLXPbuffer pbuf);
extern void         (*glXDestroyPixmap_orig_evas_set)(void *func);
extern void        *(*glXDestroyPixmap_orig_evas_get)(void);
extern void         (*evas_glXDestroyPixmap_th)(Display *dpy, GLXPixmap pixmap);
extern void         (*evas_glXDestroyWindow_th)(Display *dpy, GLXWindow window);
extern int          (*evas_glXGetConfig_th)(Display *dpy, XVisualInfo *visual, int attrib, int *value);
extern GLXContext   (*evas_glXGetCurrentContext_th)(void);
extern int          (*evas_glXGetFBConfigAttrib_th)(Display *dpy, GLXFBConfig config, int attribute, int *value);
extern void         (*glXGetVideoSync_orig_evas_set)(void *func);
extern void        *(*glXGetVideoSync_orig_evas_get)(void);
extern int          (*evas_glXGetVideoSync_th)(unsigned int *count);
extern XVisualInfo *(*evas_glXGetVisualFromFBConfig_th)(Display *dpy, GLXFBConfig config);
extern Bool         (*evas_glXMakeContextCurrent_th)(Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx);
extern void         (*glXQueryDrawable_orig_evas_set)(void *func);
extern void        *(*glXQueryDrawable_orig_evas_get)(void);
extern void         (*evas_glXQueryDrawable_th)(Display *dpy, GLXDrawable draw, int attribute, unsigned int *value);
extern Bool         (*evas_glXQueryExtension_th)(Display *dpy, int *errorb, int *event);
extern const char  *(*evas_glXQueryExtensionsString_th)(Display *dpy, int screen);
extern void         (*glXReleaseBuffersMESA_orig_evas_set)(void *func);
extern void        *(*glXReleaseBuffersMESA_orig_evas_get)(void);
extern Bool         (*evas_glXReleaseBuffersMESA_th)(Display *dpy, GLXDrawable drawable);
extern void         (*glXReleaseTexImage_orig_evas_set)(void *func);
extern void        *(*glXReleaseTexImage_orig_evas_get)(void);
extern void         (*evas_glXReleaseTexImage_th)(Display *dpy, GLXDrawable drawable, int buffer);
extern void         (*evas_glXSwapBuffers_th)(Display *dpy, GLXDrawable drawable);
extern void         (*glXSwapIntervalEXT_orig_evas_set)(void *func);
extern void        *(*glXSwapIntervalEXT_orig_evas_get)(void);
extern void         (*evas_glXSwapIntervalEXT_th)(Display *dpy, GLXDrawable drawable, int interval);
extern void         (*glXSwapIntervalSGI_orig_evas_set)(void *func);
extern void        *(*glXSwapIntervalSGI_orig_evas_get)(void);
extern int          (*evas_glXSwapIntervalSGI_th)(int interval);
extern void         (*glXWaitVideoSync_orig_evas_set)(void *func);
extern void        *(*glXWaitVideoSync_orig_evas_get)(void);
extern int          (*evas_glXWaitVideoSync_th)(int divisor, int remainder, unsigned int *count);

/****** EVAS GL ******/

extern void         (*glXBindTexImage_orig_evgl_set)(void *func);
extern void        *(*glXBindTexImage_orig_evgl_get)(void);
extern void         (*evas_glXBindTexImage_evgl_th)(Display *dpy, GLXDrawable drawable, int buffer, const int *attrib_list);
extern GLXFBConfig *(*evas_glXChooseFBConfig_evgl_th)(Display *dpy, int screen, const int *attribList, int *nitems);
extern GLXContext   (*evas_glXCreateContext_evgl_th)(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct);
extern GLXContext   (*evas_glXCreateNewContext_evgl_th)(Display *dpy, GLXFBConfig config, int renderType, GLXContext shareList, Bool direct);
extern GLXPbuffer   (*evas_glXCreatePbuffer_evgl_th)(Display *dpy, GLXFBConfig config, const int *attribList);
extern void         (*glXCreatePixmap_orig_evgl_set)(void *func);
extern void        *(*glXCreatePixmap_orig_evgl_get)(void);
extern GLXPixmap    (*evas_glXCreatePixmap_evgl_th)(Display *dpy, GLXFBConfig config, Pixmap pixmap, const int *attribList);
extern GLXWindow    (*evas_glXCreateWindow_evgl_th)(Display *dpy, GLXFBConfig config, Window win, const int *attribList);
extern void         (*evas_glXDestroyContext_evgl_th)(Display *dpy, GLXContext ctx);
extern void         (*evas_glXDestroyPbuffer_evgl_th)(Display *dpy, GLXPbuffer pbuf);
extern void         (*glXDestroyPixmap_orig_evgl_set)(void *func);
extern void        *(*glXDestroyPixmap_orig_evgl_get)(void);
extern void         (*evas_glXDestroyPixmap_evgl_th)(Display *dpy, GLXPixmap pixmap);
extern void         (*evas_glXDestroyWindow_evgl_th)(Display *dpy, GLXWindow window);
extern int          (*evas_glXGetConfig_evgl_th)(Display *dpy, XVisualInfo *visual, int attrib, int *value);
extern GLXContext   (*evas_glXGetCurrentContext_evgl_th)(void);
extern int          (*evas_glXGetFBConfigAttrib_evgl_th)(Display *dpy, GLXFBConfig config, int attribute, int *value);
extern void         (*glXGetVideoSync_orig_evgl_set)(void *func);
extern void        *(*glXGetVideoSync_orig_evgl_get)(void);
extern int          (*evas_glXGetVideoSync_evgl_th)(unsigned int *count);
extern XVisualInfo *(*evas_glXGetVisualFromFBConfig_evgl_th)(Display *dpy, GLXFBConfig config);
extern Bool         (*evas_glXMakeContextCurrent_evgl_th)(Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx);
extern void         (*glXQueryDrawable_orig_evgl_set)(void *func);
extern void        *(*glXQueryDrawable_orig_evgl_get)(void);
extern void         (*evas_glXQueryDrawable_evgl_th)(Display *dpy, GLXDrawable draw, int attribute, unsigned int *value);
extern Bool         (*evas_glXQueryExtension_evgl_th)(Display *dpy, int *errorb, int *event);
extern const char  *(*evas_glXQueryExtensionsString_evgl_th)(Display *dpy, int screen);
extern void         (*glXReleaseBuffersMESA_orig_evgl_set)(void *func);
extern void        *(*glXReleaseBuffersMESA_orig_evgl_get)(void);
extern Bool         (*evas_glXReleaseBuffersMESA_evgl_th)(Display *dpy, GLXDrawable drawable);
extern void         (*glXReleaseTexImage_orig_evgl_set)(void *func);
extern void        *(*glXReleaseTexImage_orig_evgl_get)(void);
extern void         (*evas_glXReleaseTexImage_evgl_th)(Display *dpy, GLXDrawable drawable, int buffer);
extern void         (*evas_glXSwapBuffers_evgl_th)(Display *dpy, GLXDrawable drawable);
extern void         (*glXSwapIntervalEXT_orig_evgl_set)(void *func);
extern void        *(*glXSwapIntervalEXT_orig_evgl_get)(void);
extern void         (*evas_glXSwapIntervalEXT_evgl_th)(Display *dpy, GLXDrawable drawable, int interval);
extern void         (*glXSwapIntervalSGI_orig_evgl_set)(void *func);
extern void        *(*glXSwapIntervalSGI_orig_evgl_get)(void);
extern int          (*evas_glXSwapIntervalSGI_evgl_th)(int interval);
extern void         (*glXWaitVideoSync_orig_evgl_set)(void *func);
extern void        *(*glXWaitVideoSync_orig_evgl_get)(void);
extern int          (*evas_glXWaitVideoSync_evgl_th)(int divisor, int remainder, unsigned int *count);


extern void _glx_thread_link_init();


#endif /* EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */

#endif /* ! GL_GLES */

#endif /* EVAS_GL_THREAD_GLX_H */
