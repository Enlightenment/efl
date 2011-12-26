#include "evas_engine.h"

/* local function prototypes */
static Eina_Bool _evas_outbuf_config_get(Outbuf *ob);

/* local variables */

void 
evas_outbuf_free(Outbuf *ob) 
{
   if (!ob) return;
   if (ob->priv.egl.disp) 
     {
        if (ob->priv.gl.context) 
          evas_gl_common_context_free(ob->priv.gl.context);

        if (ob->priv.egl.surface != EGL_NO_SURFACE)
          eglDestroySurface(ob->priv.egl.disp, ob->priv.egl.surface);

        if (ob->priv.egl.context) 
          eglDestroyContext(ob->priv.egl.disp, ob->priv.egl.context);

        if (ob->priv.win) wl_egl_window_destroy(ob->priv.win);

        /* NB: Fixme: This may close display, have to test */
        eglTerminate(ob->priv.egl.disp);

        eglMakeCurrent(ob->priv.egl.disp, 
                       EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
     }
   free(ob);
}

void 
evas_outbuf_resize(Outbuf *ob, int w, int h) 
{
   if (!ob) return;
   if ((ob->w == w) && (ob->h == h)) return;
   ob->w = w;
   ob->h = h;
   wl_egl_window_resize(ob->priv.win, w, h, 0, 0);
   if (ob->priv.gl.context) 
     {
        evas_gl_common_context_use(ob->priv.gl.context);
        evas_gl_common_context_resize(ob->priv.gl.context, w, h, ob->rot);
        evas_gl_common_context_flush(ob->priv.gl.context);
     }
}

Outbuf *
evas_outbuf_setup(struct wl_display *disp, struct wl_compositor *comp, struct wl_shell *shell, int w, int h, int rot) 
{
   Outbuf *ob = NULL;
   EGLint major, minor;
   struct wl_shell_surface *ssurface;
   int cattr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
   int sattr[] = { EGL_ALPHA_FORMAT, EGL_ALPHA_FORMAT_PRE, EGL_NONE };

   if (!(ob = calloc(1, sizeof(Outbuf)))) return NULL;

   ob->w = w;
   ob->h = h;
   ob->rot = rot;
   ob->priv.shell = shell;

   if (!(ob->priv.egl.disp = eglGetDisplay(disp))) 
     {
        ERR("eglGetDisplay failed: %#x", eglGetError());
        evas_outbuf_free(ob);
        return NULL;
     }

   if (!eglInitialize(ob->priv.egl.disp, &major, &minor)) 
     {
        ERR("eglInitialize failed: %#x", eglGetError());
        evas_outbuf_free(ob);
        return NULL;
     }

   eglBindAPI(EGL_OPENGL_ES_API);
   if (eglGetError() != EGL_SUCCESS) 
     {
        ERR("eglBindAPI failed: %#x", eglGetError());
        evas_outbuf_free(ob);
        return NULL;
     }

   if (!_evas_outbuf_config_get(ob)) 
     {
        evas_outbuf_free(ob);
        return NULL;
     }

   ob->priv.egl.context = 
     eglCreateContext(ob->priv.egl.disp, ob->priv.egl.config, 
                      EGL_NO_CONTEXT, cattr);
   if (ob->priv.egl.context == EGL_NO_CONTEXT) 
     {
        ERR("eglCreateContext failed: %#x", eglGetError());
        evas_outbuf_free(ob);
        return NULL;
     }

   /* create wayland surface & window */
   ob->priv.surface = wl_compositor_create_surface(comp);
   ob->priv.win = wl_egl_window_create(ob->priv.surface, w, h);

   ob->priv.egl.surface = 
     eglCreateWindowSurface(ob->priv.egl.disp, ob->priv.egl.config, 
                            ob->priv.win, sattr);
   if (ob->priv.egl.surface == EGL_NO_SURFACE) 
     {
        ERR("eglCreateWindowSurface failed: %#x", eglGetError());
        evas_outbuf_free(ob);
        return NULL;
     }

   ssurface = wl_shell_get_shell_surface(ob->priv.shell, ob->priv.surface);
   wl_shell_surface_set_toplevel(ssurface);

   if (eglMakeCurrent(ob->priv.egl.disp, ob->priv.egl.surface, 
                      ob->priv.egl.surface, ob->priv.egl.context) == EGL_FALSE) 
     {
        ERR("eglMakeCurrent failed: %#x", eglGetError());
        evas_outbuf_free(ob);
        return NULL;
     }

   if (!(ob->priv.gl.context = evas_gl_common_context_new())) 
     {
        ERR("Cannot create evas gl common context.");
        evas_outbuf_free(ob);
        return NULL;
     }

   evas_gl_common_context_use(ob->priv.gl.context);
   evas_gl_common_context_resize(ob->priv.gl.context, w, h, rot);
   evas_gl_common_context_flush(ob->priv.gl.context);

   return ob;
}

/* local functions */
static Eina_Bool 
_evas_outbuf_config_get(Outbuf *ob) 
{
   int num = 0;
   int attrs[] = 
     {
        EGL_SURFACE_TYPE, 
        EGL_WINDOW_BIT | EGL_VG_ALPHA_FORMAT_PRE_BIT, 
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, 
        EGL_DEPTH_SIZE, 1, EGL_ALPHA_SIZE, 1, 
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, 
        EGL_NONE
     };

   if (!ob) return EINA_FALSE;
   if (!eglChooseConfig(ob->priv.egl.disp, attrs, &ob->priv.egl.config, 
                        1, &num) || (num != 1)) 
     {
        ERR("eglChooseConfig failed: %#x", eglGetError());
        return EINA_FALSE;
     }

   return EINA_TRUE;
}
