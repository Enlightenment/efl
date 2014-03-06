#include "evas_engine.h"
#include <dlfcn.h>

#ifdef HAVE_DRM_HW_ACCEL
# define EGL_EGLEXT_PROTOTYPES
# define GL_GLEXT_PROTOTYPES

# include <GLES2/gl2.h>
# include <GLES2/gl2ext.h>
# include <EGL/egl.h>
# include <EGL/eglext.h>
#endif

/* NB: This union is the same in GBM and TBM so we can use it 
 * to return 'handles' */
union _ebi_bo_handle
{
   void *ptr;
   int32_t s32;
   uint32_t u32;
   int64_t s64;
   uint64_t u64;
};

enum _ebi_bo_format
{
   EBI_BO_FORMAT_XRGB8888,
   EBI_BO_FORMAT_ARGB8888
};

enum _ebi_bo_gbm_flags
{
   EBI_BO_GBM_SCANOUT = (1 << 0),
   EBI_BO_GBM_CURSOR = (1 << 1),
   EBI_BO_GBM_RENDER = (1 << 2),
   EBI_BO_GBM_WRITE = (1 << 3),
};

enum _ebi_bo_tbm_flags
{
   EBI_BO_TBM_DEFAULT = 0,
   EBI_BO_TBM_SCANOUT = (1 << 0),
   EBI_BO_TBM_NONCACHE = (1 << 1),
   EBI_BO_TBM_WC = (1 << 2),
   EBI_BO_TBM_VENDOR = (0xffff0000),
};

typedef struct _Evas_Bufmgr_Interface Evas_Bufmgr_Interface;
struct _Evas_Bufmgr_Interface
{
   int fd;

   /* actual library we linked to */
   void *lib;

   /* actual buffer manager returned from library init */
   void *mgr;

   union
     {
        struct 
          {
             void *(*init)(int fd);
             void (*shutdown)(void *mgr);
             void *(*surface_create)(void *mgr, unsigned int width, unsigned int height, unsigned int format, unsigned int flags);
             void (*surface_destroy)(void *surface);
             void *(*buffer_create)(void *mgr, unsigned int width, unsigned int height, unsigned int format, unsigned int flags);
             void (*buffer_destroy)(void *buffer);
             union _ebi_bo_handle (*buffer_handle_get)(void *buffer);
             unsigned int (*buffer_stride_get)(void *buffer);
          } gbm;
        struct 
          {
             void *(*init)(int fd);
             void (*shutdown)(void *mgr);
          } tbm;
     } funcs;

#ifdef HAVE_DRM_HW_ACCEL
   struct
     {
        EGLDisplay disp;
        EGLContext ctx;
        EGLConfig cfg;

        PFNEGLCREATEIMAGEKHRPROC image_create;
        PFNEGLDESTROYIMAGEKHRPROC image_destroy;
        PFNGLEGLIMAGETARGETTEXTURE2DOESPROC image_texture;
     } egl;
#endif
};

/* buffer manager interface */
static Evas_Bufmgr_Interface *_ebi;

static Eina_Bool 
_evas_bufmgr_egl_init(void)
{
#ifdef HAVE_DRM_HW_ACCEL
   /* const char *ext; */
   EGLint maj, min, n;
   EGLint atts[] = 
     {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT, 
        EGL_RED_SIZE, 1, EGL_GREEN_SIZE, 1, EGL_BLUE_SIZE, 1, 
        EGL_ALPHA_SIZE, 1, EGL_DEPTH_SIZE, 1, 
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE
     };

   /* try to get the egl display from the manager */
   _ebi->egl.disp = eglGetDisplay(_ebi->mgr);
   if (_ebi->egl.disp == EGL_NO_DISPLAY)
     {
        ERR("Could not get EGLDisplay: %m");
        return EINA_FALSE;
     }

   /* try to init egl library */
   if (!eglInitialize(_ebi->egl.disp, &maj, &min))
     {
        ERR("Could not init EGL library: %m");
        goto init_err;
     }

   /* check egl extensions for what we need */
   /* ext = eglQueryString(_ebi->egl.disp, EGL_EXTENSIONS); */
   /* if (!strstr(ext, "EGL_KHR_surfaceless_opengl")) */
   /*   { */
   /*      ERR("EGL_KHR_surfaceless_opengl Not Supported"); */
   /*      goto init_err; */
   /*   } */

   eglBindAPI(EGL_OPENGL_ES_API);

   /* try to find matching egl config */
   if (!eglChooseConfig(_ebi->egl.disp, atts, &_ebi->egl.cfg, 1, &n) || 
       (n != 1))
     {
        ERR("Could not find EGLConfig: %m");
        goto init_err;
     }

   /* try to create a context */
   if (!(_ebi->egl.ctx = 
         eglCreateContext(_ebi->egl.disp, _ebi->egl.cfg, EGL_NO_CONTEXT, NULL)))
     {
        ERR("Could not create EGL Context: %m");
        goto init_err;
     }

   /* try to make this context current */
   if (!eglMakeCurrent(_ebi->egl.disp, EGL_NO_SURFACE, EGL_NO_SURFACE, 
                       _ebi->egl.ctx))
     {
        ERR("Could not make EGL Context current: %m");
        goto curr_err;
     }

   /* TODO: setup shaders ?? */

   /* link to egl functions */
   _ebi->egl.image_create = (PFNEGLCREATEIMAGEKHRPROC)
     eglGetProcAddress("eglCreateImageKHR");
   _ebi->egl.image_destroy = (PFNEGLDESTROYIMAGEKHRPROC)
     eglGetProcAddress("eglDestroyImageKHR");
   _ebi->egl.image_texture = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)
     eglGetProcAddress("glEGLImageTargetTexture2DOES");

   return EINA_TRUE;

curr_err:
   /* destroy the egl context */
   eglDestroyContext(_ebi->egl.disp, _ebi->egl.ctx);

init_err:
   /* close egl display */
   eglTerminate(_ebi->egl.disp);
   eglReleaseThread();
#endif
   return EINA_FALSE;
}

static void 
_evas_bufmgr_egl_shutdown(void)
{
#ifdef HAVE_DRM_HW_ACCEL
   if (_ebi->egl.disp)
     {
        eglMakeCurrent(_ebi->egl.disp, 
                       EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(_ebi->egl.disp);
        eglReleaseThread();
     }
#endif
}

static Eina_Bool 
_evas_bufmgr_gbm_init(void)
{
   int flags = 0;

   /* set dlopen flags */
   flags = (RTLD_LAZY | RTLD_GLOBAL);

   /* try to find gbm library */
   _ebi->lib = dlopen("libgbm.so", flags);
   if (!_ebi->lib) _ebi->lib = dlopen("libgbm.so.1", flags);
   if (!_ebi->lib) _ebi->lib = dlopen("libgbm.so.1.0.0", flags);

   /* fail if we did not find the library */
   if (!_ebi->lib) return EINA_FALSE;

   /* with the library found, lets symlink the functions we need */
   _ebi->funcs.gbm.init = dlsym(_ebi->lib, "gbm_create_device");
   _ebi->funcs.gbm.shutdown = dlsym(_ebi->lib, "gbm_device_destroy");
   _ebi->funcs.gbm.surface_create = dlsym(_ebi->lib, "gbm_surface_create");
   _ebi->funcs.gbm.surface_destroy = dlsym(_ebi->lib, "gbm_surface_destroy");
   _ebi->funcs.gbm.buffer_create = dlsym(_ebi->lib, "gbm_bo_create");
   _ebi->funcs.gbm.buffer_destroy = dlsym(_ebi->lib, "gbm_bo_destroy");
   _ebi->funcs.gbm.buffer_handle_get = dlsym(_ebi->lib, "gbm_bo_get_handle");
   _ebi->funcs.gbm.buffer_stride_get = dlsym(_ebi->lib, "gbm_bo_get_stride");

   return EINA_TRUE;
}

static Eina_Bool 
_evas_bufmgr_tbm_init(void)
{
   int flags = 0;

   /* set dlopen flags */
   flags = (RTLD_LAZY | RTLD_GLOBAL);

   /* try to find gbm library */
   _ebi->lib = dlopen("libtbm.so", flags);
   if (!_ebi->lib) _ebi->lib = dlopen("libtbm.so.1", flags);
   if (!_ebi->lib) _ebi->lib = dlopen("libtbm.so.1.0.0", flags);

   /* fail if we did not find the library */
   if (!_ebi->lib) return EINA_FALSE;

   /* with the library found, lets symlink the functions we need */
   /* TODO */

   return EINA_TRUE;
}

Eina_Bool 
evas_bufmgr_init(int fd)
{
   /* if we already have an interface, then we have been here before */
   if (_ebi) return EINA_TRUE;

   /* try to allocate space for interface */
   if (!(_ebi = calloc(1, sizeof(Evas_Bufmgr_Interface)))) return EINA_FALSE;

   /* save drm card fd for later use */
   _ebi->fd = fd;

   /* try to find and link gbm library first */
   if (!_evas_bufmgr_gbm_init())
     {
        /* try to find and link tbm library */
        if (!_evas_bufmgr_tbm_init())
          {
             ERR("Could not init buffer library");
             goto err;
          }
     }

   /* with the library open and functions linked, lets try to init */
   if (_ebi->funcs.gbm.init)
     _ebi->mgr = _ebi->funcs.gbm.init(fd);
   else if (_ebi->funcs.tbm.init)
     _ebi->mgr = _ebi->funcs.tbm.init(fd);

   if (!_ebi->mgr)
     {
        ERR("Could not init buffer manager");
        goto init_err;
     }

   /* with the manager initialized, we need to init egl */
   if (!_evas_bufmgr_egl_init())
     WRN("Could not init egl");

   return EINA_TRUE;

/* egl_err: */
/*    if (_ebi->mgr) */
/*      { */
/*         if (_ebi->funcs.gbm.shutdown) _ebi->funcs.gbm.shutdown(_ebi->mgr); */
/*         else if (_ebi->funcs.tbm.shutdown) _ebi->funcs.tbm.shutdown(_ebi->mgr); */
/*         _ebi->mgr = NULL; */
/*      } */

init_err:
   /* close library */
   if (_ebi->lib) dlclose(_ebi->lib);
   _ebi->lib = NULL;
err:
   /* free allocated space */
   free(_ebi);
   _ebi = NULL;

   return EINA_FALSE;
}

void 
evas_bufmgr_shutdown(void)
{
   /* check for valid interface */
   if (!_ebi) return;

   /* shutdown egl */
   _evas_bufmgr_egl_shutdown();

   /* shutdown manager */
   if (_ebi->mgr)
     {
        if (_ebi->funcs.gbm.shutdown) _ebi->funcs.gbm.shutdown(_ebi->mgr);
        else if (_ebi->funcs.tbm.shutdown) _ebi->funcs.tbm.shutdown(_ebi->mgr);
        _ebi->mgr = NULL;
     }

   /* close library */
   if (_ebi->lib) dlclose(_ebi->lib);
   _ebi->lib = NULL;

   /* free allocated space */
   free(_ebi);
   _ebi = NULL;
}

void *
evas_bufmgr_window_create(void *surface)
{
   void *win;

   /* check for valid interface */
   if ((!_ebi) || (_ebi->egl.disp == EGL_NO_DISPLAY)) return NULL;

   /* try to create the egl window surface */
   win = eglCreateWindowSurface(_ebi->egl.disp, _ebi->egl.cfg, 
                                (EGLNativeWindowType)surface, NULL);
   if (win == EGL_NO_SURFACE)
     {
        ERR("Failed to create egl window surface: %m");
        return NULL;
     }

   /* try to make this window surface current */
   if (!eglMakeCurrent(_ebi->egl.disp, win, win, _ebi->egl.ctx))
     ERR("Could not make window surface current: %m");

   return win;
}

void 
evas_bufmgr_window_destroy(void *win)
{
   /* check for valid interface */
   if ((!_ebi) || (_ebi->egl.disp == EGL_NO_DISPLAY)) return;

   /* destroy the egl window */
   if (win) eglDestroySurface(_ebi->egl.disp, win);
}

void *
evas_bufmgr_surface_create(int w, int h, Eina_Bool alpha)
{
   unsigned int format, flags;

   /* check for valid interface */
   if ((!_ebi) || (!_ebi->mgr)) return NULL;

   format = EBI_BO_FORMAT_XRGB8888;
   if (alpha) format = EBI_BO_FORMAT_ARGB8888;

   /* call function to create surface */
   if (_ebi->funcs.gbm.surface_create)
     {
        flags = (EBI_BO_GBM_SCANOUT | EBI_BO_GBM_RENDER);
        return _ebi->funcs.gbm.surface_create(_ebi->mgr, w, h, format, flags);
     }

   /* else if (_ebi->funcs.tbm.surface_create) */
   /*   surface = _ebi->funcs.tbm.surface_create(); */

   return NULL;
}

void 
evas_bufmgr_surface_destroy(void *surface)
{
   /* check for valid interface */
   if ((!_ebi) || (!_ebi->mgr)) return;

   /* check for valid surface */
   if (!surface) return;

   /* call destroy surface */
   if (_ebi->funcs.gbm.surface_destroy)
     _ebi->funcs.gbm.surface_destroy(surface);

   /* else if (_ebi->funcs.tbm.surface_destroy) */
   /*   _ebi->funcs.tbm.surface_destroy(surface); */
}

void *
evas_bufmgr_buffer_create(int w, int h, Eina_Bool alpha)
{
   unsigned int format, flags;

   /* check for valid interface */
   if ((!_ebi) || (!_ebi->mgr)) return NULL;

   format = EBI_BO_FORMAT_XRGB8888;
   if (alpha) format = EBI_BO_FORMAT_ARGB8888;

   /* call function to create surface */
   if (_ebi->funcs.gbm.buffer_create)
     {
        flags = (EBI_BO_GBM_SCANOUT | EBI_BO_GBM_RENDER);
        return _ebi->funcs.gbm.buffer_create(_ebi->mgr, w, h, format, flags);
     }

   /* else if (_ebi->funcs.tbm.buffer_create) */
   /*   surface = _ebi->funcs.tbm.buffer_create(); */

   return NULL;
}

void 
evas_bufmgr_buffer_destroy(void *buffer)
{
   /* check for valid interface */
   if ((!_ebi) || (!_ebi->mgr)) return;

   /* check for valid buffer */
   if (!buffer) return;

   /* call function to destroy buffer */
   if (_ebi->funcs.gbm.buffer_destroy)
     _ebi->funcs.gbm.buffer_destroy(buffer);

   /* else if (_ebi->funcs.tbm.buffer_destroy) */
   /*   _ebi->funcs.tbm.buffer_destroy(buffer); */
}

int 
evas_bufmgr_buffer_handle_get(void *buffer)
{
   /* check for valid interface */
   if ((!_ebi) || (!_ebi->mgr)) return 0;

   /* check for valid buffer */
   if (!buffer) return 0;

   /* call function to get buffer handle */
   if (_ebi->funcs.gbm.buffer_handle_get)
     return _ebi->funcs.gbm.buffer_handle_get(buffer).u32;

   return 0;
}

int 
evas_bufmgr_buffer_stride_get(void *buffer)
{
   /* check for valid interface */
   if ((!_ebi) || (!_ebi->mgr)) return 0;

   /* check for valid buffer */
   if (!buffer) return 0;

   /* call function to get buffer stride */
   if (_ebi->funcs.gbm.buffer_stride_get)
     return _ebi->funcs.gbm.buffer_stride_get(buffer);

   /* else if (_ebi->funcs.tbm.buffer_stride_get) */
   /*   return _ebi->funcs.tbm.buffer_stride_get(buffer); */

   return 0;
}

void *
evas_bufmgr_image_create(void *buffer)
{
   /* check for valid interface */
   if ((!_ebi) || (_ebi->egl.disp == EGL_NO_DISPLAY)) return NULL;

   /* check for valid buffer */
   if (!buffer) return NULL;

   /* try to create an egl image from this buffer */
   if (_ebi->egl.image_create)
     return _ebi->egl.image_create(_ebi->egl.disp, EGL_NO_CONTEXT, 
                                   EGL_NATIVE_PIXMAP_KHR, buffer, NULL);

   return NULL;
}

void 
evas_bufmgr_image_destroy(void *image)
{
   /* check for valid interface */
   if ((!_ebi) || (_ebi->egl.disp == EGL_NO_DISPLAY)) return;

   /* check for valid image */
   if (!image) return;

   /* destroy egl image */
   if (_ebi->egl.image_destroy)
     _ebi->egl.image_destroy(_ebi->egl.disp, image);
}

unsigned int 
evas_bufmgr_texture_create(void *image)
{
   GLuint tex;

   /* check for valid interface */
   if ((!_ebi) || (_ebi->egl.disp == EGL_NO_DISPLAY)) return 0;

   /* check for valid image */
   if (!image) return 0;

   glGenTextures(1, &tex);
   glBindTexture(GL_TEXTURE_2D, tex);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   if (_ebi->egl.image_texture) _ebi->egl.image_texture(GL_TEXTURE_2D, image);
   glBindTexture(GL_TEXTURE_2D, 0);

   return tex;
}
