#include "evas_common_private.h" /* Also includes international specific stuff */
#include "evas_engine.h"

//#define TIMDBG 1
#ifdef TIMDBG
# include <sys/time.h>
# include <unistd.h>
#endif

#ifdef HAVE_DLSYM
# include <dlfcn.h>      /* dlopen,dlclose,etc */
#else
# error gl_x11 should not get compiled if dlsym is not found on the system!
#endif

#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif

#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

#define EVAS_GL_UPDATE_TILE_SIZE 16

static int safe_native = -1;

typedef struct _Render_Engine               Render_Engine;

struct _Render_Engine
{
   Render_Engine_GL_Generic generic;
};

const char *debug_dir;
int swap_buffer_debug_mode = -1;
int swap_buffer_debug = 0;
int partial_render_debug = -1;
int extn_have_buffer_age = 1;

static int initted = 0;
static int gl_wins = 0;
#ifdef GL_GLES
static int extn_have_y_inverted = 1;
#endif

typedef void            (*_eng_fn) (void);
typedef _eng_fn         (*glsym_func_eng_fn) ();
typedef void            (*glsym_func_void) ();
typedef void           *(*glsym_func_void_ptr) ();
typedef int             (*glsym_func_int) ();
typedef unsigned int    (*glsym_func_uint) ();
typedef const char     *(*glsym_func_const_char_ptr) ();

Evas_GL_Common_Image_Call glsym_evas_gl_common_image_ref = NULL;
Evas_GL_Common_Image_Call glsym_evas_gl_common_image_unref = NULL;
Evas_GL_Common_Image_Call glsym_evas_gl_common_image_free = NULL;
Evas_GL_Common_Image_Call glsym_evas_gl_common_image_native_disable = NULL;
Evas_GL_Common_Image_Call glsym_evas_gl_common_image_native_enable = NULL;
Evas_GL_Common_Image_New_From_Data glsym_evas_gl_common_image_new_from_data = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_image_all_unload = NULL;
Evas_GL_Preload glsym_evas_gl_preload_init = NULL;
Evas_GL_Preload glsym_evas_gl_preload_shutdown = NULL;
EVGL_Engine_Call glsym_evgl_engine_shutdown = NULL;
Evas_Gl_Symbols glsym_evas_gl_symbols = NULL;

Evas_GL_Common_Context_New glsym_evas_gl_common_context_new = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_context_flush = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_context_free = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_context_use = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_context_newframe = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_context_done = NULL;
Evas_GL_Common_Context_Resize_Call glsym_evas_gl_common_context_resize = NULL;
Evas_GL_Common_Buffer_Dump_Call glsym_evas_gl_common_buffer_dump = NULL;
Evas_GL_Preload_Render_Call glsym_evas_gl_preload_render_lock = NULL;
Evas_GL_Preload_Render_Call glsym_evas_gl_preload_render_unlock = NULL;
Evas_GL_Preload_Render_Call glsym_evas_gl_preload_render_relax = NULL;

#ifdef GL_GLES

#ifndef EGL_NATIVE_PIXMAP_KHR
# define EGL_NATIVE_PIXMAP_KHR 0x30b0
#endif
#ifndef EGL_Y_INVERTED_NOK
# define EGL_Y_INVERTED_NOK 0x307F
#endif

_eng_fn  (*glsym_eglGetProcAddress)            (const char *a) = NULL;
void    *(*glsym_eglCreateImage)               (EGLDisplay a, EGLContext b, EGLenum c, EGLClientBuffer d, const int *e) = NULL;
void     (*glsym_eglDestroyImage)              (EGLDisplay a, void *b) = NULL;
void     (*glsym_glEGLImageTargetTexture2DOES) (int a, void *b)  = NULL;
unsigned int   (*glsym_eglSwapBuffersWithDamage) (EGLDisplay a, void *b, const EGLint *d, EGLint c) = NULL;

#else

typedef XID     (*glsym_func_xid) ();

_eng_fn  (*glsym_glXGetProcAddress)  (const char *a) = NULL;
void     (*glsym_glXBindTexImage)    (Display *a, GLXDrawable b, int c, int *d) = NULL;
void     (*glsym_glXReleaseTexImage) (Display *a, GLXDrawable b, int c) = NULL;
int      (*glsym_glXGetVideoSync)    (unsigned int *a) = NULL;
int      (*glsym_glXWaitVideoSync)   (int a, int b, unsigned int *c) = NULL;
XID      (*glsym_glXCreatePixmap)    (Display *a, void *b, Pixmap c, const int *d) = NULL;
void     (*glsym_glXDestroyPixmap)   (Display *a, XID b) = NULL;
void     (*glsym_glXQueryDrawable)   (Display *a, XID b, int c, unsigned int *d) = NULL;
int      (*glsym_glXSwapIntervalSGI) (int a) = NULL;
void     (*glsym_glXSwapIntervalEXT) (Display *s, GLXDrawable b, int c) = NULL;
void     (*glsym_glXReleaseBuffersMESA)   (Display *a, XID b) = NULL;

#endif

#ifdef TIMDBG
static double
gettime(void)
{
   struct timeval      timev;
   
   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}

static void
measure(int end, const char *name)
{
   FILE *fs; 
   static unsigned long user = 0, kern = 0, user2 = 0, kern2 = 0;
   static double t = 0.0, t2 = 0.0;
   unsigned long u = 0, k = 0;
   
   fs = fopen("/proc/self/stat", "rb");
   if (fs) {
      fscanf(fs, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s "
             "%lu %lu %*s", &u, &k);
      fclose(fs);
   }
   if (end)
     {
        long hz;
        
        t2 = gettime();
        user2 = u;
        kern2 = k;
        hz = sysconf(_SC_CLK_TCK);
        fprintf(stderr, "(%8lu %8lu) k=%4lu u=%4lu == tot=%4lu@%4li in=%3.5f < %s\n", 
                user, kern, kern2 - kern, user2 - user, 
                (kern2 - kern) + (user2 - user), hz, t2 - t, name);
     }
   else
     {
        user = u;
        kern = k;
        t = gettime();
     }
}
#endif

static inline Outbuf *
eng_get_ob(Render_Engine *re)
{
   return re->generic.software.ob;
}

//----------------------------------------------------------//
// NEW_EVAS_GL Engine Functions
static void *
evgl_eng_display_get(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, NULL); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return NULL;
     }

#ifdef GL_GLES
   if (eng_get_ob(re))
      return (void*)eng_get_ob(re)->egl_disp;
#else
   if (eng_get_ob(re)->info)
      return (void*)eng_get_ob(re)->info->info.display;
#endif
   else
      return NULL;
}

static void *
evgl_eng_evas_surface_get(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, NULL); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return NULL;
     }

#ifdef GL_GLES
   if (eng_get_ob(re))
      return (void*)eng_get_ob(re)->egl_surface[0];
#else
   if (eng_get_ob(re))
      return (void*)eng_get_ob(re)->win;
#endif
   else
      return NULL;
}

static int
evgl_eng_make_current(void *data, void *surface, void *context, int flush)
{
   Render_Engine *re = (Render_Engine *)data;
   int ret = 0;

   /* EVGLINIT(re, 0); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return 0;
     }


#ifdef GL_GLES
   EGLContext ctx = (EGLContext)context;
   EGLSurface sfc = (EGLSurface)surface;
   EGLDisplay dpy = eng_get_ob(re)->egl_disp; //eglGetCurrentDisplay();

   if ((!context) && (!surface))
     {
        ret = eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (!ret)
          {
             ERR("eglMakeCurrent() failed! Error Code=%#x", eglGetError());
             return 0;
          }
        return 1;
     }

   if ((eglGetCurrentContext() != ctx) ||
       (eglGetCurrentSurface(EGL_READ) != sfc) ||
       (eglGetCurrentSurface(EGL_DRAW) != sfc) )
     {

        //!!!! Does it need to be flushed with it's set to NULL above??
        // Flush remainder of what's in Evas' pipeline
        if (flush) eng_window_use(NULL);

        // Do a make current
        ret = eglMakeCurrent(dpy, sfc, sfc, ctx);

        if (!ret)
          {
             ERR("eglMakeCurrent() failed! Error Code=%#x", eglGetError());
             return 0;
          }
     }

   return 1;
#else
   GLXContext ctx = (GLXContext)context;
   Window     sfc = (Window)surface;

   if ((!context) && (!surface))
     {
        ret = glXMakeCurrent(eng_get_ob(re)->info->info.display, None, NULL);
        if (!ret)
          {
             ERR("glXMakeCurrent() failed!");
             return 0;
          }
        return 1;
     }


   if ((glXGetCurrentContext() != ctx))
     {
        //!!!! Does it need to be flushed with it's set to NULL above??
        // Flush remainder of what's in Evas' pipeline
        if (flush) eng_window_use(NULL);

        // Do a make current
        ret = glXMakeCurrent(eng_get_ob(re)->info->info.display, sfc, ctx);

        if (!ret)
          {
             ERR("glXMakeCurrent() failed. Ret: %d! Context: %p Surface: %p", ret, (void*)ctx, (void*)sfc);
             return 0;
          }
     }
   return 1;
#endif
}



static void *
evgl_eng_native_window_create(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, NULL); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return NULL;
     }

   XSetWindowAttributes attr;
   Window win;

   attr.backing_store = NotUseful;
   attr.override_redirect = True;
   attr.border_pixel = 0;
   attr.background_pixmap = None;
   attr.bit_gravity = NorthWestGravity;
   attr.win_gravity = NorthWestGravity;
   attr.save_under = False;
   attr.do_not_propagate_mask = NoEventMask;
   attr.event_mask = 0; 

   win = XCreateWindow(eng_get_ob(re)->info->info.display,
                       DefaultRootWindow(eng_get_ob(re)->info->info.display),
                       0, 0, 2, 2, 0,
                       CopyFromParent, InputOutput, CopyFromParent, 
                       CWBackingStore | CWOverrideRedirect |
                       CWBorderPixel | CWBackPixmap |
                       CWSaveUnder | CWDontPropagate |
                       CWEventMask | CWBitGravity |
                       CWWinGravity, &attr);
   if (!win)
     {
        ERR("Creating native X window failed.");
        return NULL;
     }

   return (void*)win;
}

static int
evgl_eng_native_window_destroy(void *data, void *native_window)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, 0); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return 0;
     }

   if (!native_window)
     {
        ERR("Inavlid native surface.");
        return 0;
     }

   XDestroyWindow(eng_get_ob(re)->info->info.display, (Window)native_window);

   native_window = NULL;

   return 1;
}


// Theoretically, we wouldn't need this functoin if the surfaceless context
// is supported. But, until then... 
static void *
evgl_eng_window_surface_create(void *data, void *native_window EINA_UNUSED)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, NULL); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return NULL;
     }

#ifdef GL_GLES
   EGLSurface surface = EGL_NO_SURFACE;

   // Create resource surface for EGL
   surface = eglCreateWindowSurface(eng_get_ob(re)->egl_disp,
                                    eng_get_ob(re)->egl_config,
                                    (EGLNativeWindowType)native_window,
                                    NULL);
   if (!surface)
     {
        ERR("Creating window surface failed. Error: %#x.", eglGetError());
        return NULL;
     }

   return (void*)surface;
#else
   /*
   // We don't need to create new one for GLX
   Window surface;

   surface = eng_get_ob(re)->win;

   return (void *)surface;
   */
   return (void *)native_window;
#endif
}

static int
evgl_eng_window_surface_destroy(void *data, void *surface)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, 0); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return 0;
     }

#ifdef GL_GLES
   if (!surface)
     {
        ERR("Invalid surface.");
        return 0;
     }

   eglDestroySurface(eng_get_ob(re)->egl_disp, (EGLSurface)surface);
#endif

   return 1;
   if (surface) return 0;
}

static void *
evgl_eng_context_create(void *data, void *share_ctx)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, NULL); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return NULL;
     }

#ifdef GL_GLES
   EGLContext context = EGL_NO_CONTEXT;
   int context_attrs[3];

   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = 2;
   context_attrs[2] = EGL_NONE;

   // Share context already assumes that it's sharing with evas' context
   if (share_ctx)
     {
        context = eglCreateContext(eng_get_ob(re)->egl_disp,
                                   eng_get_ob(re)->egl_config,
                                   (EGLContext)share_ctx,
                                   context_attrs);
     }
   else
     {
        context = eglCreateContext(eng_get_ob(re)->egl_disp,
                                   eng_get_ob(re)->egl_config,
                                   eng_get_ob(re)->egl_context[0], // Evas' GL Context
                                   context_attrs);
     }

   if (!context)
     {
        ERR("Engine Context Creations Failed. Error: %#x.", eglGetError());
        return NULL;
     }

   return (void*)context;
#else
   GLXContext context = NULL;

   // Share context already assumes that it's sharing with evas' context
   if (share_ctx)
     {
        context = glXCreateContext(eng_get_ob(re)->info->info.display,
                                   eng_get_ob(re)->visualinfo,
                                   (GLXContext)share_ctx,
                                   1);
     }
   else
     {
        context = glXCreateContext(eng_get_ob(re)->info->info.display,
                                   eng_get_ob(re)->visualinfo,
                                   eng_get_ob(re)->context,      // Evas' GL Context
                                   1);
     }

   if (!context)
     {
        ERR("Internal Resource Context Creations Failed.");
        return NULL;
     }

   return (void*)context;
#endif

}

static int
evgl_eng_context_destroy(void *data, void *context)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, 0); */
   if ((!re) || (!context))
     {
        ERR("Invalid Render Input Data. Engine: %p, Context: %p", data, context);
        return 0;
     }

#ifdef GL_GLES
   eglDestroyContext(eng_get_ob(re)->egl_disp, (EGLContext)context);
#else
   glXDestroyContext(eng_get_ob(re)->info->info.display, (GLXContext)context);
#endif

   return 1;
}

static const char *
evgl_eng_string_get(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, NULL); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return NULL;
     }

#ifdef GL_GLES
   return eglQueryString(eng_get_ob(re)->egl_disp, EGL_EXTENSIONS);
#else
   return glXQueryExtensionsString(eng_get_ob(re)->info->info.display,
                                   eng_get_ob(re)->info->info.screen);
#endif
}

static void *
evgl_eng_proc_address_get(const char *name)
{
#ifdef GL_GLES
   if (glsym_eglGetProcAddress) return glsym_eglGetProcAddress(name);
   return dlsym(RTLD_DEFAULT, name);
#else
   if (glsym_glXGetProcAddress) return glsym_glXGetProcAddress(name);
   return dlsym(RTLD_DEFAULT, name);
#endif
}

static int
evgl_eng_rotation_angle_get(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, 0); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return 0;
     }

   if ((eng_get_ob(re)) && (eng_get_ob(re)->gl_context))
      return eng_get_ob(re)->gl_context->rot;
   else
     {
        ERR("Unable to retrieve rotation angle.");
        return 0;
     }
}

static const EVGL_Interface evgl_funcs =
{
   evgl_eng_display_get,
   evgl_eng_evas_surface_get,
   evgl_eng_native_window_create,
   evgl_eng_native_window_destroy,
   evgl_eng_window_surface_create,
   evgl_eng_window_surface_destroy,
   evgl_eng_context_create,
   evgl_eng_context_destroy,
   evgl_eng_make_current,
   evgl_eng_proc_address_get,
   evgl_eng_string_get,
   evgl_eng_rotation_angle_get
};

//----------------------------------------------------------//


static void
gl_symbols(void)
{
   static int done = 0;

   if (done) return;

#define LINK2GENERIC(sym) \
   glsym_##sym = dlsym(RTLD_DEFAULT, #sym);

   // Get function pointer to evas_gl_common that is now provided through the link of GL_Generic.
   LINK2GENERIC(evas_gl_common_image_all_unload);
   LINK2GENERIC(evas_gl_common_image_ref);
   LINK2GENERIC(evas_gl_common_image_unref);
   LINK2GENERIC(evas_gl_common_image_new_from_data);
   LINK2GENERIC(evas_gl_common_image_native_disable);
   LINK2GENERIC(evas_gl_common_image_free);
   LINK2GENERIC(evas_gl_common_image_native_enable);
   LINK2GENERIC(evas_gl_common_context_new);
   LINK2GENERIC(evas_gl_common_context_flush);
   LINK2GENERIC(evas_gl_common_context_free);
   LINK2GENERIC(evas_gl_common_context_use);
   LINK2GENERIC(evas_gl_common_context_newframe);
   LINK2GENERIC(evas_gl_common_context_done);
   LINK2GENERIC(evas_gl_common_context_resize);
   LINK2GENERIC(evas_gl_common_buffer_dump);
   LINK2GENERIC(evas_gl_preload_render_lock);
   LINK2GENERIC(evas_gl_preload_render_unlock);
   LINK2GENERIC(evas_gl_preload_render_relax);
   LINK2GENERIC(evas_gl_preload_init);
   LINK2GENERIC(evas_gl_preload_shutdown);
   LINK2GENERIC(evgl_engine_shutdown);
   LINK2GENERIC(evas_gl_symbols);

#ifdef GL_GLES
#define FINDSYM(dst, sym, typ) \
   if (glsym_eglGetProcAddress) { \
      if (!dst) dst = (typ)glsym_eglGetProcAddress(sym); \
   } else { \
      if (!dst) dst = (typ)dlsym(RTLD_DEFAULT, sym); \
   }

   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressKHR", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressEXT", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressARB", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddress", glsym_func_eng_fn);

   glsym_evas_gl_symbols((void*)glsym_eglGetProcAddress);

   FINDSYM(glsym_eglCreateImage, "eglCreateImageKHR", glsym_func_void_ptr);
   FINDSYM(glsym_eglCreateImage, "eglCreateImageEXT", glsym_func_void_ptr);
   FINDSYM(glsym_eglCreateImage, "eglCreateImageARB", glsym_func_void_ptr);
   FINDSYM(glsym_eglCreateImage, "eglCreateImage", glsym_func_void_ptr);

   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageKHR", glsym_func_void);
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageEXT", glsym_func_void);
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageARB", glsym_func_void);
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImage", glsym_func_void);

   FINDSYM(glsym_glEGLImageTargetTexture2DOES, "glEGLImageTargetTexture2DOES", glsym_func_void);

   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamageEXT", glsym_func_uint);
   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamageINTEL", glsym_func_uint);
   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamage", glsym_func_uint);


#else
#define FINDSYM(dst, sym, typ) \
   if (glsym_glXGetProcAddress) { \
      if (!dst) dst = (typ)glsym_glXGetProcAddress(sym); \
   } else { \
      if (!dst) dst = (typ)dlsym(RTLD_DEFAULT, sym); \
   }

   FINDSYM(glsym_glXGetProcAddress, "glXGetProcAddressEXT", glsym_func_eng_fn);
   FINDSYM(glsym_glXGetProcAddress, "glXGetProcAddressARB", glsym_func_eng_fn);
   FINDSYM(glsym_glXGetProcAddress, "glXGetProcAddress", glsym_func_eng_fn);

   glsym_evas_gl_symbols((void*)glsym_glXGetProcAddress);

   FINDSYM(glsym_glXBindTexImage, "glXBindTexImageEXT", glsym_func_void);
   FINDSYM(glsym_glXBindTexImage, "glXBindTexImageARB", glsym_func_void);
   FINDSYM(glsym_glXBindTexImage, "glXBindTexImage", glsym_func_void);

   FINDSYM(glsym_glXReleaseTexImage, "glXReleaseTexImageEXT", glsym_func_void);
   FINDSYM(glsym_glXReleaseTexImage, "glXReleaseTexImageARB", glsym_func_void);
   FINDSYM(glsym_glXReleaseTexImage, "glXReleaseTexImage", glsym_func_void);

   FINDSYM(glsym_glXGetVideoSync, "glXGetVideoSyncSGI", glsym_func_int);

   FINDSYM(glsym_glXWaitVideoSync, "glXWaitVideoSyncSGI", glsym_func_int);

   FINDSYM(glsym_glXCreatePixmap, "glXCreatePixmapEXT", glsym_func_xid);
   FINDSYM(glsym_glXCreatePixmap, "glXCreatePixmapARB", glsym_func_xid);
   FINDSYM(glsym_glXCreatePixmap, "glXCreatePixmap", glsym_func_xid);

   FINDSYM(glsym_glXDestroyPixmap, "glXDestroyPixmapEXT", glsym_func_void);
   FINDSYM(glsym_glXDestroyPixmap, "glXDestroyPixmapARB", glsym_func_void);
   FINDSYM(glsym_glXDestroyPixmap, "glXDestroyPixmap", glsym_func_void);

   FINDSYM(glsym_glXQueryDrawable, "glXQueryDrawableEXT", glsym_func_void);
   FINDSYM(glsym_glXQueryDrawable, "glXQueryDrawableARB", glsym_func_void);
   FINDSYM(glsym_glXQueryDrawable, "glXQueryDrawable", glsym_func_void);

   FINDSYM(glsym_glXSwapIntervalSGI, "glXSwapIntervalMESA", glsym_func_int);
   FINDSYM(glsym_glXSwapIntervalSGI, "glXSwapIntervalSGI", glsym_func_int);

   FINDSYM(glsym_glXSwapIntervalEXT, "glXSwapIntervalEXT", glsym_func_void);

   FINDSYM(glsym_glXReleaseBuffersMESA, "glXReleaseBuffersMESA", glsym_func_void);

#endif

   done = 1;
}

static void
gl_extn_veto(Render_Engine *re)
{
   const char *str = NULL;
#ifdef GL_GLES
   str = eglQueryString(eng_get_ob(re)->egl_disp, EGL_EXTENSIONS);
   if (str)
     {
        if (getenv("EVAS_GL_INFO"))
          printf("EGL EXTN:\n%s\n", str);
        if (!strstr(str, "EGL_EXT_buffer_age"))
          {
             extn_have_buffer_age = 0;
          }
        if (!strstr(str, "EGL_NOK_texture_from_pixmap"))
          {
             extn_have_y_inverted = 0;
          }
        else
          {
             const GLubyte *vendor, *renderer;

             vendor = glGetString(GL_VENDOR);
             renderer = glGetString(GL_RENDERER);
             // XXX: workaround mesa bug!
             // looking for mesa and intel build which is known to
             // advertise the EGL_NOK_texture_from_pixmap extension
             // but not set it correctly. guessing vendor/renderer
             // strings will be like the following:
             // OpenGL vendor string: Intel Open Source Technology Center
             // OpenGL renderer string: Mesa DRI Intel(R) Sandybridge Desktop
             if (((vendor) && (strstr((const char *)vendor, "Intel"))) &&
                 ((renderer) && (strstr((const char *)renderer, "Mesa"))) &&
                 ((renderer) && (strstr((const char *)renderer, "Intel")))
                )
               extn_have_y_inverted = 0;
          }
        if (!strstr(str, "EGL_EXT_swap_buffers_with_damage"))
          {
             glsym_eglSwapBuffersWithDamage = NULL;
          }
     }
   else
     {
        if (getenv("EVAS_GL_INFO"))
          printf("NO EGL EXTN!\n");
        extn_have_buffer_age = 0;
     }
#else
   str = glXQueryExtensionsString(eng_get_ob(re)->info->info.display,
                                  eng_get_ob(re)->info->info.screen);
   if (str)
     {
        if (getenv("EVAS_GL_INFO"))
          printf("GLX EXTN:\n%s\n", str);
        if (!strstr(str, "_texture_from_pixmap"))
          {
             glsym_glXBindTexImage = NULL;
             glsym_glXReleaseTexImage = NULL;
          }
        if (!strstr(str, "GLX_SGI_video_sync"))
          {
             glsym_glXGetVideoSync = NULL;
             glsym_glXWaitVideoSync = NULL;
          }
        if (!strstr(str, "GLX_EXT_buffer_age"))
          {
             extn_have_buffer_age = 0;
          }
        if (!strstr(str, "GLX_EXT_swap_control"))
          {
             glsym_glXSwapIntervalEXT = NULL;
          }
        if (!strstr(str, "GLX_SGI_swap_control"))
          {
             glsym_glXSwapIntervalSGI = NULL;
          }
        if (!strstr(str, "GLX_MESA_release_buffers"))
          {
             glsym_glXReleaseBuffersMESA = NULL;
          }
     }
   else
     {
        if (getenv("EVAS_GL_INFO"))
          printf("NO GLX EXTN!\n");
        glsym_glXBindTexImage = NULL;
        glsym_glXReleaseTexImage = NULL;
        glsym_glXGetVideoSync = NULL;
        glsym_glXWaitVideoSync = NULL;
        extn_have_buffer_age = 0;
        glsym_glXSwapIntervalEXT = NULL;
        glsym_glXSwapIntervalSGI = NULL;
        glsym_glXReleaseBuffersMESA = NULL;
     }
#endif
}

int _evas_engine_GL_X11_log_dom = -1;
/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

static void *
eng_info(Evas *eo_e EINA_UNUSED)
{
   Evas_Engine_Info_GL_X11 *info;

   info = calloc(1, sizeof(Evas_Engine_Info_GL_X11));
   info->magic.magic = rand();
   info->func.best_visual_get = eng_best_visual_get;
   info->func.best_colormap_get = eng_best_colormap_get;
   info->func.best_depth_get = eng_best_depth_get;
   info->render_mode = EVAS_RENDER_MODE_BLOCKING;
   return info;
}

static void
eng_info_free(Evas *eo_e EINA_UNUSED, void *info)
{
   Evas_Engine_Info_GL_X11 *in;
// dont free! why bother? its not worth it
//   eina_log_domain_unregister(_evas_engine_GL_X11_log_dom);
   in = (Evas_Engine_Info_GL_X11 *)info;
   free(in);
}

static void
_re_winfree(Render_Engine *re)
{
   if (!eng_get_ob(re)->surf) return;
   glsym_evas_gl_preload_render_relax(eng_preload_make_current, eng_get_ob(re));
   eng_window_unsurf(eng_get_ob(re));
}

static int
eng_setup(Evas *eo_e, void *in)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Render_Engine *re;
   Evas_Engine_Info_GL_X11 *info;
   Render_Engine_Swap_Mode swap_mode = MODE_FULL;
   const char *s;

   info = (Evas_Engine_Info_GL_X11 *)in;

   if ((s = getenv("EVAS_GL_SWAP_MODE")))
     {
        if ((!strcasecmp(s, "full")) ||
            (!strcasecmp(s, "f")))
          swap_mode = MODE_FULL;
        else if ((!strcasecmp(s, "copy")) ||
                 (!strcasecmp(s, "c")))
          swap_mode = MODE_COPY;
        else if ((!strcasecmp(s, "double")) ||
                 (!strcasecmp(s, "d")) ||
                 (!strcasecmp(s, "2")))
          swap_mode = MODE_DOUBLE;
        else if ((!strcasecmp(s, "triple")) ||
                 (!strcasecmp(s, "t")) ||
                 (!strcasecmp(s, "3")))
          swap_mode = MODE_TRIPLE;
        else if ((!strcasecmp(s, "quadruple")) ||
                 (!strcasecmp(s, "q")) ||
                 (!strcasecmp(s, "4")))
          swap_mode = MODE_QUADRUPLE;
     }
   else
     {
// in most gl implementations - egl and glx here that we care about the TEND
// to either swap or copy backbuffer and front buffer, but strictly that is
// not true. technically backbuffer content is totally undefined after a swap
// and thus you MUST re-render all of it, thus MODE_FULL
        swap_mode = MODE_FULL;
// BUT... reality is that lmost every implementation copies or swaps so
// triple buffer mode can be used as it is a superset of double buffer and
// copy (though using those explicitly is more efficient). so let's play with
// triple buffer mdoe as a default and see.
//        re->mode = MODE_TRIPLE;
// XXX: note - the above seems to break on some older intel chipsets and
// drivers. it seems we CANT depend on backbuffer staying around. bugger!
        switch (info->swap_mode)
          {
           case EVAS_ENGINE_GL_X11_SWAP_MODE_FULL:
             swap_mode = MODE_FULL;
             break;
           case EVAS_ENGINE_GL_X11_SWAP_MODE_COPY:
             swap_mode = MODE_COPY;
             break;
           case EVAS_ENGINE_GL_X11_SWAP_MODE_DOUBLE:
             swap_mode = MODE_DOUBLE;
             break;
           case EVAS_ENGINE_GL_X11_SWAP_MODE_TRIPLE:
             swap_mode = MODE_TRIPLE;
             break;
           case EVAS_ENGINE_GL_X11_SWAP_MODE_QUADRUPLE:
             swap_mode = MODE_QUADRUPLE;
             break;
           default:
             swap_mode = MODE_AUTO;
             break;
          }
     }

   if (safe_native == -1)
     {
        s = getenv("EVAS_GL_SAFE_NATIVE");
        safe_native = 0;
        if (s) safe_native = atoi(s);
        else
          {
             s = (const char *)glGetString(GL_RENDERER);
             if (s)
               {
                  if (strstr(s, "PowerVR SGX 540") ||
                      strstr(s, "Mali-400 MP"))
                    safe_native = 1;
               }
          }
     }

   // Set this env var to dump files every frame
   // Or set the global var in gdb to 1|0 to turn it on and off
   if (getenv("EVAS_GL_SWAP_BUFFER_DEBUG_ALWAYS"))
     swap_buffer_debug = 1;

   if (swap_buffer_debug_mode == -1)
     {
        if (
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
            (getuid() == geteuid()) &&
#endif
            ((debug_dir = getenv("EVAS_GL_SWAP_BUFFER_DEBUG_DIR"))))
          {
             int stat;
             // Create a directory with 0775 permission
             stat = mkdir(debug_dir, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
             if ((!stat) || errno == EEXIST) swap_buffer_debug_mode = 1;
          }
        else
           swap_buffer_debug_mode = 0;
     }


   if (!e->engine.data.output)
     {
        Outbuf *ob;
        Render_Engine_Merge_Mode merge_mode = MERGE_BOUNDING;

        if (!initted)
          {
             evas_common_cpu_init();
             evas_common_blend_init();
             evas_common_image_init();
             evas_common_convert_init();
             evas_common_scale_init();
             evas_common_rectangle_init();
             evas_common_polygon_init();
             evas_common_line_init();
             evas_common_font_init();
             evas_common_draw_init();
             evas_common_tilebuf_init();
             glsym_evas_gl_preload_init();
          }

#ifdef GL_GLES
#else
        int eb, evb;

        if (!glXQueryExtension(info->info.display, &eb, &evb)) return 0;
#endif
        re = calloc(1, sizeof(Render_Engine));
        if (!re) return 0;
        ob = eng_window_new(info, eo_e,
                            info->info.display,
                            info->info.drawable,
                            info->info.screen,
                            info->info.visual,
                            info->info.colormap,
                            info->info.depth,
                            e->output.w, e->output.h,
                            info->indirect,
                            info->info.destination_alpha,
                            info->info.rotation,
                            swap_mode);
        if (!ob)
          {
             free(re);
             return 0;
          }

        if (!evas_render_engine_gl_generic_init(&re->generic, ob,
                                                eng_outbuf_swap_mode,
                                                eng_outbuf_get_rot,
                                                eng_outbuf_reconfigure,
                                                eng_outbuf_region_first_rect,
                                                eng_outbuf_new_region_for_update,
                                                eng_outbuf_push_updated_region,
                                                eng_outbuf_push_free_region_for_update,
                                                NULL,
                                                eng_outbuf_flush,
                                                eng_window_free,
                                                eng_window_use,
                                                eng_outbuf_gl_context_get,
                                                eng_outbuf_egl_display_get,
                                                eng_gl_context_new,
                                                eng_gl_context_use,
                                                &evgl_funcs,
                                                e->output.w, e->output.h))
          {
             eng_window_free(ob);
             free(re);
             return 0;
          }

        e->engine.data.output = re;
        gl_wins++;

        if ((s = getenv("EVAS_GL_PARTIAL_MERGE")))
          {
             if ((!strcmp(s, "bounding")) ||
                 (!strcmp(s, "b")))
               merge_mode = MERGE_BOUNDING;
             else if ((!strcmp(s, "full")) ||
                      (!strcmp(s, "f")))
               merge_mode = MERGE_FULL;
          }

        evas_render_engine_software_generic_merge_mode_set(&re->generic.software, merge_mode);

        if (!initted)
          {
             gl_extn_veto(re);
//             evgl_engine_init(re, &evgl_funcs);
             initted = 1;
          }

     }
   else
     {
        re = e->engine.data.output;
        if (eng_get_ob(re) && _re_wincheck(eng_get_ob(re)))
          {
             if ((eng_get_ob(re)->info->info.display != eng_get_ob(re)->disp) ||
                 (eng_get_ob(re)->info->info.drawable != eng_get_ob(re)->win) ||
                 (eng_get_ob(re)->info->info.screen != eng_get_ob(re)->screen) ||
                 (eng_get_ob(re)->info->info.visual != eng_get_ob(re)->visual) ||
                 (eng_get_ob(re)->info->info.colormap != eng_get_ob(re)->colormap) ||
                 (eng_get_ob(re)->info->info.depth != eng_get_ob(re)->depth) ||
                 (eng_get_ob(re)->info->info.destination_alpha != eng_get_ob(re)->alpha))
               {
                  Outbuf *ob;

                  eng_get_ob(re)->gl_context->references++;
                  gl_wins--;

                  ob = eng_window_new(info, eo_e,
                                      eng_get_ob(re)->info->info.display,
                                      eng_get_ob(re)->info->info.drawable,
                                      eng_get_ob(re)->info->info.screen,
                                      eng_get_ob(re)->info->info.visual,
                                      eng_get_ob(re)->info->info.colormap,
                                      eng_get_ob(re)->info->info.depth,
                                      e->output.w, e->output.h,
                                      eng_get_ob(re)->info->indirect,
                                      eng_get_ob(re)->info->info.destination_alpha,
                                      eng_get_ob(re)->info->info.rotation,
                                      swap_mode);

                  eng_window_free(eng_get_ob(re));
                  re->generic.software.ob = NULL;

                  eng_window_use(ob);
                  if (ob)
                    {
                       evas_render_engine_software_generic_update(&re->generic.software, ob,
                                                                  e->output.w, e->output.h);

                       gl_wins++;
                       eng_get_ob(re)->gl_context->references--;
                    }
               }
             else if ((eng_get_ob(re)->w != e->output.w) ||
                      (eng_get_ob(re)->h != e->output.h) ||
                      (eng_get_ob(re)->info->info.rotation != eng_get_ob(re)->rot))
               {
                  eng_outbuf_reconfigure(eng_get_ob(re), e->output.w, e->output.h, eng_get_ob(re)->info->info.rotation, 0);
                  if (re->generic.software.tb)
                    evas_common_tilebuf_free(re->generic.software.tb);
                  re->generic.software.tb = evas_common_tilebuf_new(e->output.w, e->output.h);
                  if (re->generic.software.tb)
                    evas_common_tilebuf_set_tile_size(re->generic.software.tb,
                                                      TILESIZE, TILESIZE);
               }
          }
     }
   if (!eng_get_ob(re))
     {
        free(re);
        return 0;
     }

   if (!e->engine.data.output)
     {
        if (eng_get_ob(re))
          {
             eng_window_free(eng_get_ob(re));
             gl_wins--;
          }
        free(re);
        return 0;
     }

   if (re->generic.software.tb)
     evas_render_engine_software_generic_tile_strict_set
       (&re->generic.software, EINA_TRUE);

   if (!e->engine.data.context)
     e->engine.data.context =
     e->engine.func->context_new(e->engine.data.output);
   eng_window_use(eng_get_ob(re));

   return 1;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;

   if (re)
     {
#ifndef GL_GLES
        Display *disp = eng_get_ob(re)->disp;
        Window win = eng_get_ob(re)->win;
#endif

        glsym_evas_gl_preload_render_relax(eng_preload_make_current, eng_get_ob(re));

#if 0
#ifdef GL_GLES
        // Destroy the resource surface
        // Only required for EGL case
        if (re->surface)
           eglDestroySurface(eng_get_ob(re)->egl_disp, re->surface);
#endif

        // Destroy the resource context
        _destroy_internal_context(re, context);
#endif

        if (gl_wins == 1) glsym_evgl_engine_shutdown(re);

        evas_render_engine_software_generic_clean(&re->generic.software);

#ifndef GL_GLES
        if (glsym_glXReleaseBuffersMESA)
          glsym_glXReleaseBuffersMESA(disp, win);
#endif
        gl_wins--;

        free(re);
     }
   if ((initted == 1) && (gl_wins == 0))
     {
        glsym_evas_gl_preload_shutdown();
        evas_common_image_shutdown();
        evas_common_font_shutdown();
        initted = 0;
     }
}

/* vsync games - not for now though */
#define VSYNC_TO_SCREEN 1

Eina_Bool
eng_preload_make_current(void *data, void *doit)
{
   Outbuf *ob = data;

   if (doit)
     {
#ifdef GL_GLES
        if (!eglMakeCurrent(ob->egl_disp, ob->egl_surface[0], ob->egl_surface[0], ob->egl_context[0]))
          return EINA_FALSE;
#else
        if (!glXMakeCurrent(ob->info->info.display, ob->win, ob->context))
          {
             ERR("glXMakeCurrent(%p, 0x%x, %p) failed", ob->info->info.display, (unsigned int)ob->win, (void *)ob->context);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             return EINA_FALSE;
          }
#endif
     }
   else
     {
#ifdef GL_GLES
        if (!eglMakeCurrent(ob->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
          return EINA_FALSE;
#else
        if (!glXMakeCurrent(ob->info->info.display, None, NULL))
          {
             ERR("glXMakeCurrent(%p, None, NULL) failed", ob->info->info.display);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             return EINA_FALSE;
          }
#endif
     }
   return EINA_TRUE;
}

static Eina_Bool
eng_canvas_alpha_get(void *data, void *info EINA_UNUSED)
{
   Render_Engine *re = (Render_Engine *)data;
   return re->generic.software.ob->alpha;
}

static void
eng_output_dump(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_image_image_all_unload();
   evas_common_font_font_all_unload();
   glsym_evas_gl_common_image_all_unload(eng_get_ob(re)->gl_context);
   _re_winfree(re);
}

/////////////////////////////////////////////////////////////////////////
//
//
typedef struct _Native Native;

struct _Native
{
   Evas_Native_Surface ns;
   Pixmap     pixmap;
   Visual    *visual;

#ifdef GL_GLES
   void      *egl_surface;
#else
   void  *fbc;
   XID    glx_pixmap;
#endif
};

// FIXME: this is enabled so updates happen - but its SLOOOOOOOOOOOOOOOW
// (i am sure this is the reason)  not to mention seemingly superfluous. but
// i need to enable it for it to work on fglrx at least. havent tried nvidia.
//
// why is this the case? does anyone know? has anyone tried it on other gfx
// drivers?
//
//#define GLX_TEX_PIXMAP_RECREATE 1

static void
_native_bind_cb(void *data, void *image)
{
   Evas_GL_Image *im = image;
   Native *n = im->native.data;

  if (n->ns.type == EVAS_NATIVE_SURFACE_X11)
    {
#ifdef GL_GLES
      if (n->egl_surface)
        {
          if (glsym_glEGLImageTargetTexture2DOES)
            {
              glsym_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, n->egl_surface);
              if (eglGetError() != EGL_SUCCESS)
                ERR("glEGLImageTargetTexture2DOES() failed.");
            }
          else
            ERR("Try glEGLImageTargetTexture2DOES on EGL with no support");
        }
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT
      Render_Engine *re = data;

      if (glsym_glXBindTexImage)
        {
          glsym_glXBindTexImage(eng_get_ob(re)->disp, n->glx_pixmap,
                                GLX_FRONT_LEFT_EXT, NULL);
          GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        }
      else
        ERR("Try glXBindTexImage on GLX with no support");
# endif
#endif
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
    {
      glBindTexture(GL_TEXTURE_2D, n->ns.data.opengl.texture_id);
      GLERR(__FUNCTION__, __FILE__, __LINE__, "");
    }
   return;
   (void *) (data = NULL);
}

static void
_native_unbind_cb(void *data, void *image)
{
  Evas_GL_Image *im = image;
  Native *n = im->native.data;

  if (n->ns.type == EVAS_NATIVE_SURFACE_X11)
    {
#ifdef GL_GLES
      // nothing
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT
      Render_Engine *re = data;

      if (glsym_glXReleaseTexImage)
        {
          glsym_glXReleaseTexImage(eng_get_ob(re)->disp, n->glx_pixmap,
                                   GLX_FRONT_LEFT_EXT);
          GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        }
      else
        ERR("Try glXReleaseTexImage on GLX with no support");
# endif
#endif
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
    {
      glBindTexture(GL_TEXTURE_2D, 0);
      GLERR(__FUNCTION__, __FILE__, __LINE__, "");
    }
   return;
   (void *) (data = NULL);
}

static void
_native_free_cb(void *data, void *image)
{
  Render_Engine *re = data;
  Evas_GL_Image *im = image;
  Native *n = im->native.data;
  uint32_t pmid, texid;

  if (n->ns.type == EVAS_NATIVE_SURFACE_X11)
    {
      pmid = n->pixmap;
      eina_hash_del(eng_get_ob(re)->gl_context->shared->native_pm_hash, &pmid, im);
#ifdef GL_GLES
      if (n->egl_surface)
        {
          if (glsym_eglDestroyImage)
            {
              glsym_eglDestroyImage(eng_get_ob(re)->egl_disp,
                                    n->egl_surface);
              if (eglGetError() != EGL_SUCCESS)
                ERR("eglDestroyImage() failed.");
            }
          else
            ERR("Try eglDestroyImage on EGL with no support");
        }
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT
      if (n->glx_pixmap)
        {
          if (im->native.loose)
            {
              if (glsym_glXReleaseTexImage)
                {
                  glsym_glXReleaseTexImage(eng_get_ob(re)->disp, n->glx_pixmap,
                                           GLX_FRONT_LEFT_EXT);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                }
              else
                ERR("Try glXReleaseTexImage on GLX with no support");
            }
          if (glsym_glXDestroyPixmap)
            {
              glsym_glXDestroyPixmap(eng_get_ob(re)->disp, n->glx_pixmap);
              GLERR(__FUNCTION__, __FILE__, __LINE__, "");
            }
          else
            ERR("Try glXDestroyPixmap on GLX with no support");
          n->glx_pixmap = 0;
        }
# endif
#endif
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
    {
      texid = n->ns.data.opengl.texture_id;
      eina_hash_del(eng_get_ob(re)->gl_context->shared->native_tex_hash, &texid, im);
    }
  im->native.data        = NULL;
  im->native.func.data   = NULL;
  im->native.func.bind   = NULL;
  im->native.func.unbind = NULL;
  im->native.func.free   = NULL;
  free(n);
}

static void *
eng_image_native_set(void *data, void *image, void *native)
{
  Render_Engine *re = (Render_Engine *)data;
  Evas_Native_Surface *ns = native;
  Evas_GL_Image *im = image, *im2 = NULL;
  Visual *vis = NULL;
  Pixmap pm = 0;
  Native *n = NULL;
  uint32_t pmid, texid;
  unsigned int tex = 0;
  unsigned int fbo = 0;

  if (!im)
    {
       if ((ns) && (ns->type == EVAS_NATIVE_SURFACE_OPENGL))
         {
            im = glsym_evas_gl_common_image_new_from_data(eng_get_ob(re)->gl_context,
                                                    ns->data.opengl.w,
                                                    ns->data.opengl.h,
                                                    NULL, 1,
                                                    EVAS_COLORSPACE_ARGB8888);
         }
       else
         return NULL;
    }

  if (ns)
    {
      if (ns->type == EVAS_NATIVE_SURFACE_X11)
        {
          vis = ns->data.x11.visual;
          pm = ns->data.x11.pixmap;
          if (im->native.data)
            {
              Evas_Native_Surface *ens = im->native.data;
              if ((ens->data.x11.visual == vis) &&
                  (ens->data.x11.pixmap == pm))
                return im;
            }
        }
      else if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
        {
          tex = ns->data.opengl.texture_id;
          fbo = ns->data.opengl.framebuffer_id;
          if (im->native.data)
            {
              Evas_Native_Surface *ens = im->native.data;
              if ((ens->data.opengl.texture_id == tex) &&
                  (ens->data.opengl.framebuffer_id == fbo))
                return im;
            }
        }
    }
  if ((!ns) && (!im->native.data)) return im;

  eng_window_use(eng_get_ob(re));

  if (im->native.data)
    {
      if (im->native.func.free)
        im->native.func.free(im->native.func.data, im);
      glsym_evas_gl_common_image_native_disable(im);
    }

  if (!ns) return im;

  if (ns->type == EVAS_NATIVE_SURFACE_X11)
    {
      pmid = pm;
      im2 = eina_hash_find(eng_get_ob(re)->gl_context->shared->native_pm_hash, &pmid);
      if (im2 == im) return im;
      if (im2)
        {
           n = im2->native.data;
           if (n)
             {
                glsym_evas_gl_common_image_ref(im2);
                glsym_evas_gl_common_image_free(im);
                return im2;
             }
        }
    }
  else if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
    {
       texid = tex;
       im2 = eina_hash_find(eng_get_ob(re)->gl_context->shared->native_tex_hash, &texid);
       if (im2 == im) return im;
       if (im2)
         {
            n = im2->native.data;
            if (n)
              {
                 glsym_evas_gl_common_image_ref(im2);
                 glsym_evas_gl_common_image_free(im);
                 return im2;
              }
         }

    }
  im2 = glsym_evas_gl_common_image_new_from_data(eng_get_ob(re)->gl_context,
                                           im->w, im->h, NULL, im->alpha,
                                           EVAS_COLORSPACE_ARGB8888);
  glsym_evas_gl_common_image_free(im);
  im = im2;
  if (!im) return NULL;
  if (ns->type == EVAS_NATIVE_SURFACE_X11)
    {
#ifdef GL_GLES
      if (native)
        {
          n = calloc(1, sizeof(Native));
          if (n)
            {
              EGLConfig egl_config;
              int config_attrs[20];
              int num_config, i = 0;
              int yinvert = 1;

              eina_hash_add(eng_get_ob(re)->gl_context->shared->native_pm_hash, &pmid, im);

              // assume 32bit pixmap! :)
              config_attrs[i++] = EGL_RED_SIZE;
              config_attrs[i++] = 8;
              config_attrs[i++] = EGL_GREEN_SIZE;
              config_attrs[i++] = 8;
              config_attrs[i++] = EGL_BLUE_SIZE;
              config_attrs[i++] = 8;
              config_attrs[i++] = EGL_ALPHA_SIZE;
              config_attrs[i++] = 8;
              config_attrs[i++] = EGL_DEPTH_SIZE;
              config_attrs[i++] = 0;
              config_attrs[i++] = EGL_STENCIL_SIZE;
              config_attrs[i++] = 0;
              config_attrs[i++] = EGL_RENDERABLE_TYPE;
              config_attrs[i++] = EGL_OPENGL_ES2_BIT;
              config_attrs[i++] = EGL_SURFACE_TYPE;
              config_attrs[i++] = EGL_PIXMAP_BIT;
              config_attrs[i++] = EGL_NONE;

              if (!eglChooseConfig(eng_get_ob(re)->egl_disp, config_attrs,
                                   &egl_config, 1, &num_config))
                ERR("eglChooseConfig() failed for pixmap 0x%x, num_config = %i", (unsigned int)pm, num_config);
              else
                {
                  int val;
                  if (extn_have_y_inverted &&
                      eglGetConfigAttrib(eng_get_ob(re)->egl_disp, egl_config,
                                         EGL_Y_INVERTED_NOK, &val))
                        yinvert = val;
                }

              memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
              n->pixmap = pm;
              n->visual = vis;
              if (glsym_eglCreateImage)
                n->egl_surface = glsym_eglCreateImage(eng_get_ob(re)->egl_disp,
                                                      EGL_NO_CONTEXT,
                                                      EGL_NATIVE_PIXMAP_KHR,
                                                      (void *)pm,
                                                      NULL);
              else
                ERR("Try eglCreateImage on EGL with no support");
              if (!n->egl_surface)
                ERR("eglCreatePixmapSurface() for 0x%x failed", (unsigned int)pm);
              im->native.yinvert     = yinvert;
              im->native.loose       = 0;
              im->native.data        = n;
              im->native.func.data   = re;
              im->native.func.bind   = _native_bind_cb;
              im->native.func.unbind = _native_unbind_cb;
              im->native.func.free   = _native_free_cb;
              im->native.target      = GL_TEXTURE_2D;
              im->native.mipmap      = 0;
              glsym_evas_gl_common_image_native_enable(im);
            }
        }
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT
       if (native)
         {
            int dummy;
            unsigned int w, h, depth = 32, border;
            Window wdummy;
            
            // fixme: round trip :(
            XGetGeometry(eng_get_ob(re)->disp, pm, &wdummy, &dummy, &dummy,
                         &w, &h, &border, &depth);
            if (depth <= 32)
              {
                 n = calloc(1, sizeof(Native));
                 if (n)
                   {
                      int pixmap_att[20], i;
                      int config_attrs[40], num = 0;
                      int tex_format = 0, tex_target = 0, yinvert = 0, mipmap = 0;
                      unsigned int target = 0;
                      GLXFBConfig *configs;
                      
                      i = 0;
                      config_attrs[i++] = GLX_BUFFER_SIZE;
                      config_attrs[i++] = depth;
                      if (depth == 32)
                        {
                           config_attrs[i++] = GLX_BIND_TO_TEXTURE_RGBA_EXT;
                           config_attrs[i++] = 1;
                        }
                      else
                        {
                           config_attrs[i++] = GLX_BIND_TO_TEXTURE_RGB_EXT;
                           config_attrs[i++] = 1;
                        }
                      
#ifndef GLX_VISUAL_ID
# define GLX_VISUAL_ID 0x800b
#endif
                      config_attrs[i++] = GLX_VISUAL_ID;
                      config_attrs[i++] = XVisualIDFromVisual(vis);
#ifndef GLX_SAMPLE_BUFFERS
# define GLX_SAMPLE_BUFFERS 0x186a0
#endif
                      config_attrs[i++] = GLX_SAMPLE_BUFFERS;
                      config_attrs[i++] = 0;
                      config_attrs[i++] = GLX_DEPTH_SIZE;
                      config_attrs[i++] = 0;
                      config_attrs[i++] = GLX_STENCIL_SIZE;
                      config_attrs[i++] = 0;
                      config_attrs[i++] = GLX_AUX_BUFFERS;
                      config_attrs[i++] = 0;
                      config_attrs[i++] = GLX_STEREO;
                      config_attrs[i++] = 0;
                      
                      config_attrs[i++] = 0;
                      
                      configs = glXChooseFBConfig(eng_get_ob(re)->disp,
                                                  eng_get_ob(re)->screen,
                                                  config_attrs,
                                                  &num);
                      if (configs)
                        {
                           int j = 0, val = 0, found = 0;
                           
                           try_again:
                           for (j = 0; j < num; j++)
                             {
                                if (found == 0)
                                  {
                                     XVisualInfo *vi;
                                     
                                     vi = glXGetVisualFromFBConfig(eng_get_ob(re)->disp, configs[j]);
                                     if (!vi) continue;
                                     if (vi->depth != (int)depth) continue;
                                     XFree(vi);
                                     
                                     glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                          GLX_BUFFER_SIZE, &val);
                                     if (val != (int) depth) continue;
                                  }
                                glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                     GLX_DRAWABLE_TYPE, &val);
                                if (!(val & GLX_PIXMAP_BIT)) continue;
                                tex_format = GLX_TEXTURE_FORMAT_RGB_EXT;
                                glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                     GLX_ALPHA_SIZE, &val);
                                if ((depth == 32) && (!val)) continue;
                                if (val > 0)
                                  {
                                     glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                          GLX_BIND_TO_TEXTURE_RGBA_EXT, &val);
                                     if (val) tex_format = GLX_TEXTURE_FORMAT_RGBA_EXT;
                                  }
                                else
                                  {
                                     glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                          GLX_BIND_TO_TEXTURE_RGB_EXT, &val);
                                     if (val) tex_format = GLX_TEXTURE_FORMAT_RGB_EXT;
                                  }
                                glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                     GLX_Y_INVERTED_EXT, &val);
                                yinvert = val;
                                glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                     GLX_BIND_TO_TEXTURE_TARGETS_EXT,
                                                     &val);
                                tex_target = val;
                                glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                     GLX_BIND_TO_MIPMAP_TEXTURE_EXT, &val);
                                mipmap = val;
                                n->fbc = configs[j];
                                found = 1;
                                break;
                             }
                           if (found == 0)
                             {
                                found = -1;
                                goto try_again;
                             }
                           XFree(configs);
                        }
                      
                      eina_hash_add(eng_get_ob(re)->gl_context->shared->native_pm_hash, &pmid, im);
                      if ((tex_target & GLX_TEXTURE_2D_BIT_EXT))
                        target = GLX_TEXTURE_2D_EXT;
                      else if ((tex_target & GLX_TEXTURE_RECTANGLE_BIT_EXT))
                        {
                           ERR("rect!!! (not handled)");
                           target = GLX_TEXTURE_RECTANGLE_EXT;
                        }
                      if (!target)
                        {
                           ERR("broken tex-from-pixmap");
                           if (!(tex_target & GLX_TEXTURE_2D_BIT_EXT))
                             target = GLX_TEXTURE_RECTANGLE_EXT;
                           else if (!(tex_target & GLX_TEXTURE_RECTANGLE_BIT_EXT))
                             target = GLX_TEXTURE_2D_EXT;
                        }
                      
                      i = 0;
                      pixmap_att[i++] = GLX_TEXTURE_FORMAT_EXT;
                      pixmap_att[i++] = tex_format;
                      pixmap_att[i++] = GLX_MIPMAP_TEXTURE_EXT;
                      pixmap_att[i++] = mipmap;
                      if (target)
                        {
                           pixmap_att[i++] = GLX_TEXTURE_TARGET_EXT;
                           pixmap_att[i++] = target;
                        }
                      pixmap_att[i++] = 0;
                      
                      memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
                      n->pixmap = pm;
                      n->visual = vis;
                      if (glsym_glXCreatePixmap)
                        n->glx_pixmap = glsym_glXCreatePixmap(eng_get_ob(re)->disp,
                                                              n->fbc,
                                                              n->pixmap,
                                                              pixmap_att);
                      else
                        ERR("Try glXCreatePixmap on GLX with no support");
                      if (n->glx_pixmap)
                        {
//                          printf("%p: new native texture for %x | %4i x %4i @ %2i = %p\n",
//                                  n, pm, w, h, depth, n->glx_pixmap);
                           if (!target)
                             {
                                ERR("no target :(");
                                if (glsym_glXQueryDrawable)
                                  glsym_glXQueryDrawable(eng_get_ob(re)->disp,
                                                         n->pixmap,
                                                         GLX_TEXTURE_TARGET_EXT,
                                                         &target);
                             }
                           if (target == GLX_TEXTURE_2D_EXT)
                             {
                                im->native.target = GL_TEXTURE_2D;
                                im->native.mipmap = mipmap;
                             }
#  ifdef GL_TEXTURE_RECTANGLE_ARB
                           else if (target == GLX_TEXTURE_RECTANGLE_EXT)
                             {
                                im->native.target = GL_TEXTURE_RECTANGLE_ARB;
                                im->native.mipmap = 0;
                             }
#  endif
                           else
                             {
                                im->native.target = GL_TEXTURE_2D;
                                im->native.mipmap = 0;
                                ERR("still unknown target");
                             }
                        }
                      else
                        ERR("GLX Pixmap create fail");
                      im->native.yinvert     = yinvert;
                      im->native.loose       = eng_get_ob(re)->detected.loose_binding;
                      im->native.data        = n;
                      im->native.func.data   = re;
                      im->native.func.bind   = _native_bind_cb;
                      im->native.func.unbind = _native_unbind_cb;
                      im->native.func.free   = _native_free_cb;
                      
                      glsym_evas_gl_common_image_native_enable(im);
                   }
              }
         }
# endif
#endif
    }
  else if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
    {
      if (native)
        {
          n = calloc(1, sizeof(Native));
          if (n)
            {
              memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));

              eina_hash_add(eng_get_ob(re)->gl_context->shared->native_tex_hash, &texid, im);

              n->pixmap = 0;
              n->visual = 0;
#ifdef GL_GLES
              n->egl_surface = 0;
#else
              n->fbc = 0;
              n->glx_pixmap = 0;
#endif

              im->native.yinvert     = 0;
              im->native.loose       = 0;
              im->native.data        = n;
              im->native.func.data   = re;
              im->native.func.bind   = _native_bind_cb;
              im->native.func.unbind = _native_unbind_cb;
              im->native.func.free   = _native_free_cb;
              im->native.target      = GL_TEXTURE_2D;
              im->native.mipmap      = 0;

              // FIXME: need to implement mapping sub texture regions
              // x, y, w, h for possible texture atlasing

              glsym_evas_gl_common_image_native_enable(im);
            }
        }

    }
   return im;
}

static int
module_open(Evas_Module *em)
{
   static Eina_Bool xrm_inited = EINA_FALSE;
   if (!xrm_inited)
     {
        xrm_inited = EINA_TRUE;
        XrmInitialize();
     }
   if (!em) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "gl_generic")) return 0;
   if (_evas_engine_GL_X11_log_dom < 0)
     _evas_engine_GL_X11_log_dom = eina_log_domain_register
       ("evas-gl_x11", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_GL_X11_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   if (partial_render_debug == -1)
     {
        if (getenv("EVAS_GL_PARTIAL_DEBUG")) partial_render_debug = 1;
        else partial_render_debug = 0;
     }

   /* store it for later use */
   func = pfunc;
   /* now to override methods */
   #define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(info);
   ORD(info_free);
   ORD(setup);
   ORD(canvas_alpha_get);
   ORD(output_free);
   ORD(output_dump);

   ORD(image_native_set);

   gl_symbols();

   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
    eina_log_domain_unregister(_evas_engine_GL_X11_log_dom);
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "gl_x11",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, gl_x11);

#ifndef EVAS_STATIC_BUILD_GL_XLIB
EVAS_EINA_MODULE_DEFINE(engine, gl_x11);
#endif

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
