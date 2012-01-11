#include "evas_engine.h"

static Evas_GL_Wl_Window *_evas_gl_x11_window = NULL;

static EGLContext context = EGL_NO_CONTEXT;

// fixme: something is up/wrong here - dont know what tho...
//#define NEWGL 1

static int win_count = 0;

Evas_GL_Wl_Window *
eng_window_new(struct wl_display *disp, struct wl_surface *surface, int screen,
               int depth, int w, int h, int indirect, int alpha, int rot)
{
   Evas_GL_Wl_Window *gw;
   int context_attrs[3];
   int config_attrs[40];
   int major_version, minor_version;
   int num_config, n = 0;
   const GLubyte *vendor, *renderer, *version;

   gw = calloc(1, sizeof(Evas_GL_Wl_Window));
   if (!gw) return NULL;

   win_count++;
   gw->disp = disp;
//   gw->win = win;
   gw->screen = screen;
//   gw->visual = vis;
//   gw->colormap = cmap;
   gw->depth = depth;
   gw->alpha = alpha;
   gw->w = w;
   gw->h = h;
   gw->rot = rot;

// EGL / GLES
   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = 2;
   context_attrs[2] = EGL_NONE;

#if defined(GLES_VARIETY_S3C6410)
   if (gw->visualinfo->depth == 16) // 16bpp
     {
        config_attrs[n++] = EGL_SURFACE_TYPE;
        config_attrs[n++] = EGL_WINDOW_BIT;
        config_attrs[n++] = EGL_RENDERABLE_TYPE;
        config_attrs[n++] = EGL_OPENGL_ES2_BIT;
        config_attrs[n++] = EGL_RED_SIZE;
        config_attrs[n++] = 5;
        config_attrs[n++] = EGL_GREEN_SIZE;
        config_attrs[n++] = 6;
        config_attrs[n++] = EGL_BLUE_SIZE;
        config_attrs[n++] = 5;
        config_attrs[n++] = EGL_DEPTH_SIZE;
        config_attrs[n++] = 0;
        config_attrs[n++] = EGL_STENCIL_SIZE;
        config_attrs[n++] = 0;
        config_attrs[n++] = EGL_NONE;
     }
   else // 24/32bit. no one does 8bpp anymore. and 15bpp... dead
     {
        config_attrs[n++] = EGL_SURFACE_TYPE;
        config_attrs[n++] = EGL_WINDOW_BIT;
        config_attrs[n++] = EGL_RENDERABLE_TYPE;
        config_attrs[n++] = EGL_OPENGL_ES2_BIT;
        config_attrs[n++] = EGL_RED_SIZE;
        config_attrs[n++] = 8;
        config_attrs[n++] = EGL_GREEN_SIZE;
        config_attrs[n++] = 8;
        config_attrs[n++] = EGL_BLUE_SIZE;
        config_attrs[n++] = 8;
        config_attrs[n++] = EGL_DEPTH_SIZE;
        config_attrs[n++] = 0;
        config_attrs[n++] = EGL_STENCIL_SIZE;
        config_attrs[n++] = 0;
        config_attrs[n++] = EGL_NONE;
     }
#elif defined(GLES_VARIETY_SGX)
   config_attrs[n++] = EGL_SURFACE_TYPE;
   config_attrs[n++] = EGL_WINDOW_BIT;
   config_attrs[n++] = EGL_RENDERABLE_TYPE;
   config_attrs[n++] = EGL_OPENGL_ES2_BIT;
# if 0
// FIXME: n900 - omap3 sgx libs break here
   config_attrs[n++] = EGL_RED_SIZE;
   config_attrs[n++] = 1;
   config_attrs[n++] = EGL_GREEN_SIZE;
   config_attrs[n++] = 1;
   config_attrs[n++] = EGL_BLUE_SIZE;
   config_attrs[n++] = 1;
// FIXME: end n900 breakage
# endif
   if (gw->alpha)
     {
        config_attrs[n++] = EGL_ALPHA_SIZE;
        config_attrs[n++] = 1;
     }
   else
     {
        config_attrs[n++] = EGL_ALPHA_SIZE;
        config_attrs[n++] = 0;
     }
   config_attrs[n++] = EGL_DEPTH_SIZE;
   config_attrs[n++] = 0;
   config_attrs[n++] = EGL_STENCIL_SIZE;
   config_attrs[n++] = 0;
   config_attrs[n++] = EGL_NONE;
#endif
   
   gw->egl_disp = eglGetDisplay((EGLNativeDisplayType)(gw->disp));
   if (!gw->egl_disp)
     {
        ERR("eglGetDisplay() fail. code=%#x", eglGetError());
	eng_window_free(gw);
        return NULL;
     }
   if (!eglInitialize(gw->egl_disp, &major_version, &minor_version))
     {
        ERR("eglInitialize() fail. code=%#x", eglGetError());
	eng_window_free(gw);
        return NULL;
     }
   eglBindAPI(EGL_OPENGL_ES_API);
   if (eglGetError() != EGL_SUCCESS)
     {
        ERR("eglBindAPI() fail. code=%#x", eglGetError());
	eng_window_free(gw);
        return NULL;
     }

   num_config = 0;
   if (!eglChooseConfig(gw->egl_disp, config_attrs, &gw->egl_config,
                        1, &num_config) || (num_config != 1))
     {
        ERR("eglChooseConfig() fail. code=%#x", eglGetError());
	eng_window_free(gw);
        return NULL;
     }
   /* FIXME: !!! */
   gw->win = wl_egl_window_create(surface, gw->w, gw->h);

   gw->egl_surface[0] = eglCreateWindowSurface(gw->egl_disp, gw->egl_config,
                                               (EGLNativeWindowType)gw->win,
                                               NULL);
   if (gw->egl_surface[0] == EGL_NO_SURFACE)
     {
        /* FIXME: !! */
        /* ERR("eglCreateWindowSurface() fail for %#x. code=%#x", */
        /*     (unsigned int)gw->win, eglGetError()); */
	eng_window_free(gw);
        return NULL;
     }
   if (context == EGL_NO_CONTEXT)
     context = eglCreateContext(gw->egl_disp, gw->egl_config, NULL,
                                context_attrs);
   gw->egl_context[0] = context;
   if (gw->egl_context[0] == EGL_NO_CONTEXT)
     {
        ERR("eglCreateContext() fail. code=%#x", eglGetError());
	eng_window_free(gw);
        return NULL;
     }
   if (eglMakeCurrent(gw->egl_disp, gw->egl_surface[0], gw->egl_surface[0],
                      gw->egl_context[0]) == EGL_FALSE)
     {
        ERR("eglMakeCurrent() fail. code=%#x", eglGetError());
	eng_window_free(gw);
        return NULL;
     }

   vendor = glGetString(GL_VENDOR);
   renderer = glGetString(GL_RENDERER);
   version = glGetString(GL_VERSION);
   if (!vendor) vendor   = (unsigned char *)"-UNKNOWN-";
   if (!renderer) renderer = (unsigned char *)"-UNKNOWN-";
   if (!version) version  = (unsigned char *)"-UNKNOWN-";
   if (getenv("EVAS_GL_INFO"))
     {
        fprintf(stderr, "vendor: %s\n", vendor);
        fprintf(stderr, "renderer: %s\n", renderer);
        fprintf(stderr, "version: %s\n", version);
     }

   gw->gl_context = evas_gl_common_context_new();
   if (!gw->gl_context)
     {
	eng_window_free(gw);
	return NULL;
     }
   gw->gl_context->egldisp = gw->egl_disp;
   eng_window_use(gw);
   evas_gl_common_context_resize(gw->gl_context, w, h, rot);
   gw->surf = 1;
   return gw;
   indirect = 0;
}

void
eng_window_free(Evas_GL_Wl_Window *gw)
{
   int ref = 0;

   win_count--;
   eng_window_use(gw);
   if (gw == _evas_gl_x11_window) _evas_gl_x11_window = NULL;
   if (gw->gl_context)
     {
        ref = gw->gl_context->references - 1;
        evas_gl_common_context_free(gw->gl_context);
     }
   if (gw->egl_surface[0] != EGL_NO_SURFACE)
      eglDestroySurface(gw->egl_disp, gw->egl_surface[0]);
   if (ref == 0)
     {
        if (context) eglDestroyContext(gw->egl_disp, context);
        eglTerminate(gw->egl_disp);
        context = EGL_NO_CONTEXT;
     }
   eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   free(gw);
}

void
eng_window_use(Evas_GL_Wl_Window *gw)
{
   Eina_Bool force_use = EINA_FALSE;

   if (_evas_gl_x11_window)
     {
        if ((eglGetCurrentContext() !=
             _evas_gl_x11_window->egl_context[0]) ||
            (eglGetCurrentSurface(EGL_READ) !=
                _evas_gl_x11_window->egl_surface[0]) ||
            (eglGetCurrentSurface(EGL_DRAW) !=
                _evas_gl_x11_window->egl_surface[0]))
           force_use = EINA_TRUE;
     }
   if ((_evas_gl_x11_window != gw) || (force_use))
     {
        if (_evas_gl_x11_window)
          {
             evas_gl_common_context_use(_evas_gl_x11_window->gl_context);
             evas_gl_common_context_flush(_evas_gl_x11_window->gl_context);
          }
        _evas_gl_x11_window = gw;
        if (gw)
          {
// EGL / GLES
           if (gw->egl_surface[0] != EGL_NO_SURFACE)
             {
                if (eglMakeCurrent(gw->egl_disp, gw->egl_surface[0],
                                   gw->egl_surface[0],
                                   gw->egl_context[0]) == EGL_FALSE)
                  {
                     ERR("eglMakeCurrent() failed!");
                  }
             }
          }
     }
   if (gw) evas_gl_common_context_use(gw->gl_context);
}

void
eng_window_unsurf(Evas_GL_Wl_Window *gw)
{
   if (!gw->surf) return;
   if (!getenv("EVAS_GL_WIN_RESURF")) return;
   if (getenv("EVAS_GL_INFO")) printf("unsurf %p\n", gw);

   if (_evas_gl_x11_window)
      evas_gl_common_context_flush(_evas_gl_x11_window->gl_context);
   if (_evas_gl_x11_window == gw)
     {
        eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (gw->egl_surface[0] != EGL_NO_SURFACE)
           eglDestroySurface(gw->egl_disp, gw->egl_surface[0]);
        gw->egl_surface[0] = EGL_NO_SURFACE;
        _evas_gl_x11_window = NULL;
     }
   gw->surf = 0;
}

void
eng_window_resurf(Evas_GL_Wl_Window *gw)
{
   if (gw->surf) return;
   if (getenv("EVAS_GL_INFO")) printf("resurf %p\n", gw);

   /* FIXME !! */
   /* gw->egl_surface[0] = eglCreateWindowSurface(gw->egl_disp, gw->egl_config, */
   /*                                             (EGLNativeWindowType)gw->win, */
   /*                                             NULL); */
   if (gw->egl_surface[0] == EGL_NO_SURFACE)
     {
        /* FIXME: !! */
        /* ERR("eglCreateWindowSurface() fail for %#x. code=%#x", */
        /*     (unsigned int)gw->win, eglGetError()); */
        return;
     }
   if (eglMakeCurrent(gw->egl_disp, gw->egl_surface[0], gw->egl_surface[0],
                      gw->egl_context[0]) == EGL_FALSE)
     {
        ERR("eglMakeCurrent() failed!");
     }
   gw->surf = 1;
}

int
eng_best_depth_get(Evas_Engine_Info_GL_Wl *einfo)
{
   if (!einfo) return 0;
   if (!einfo->info.display) return 0;
   return 32;
   /* if (!_evas_gl_x11_vi) eng_best_visual_get(einfo); */
   /* if (!_evas_gl_x11_vi) return 0; */
   /* if (einfo->info.destination_alpha) */
   /*   { */
   /*      if (_evas_gl_x11_rgba_vi) return _evas_gl_x11_rgba_vi->depth; */
   /*   } */
   /* return _evas_gl_x11_vi->depth; */
}
