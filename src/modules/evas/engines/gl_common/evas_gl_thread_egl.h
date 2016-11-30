#ifndef EVAS_GL_THREAD_EGL_H
#define EVAS_GL_THREAD_EGL_H

#ifdef GL_GLES

#include <EGL/egl.h>

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

/* EGL 1.4 Referencing to Thread Local Storage */
EAPI EGLint     evas_eglGetError_th();
EAPI EGLBoolean evas_eglBindAPI_th(EGLenum api);
EAPI EGLenum    evas_eglQueryAPI_th();
EAPI EGLBoolean evas_eglMakeCurrent_th(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
EAPI EGLContext evas_eglGetCurrentContext_th(void);
EAPI EGLSurface evas_eglGetCurrentSurface_th(EGLint readdraw);
EAPI EGLDisplay evas_eglGetCurrentDisplay_th(void);
EAPI EGLBoolean evas_eglReleaseThread_th();


/* EGL 1.4 Sequential Operations */
EAPI EGLBoolean evas_eglQuerySurface_th(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
EAPI EGLBoolean evas_eglSwapInterval_th(EGLDisplay dpy, EGLint interval);
EAPI EGLBoolean evas_eglWaitGL_th(void);
EAPI EGLBoolean evas_eglSwapBuffers_th(EGLDisplay dpy, EGLSurface surface);

EAPI void       eglSwapBuffersWithDamage_orig_evas_set(void *func);
EAPI void      *eglSwapBuffersWithDamage_orig_evas_get();
EAPI EGLBoolean evas_eglSwapBuffersWithDamage_th(EGLDisplay dpy, EGLSurface surface, EGLint *rects, EGLint n_rects);

EAPI void       eglSetDamageRegion_orig_evas_set(void *func);
EAPI void      *eglSetDamageRegion_orig_evas_get();
EAPI EGLBoolean evas_eglSetDamageRegion_th(EGLDisplay dpy, EGLSurface surface, EGLint *rects, EGLint n_rects);

EAPI void       eglQueryWaylandBuffer_orig_evas_set(void *func);
EAPI void      *eglQueryWaylandBuffer_orig_evas_get();
EAPI EGLBoolean evas_eglQueryWaylandBuffer_th(EGLDisplay dpy, void *buffer, EGLint attribute, EGLint *value);


/***** EVAS GL *****/

/* EGL 1.4 Referencing to Thread Local Storage */
EAPI EGLint     evas_eglGetError_evgl_th();
EAPI EGLBoolean evas_eglMakeCurrent_evgl_th(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
EAPI EGLContext evas_eglGetCurrentContext_evgl_th(void);
EAPI EGLSurface evas_eglGetCurrentSurface_evgl_th(EGLint readdraw);
EAPI EGLDisplay evas_eglGetCurrentDisplay_evgl_th(void);


#else /* ! EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */


/* EGL 1.4 Referencing to Thread Local Storage */
extern EGLint     (*evas_eglGetError_th)();
extern EGLBoolean (*evas_eglBindAPI_th)(EGLenum api);
extern EGLenum    (*evas_eglQueryAPI_th)();
extern EGLBoolean (*evas_eglMakeCurrent_th)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
extern EGLContext (*evas_eglGetCurrentContext_th)(void);
extern EGLSurface (*evas_eglGetCurrentSurface_th)(EGLint readdraw);
extern EGLDisplay (*evas_eglGetCurrentDisplay_th)(void);
extern EGLBoolean (*evas_eglReleaseThread_th)();


/* EGL 1.4 Sequential Operations */
extern EGLBoolean (*evas_eglQuerySurface_th)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
extern EGLBoolean (*evas_eglSwapInterval_th)(EGLDisplay dpy, EGLint interval);
extern EGLBoolean (*evas_eglWaitGL_th)(void);
extern EGLBoolean (*evas_eglSwapBuffers_th)(EGLDisplay dpy, EGLSurface surface);
extern void       (*eglSwapBuffersWithDamage_orig_evas_set)(void *func);
extern void      *(*eglSwapBuffersWithDamage_orig_evas_get)();
extern EGLBoolean (*evas_eglSwapBuffersWithDamage_th)(EGLDisplay dpy, EGLSurface surface, EGLint *rects, EGLint n_rects);
extern void       (*eglSetDamageRegion_orig_evas_set)(void *func);
extern void      *(*eglSetDamageRegion_orig_evas_get)();
extern EGLBoolean (*evas_eglSetDamageRegion_th)(EGLDisplay dpy, EGLSurface surface, EGLint *rects, EGLint n_rects);
extern void       (*eglQueryWaylandBuffer_orig_evas_set)(void *func);
extern void      *(*eglQueryWaylandBuffer_orig_evas_get)();
extern EGLBoolean (*evas_eglQueryWaylandBuffer_th)(EGLDisplay dpy, void *buffer, EGLint attribute, EGLint *value);


/***** EVAS GL *****/

/* EGL 1.4 Referencing to Thread Local Storage */
extern EGLint     (*evas_eglGetError_evgl_th)();
extern EGLBoolean (*evas_eglMakeCurrent_evgl_th)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
extern EGLContext (*evas_eglGetCurrentContext_evgl_th)(void);
extern EGLSurface (*evas_eglGetCurrentSurface_evgl_th)(EGLint readdraw);
extern EGLDisplay (*evas_eglGetCurrentDisplay_evgl_th)(void);



extern void _egl_thread_link_init();


#endif /* EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */

#endif /* GL_GLES */

#endif
