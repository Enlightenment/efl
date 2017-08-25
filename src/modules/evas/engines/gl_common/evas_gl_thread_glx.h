#ifndef EVAS_GL_THREAD_GLX_H
#define EVAS_GL_THREAD_GLX_H

#ifndef GL_GLES

#include <GL/glx.h>

extern void *evas_gl_thread_glx_func_get(void);


#define EVAS_TH_GLX_FN_LIST \
EVAS_TH_GLX_FN      (void          , glXBindTexImage, Display *dpy, GLXDrawable drawable, int buffer, const int *attrib_list) \
EVAS_TH_GLX_FN      (GLXFBConfig  *, glXChooseFBConfig, Display *dpy, int screen, const int *attribList, int *nitems) \
EVAS_TH_GLX_FN      (GLXContext    , glXCreateContext, Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct) \
EVAS_TH_GLX_FN      (GLXContext    , glXCreateNewContext, Display *dpy, GLXFBConfig config, int renderType, GLXContext shareList, Bool direct) \
EVAS_TH_GLX_FN      (GLXPbuffer    , glXCreatePbuffer, Display *dpy, GLXFBConfig config, const int *attribList) \
EVAS_TH_GLX_FN      (GLXPixmap     , glXCreatePixmap, Display *dpy, GLXFBConfig config, Pixmap pixmap, const int *attribList) \
EVAS_TH_GLX_FN      (GLXWindow     , glXCreateWindow, Display *dpy, GLXFBConfig config, Window win, const int *attribList) \
EVAS_TH_GLX_FN      (void          , glXDestroyContext, Display *dpy, GLXContext ctx) \
EVAS_TH_GLX_FN      (void          , glXDestroyPbuffer, Display *dpy, GLXPbuffer pbuf) \
EVAS_TH_GLX_FN      (void          , glXDestroyPixmap, Display *dpy, GLXPixmap pixmap) \
EVAS_TH_GLX_FN      (void          , glXDestroyWindow, Display *dpy, GLXWindow window) \
EVAS_TH_GLX_FN      (int           , glXGetConfig, Display *dpy, XVisualInfo *visual, int attrib, int *value) \
EVAS_TH_GLX_FN      (GLXContext    , glXGetCurrentContext) \
EVAS_TH_GLX_FN      (int           , glXGetFBConfigAttrib, Display *dpy, GLXFBConfig config, int attribute, int *value) \
EVAS_TH_GLX_FN      (int           , glXGetVideoSync, unsigned int *count) \
EVAS_TH_GLX_FN      (XVisualInfo  *, glXGetVisualFromFBConfig, Display *dpy, GLXFBConfig config) \
EVAS_TH_GLX_FN      (Bool          , glXMakeContextCurrent, Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx) \
EVAS_TH_GLX_FN_ASYNC(Bool          , glXMakeContextCurrentASYNC, Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx) \
EVAS_TH_GLX_FN      (int           , glXQueryDrawable, Display *dpy, GLXDrawable draw, int attribute, unsigned int *value) \
EVAS_TH_GLX_FN      (Bool          , glXQueryExtension, Display *dpy, int *errorb, int *event) \
EVAS_TH_GLX_FN      (const char   *, glXQueryExtensionsString, Display *dpy, int screen) \
EVAS_TH_GLX_FN      (Bool          , glXReleaseBuffersMESA, Display *dpy, GLXDrawable drawable) \
EVAS_TH_GLX_FN      (void          , glXReleaseTexImage, Display *dpy, GLXDrawable drawable, int buffer) \
EVAS_TH_GLX_FN      (void          , glXSwapBuffers, Display *dpy, GLXDrawable drawable) \
EVAS_TH_GLX_FN      (void          , glXSwapIntervalEXT, Display *dpy, GLXDrawable drawable, int interval) \
EVAS_TH_GLX_FN      (int           , glXSwapIntervalSGI, int interval) \
EVAS_TH_GLX_FN      (int           , glXWaitVideoSync, int divisor, int remainder, unsigned int *count)


#ifdef EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC


#define GL_GLEXT_PROTOTYPES

#define EVAS_TH_GLX_FN(ret, name, ...) \
 extern ret GL_TH_FN(name)(GL_TH_DP, ##__VA_ARGS__);
#define EVAS_TH_GLX_FN_ASYNC(ret, name, ...) \
 extern void *GL_TH_FN(name##_begin)(GL_TH_DP, ##__VA_ARGS__); \
 extern ret GL_TH_FN(name##_end)(void *ref);

EVAS_TH_GLX_FN_LIST

#undef EVAS_TH_GLX_FN_ASYNC
#undef EVAS_TH_GLX_FN


#else /* ! EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */


#define EVAS_TH_GLX_FN(ret, name, ...) \
 extern ret (*GL_TH_FN(name))(GL_TH_DP, ##__VA_ARGS__);
#define EVAS_TH_GLX_FN_ASYNC(ret, name, ...) \
 extern void *(*GL_TH_FN(name##_begin))(GL_TH_DP, ##__VA_ARGS__); \
 extern ret (*GL_TH_FN(name##_end))(void *ref);

EVAS_TH_GLX_FN_LIST

#undef EVAS_TH_GLX_FN_ASYNC
#undef EVAS_TH_GLX_FN

extern void _glx_thread_link_init(void *func_ptr);


#endif /* EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */


#endif /* ! GL_GLES */

#endif /* EVAS_GL_THREAD_GLX_H */
