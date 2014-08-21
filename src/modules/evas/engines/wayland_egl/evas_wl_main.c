#include "evas_engine.h"

/* local function prototypes */

/* local variables */
static Outbuf *_evas_gl_wl_window = NULL;
static EGLContext context = EGL_NO_CONTEXT;
static int win_count = 0;

Outbuf *
eng_window_new(Evas *evas, Evas_Engine_Info_Wayland_Egl *einfo, int w, int h, Render_Engine_Swap_Mode swap_mode)
{
   Outbuf *gw;
   int context_attrs[3];
   int config_attrs[40];
   int major_version, minor_version;
   int num_config, n = 0;
   const GLubyte *vendor, *renderer, *version;
   Eina_Bool blacklist = EINA_FALSE;

   /* try to allocate space for our window */
   if (!(gw = calloc(1, sizeof(Outbuf))))
     return NULL;

   win_count++;
   gw->info = einfo;
   gw->evas = evas;
   gw->w = w;
   gw->h = h;
   gw->swap_mode = swap_mode;
   gw->disp = einfo->info.display;
   gw->surface = einfo->info.surface;
   gw->screen = einfo->info.screen;
   gw->depth = einfo->info.depth;
   gw->alpha = einfo->info.destination_alpha;
   gw->rot = einfo->info.rotation;

   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = 2;
   context_attrs[2] = EGL_NONE;

   config_attrs[n++] = EGL_SURFACE_TYPE;
   config_attrs[n++] = EGL_WINDOW_BIT;
   config_attrs[n++] = EGL_RENDERABLE_TYPE;
   config_attrs[n++] = EGL_OPENGL_ES2_BIT;
# if 0
   /* FIXME: n900 - omap3 sgx libs break here */
   config_attrs[n++] = EGL_RED_SIZE;
   config_attrs[n++] = 1;
   config_attrs[n++] = EGL_GREEN_SIZE;
   config_attrs[n++] = 1;
   config_attrs[n++] = EGL_BLUE_SIZE;
   config_attrs[n++] = 1;
   /* FIXME: end n900 breakage */
# endif
   config_attrs[n++] = EGL_ALPHA_SIZE;
   config_attrs[n++] = gw->alpha;
   config_attrs[n++] = EGL_DEPTH_SIZE;
   config_attrs[n++] = 0;
   config_attrs[n++] = EGL_STENCIL_SIZE;
   config_attrs[n++] = 0;
   config_attrs[n++] = EGL_NONE;

   /* FIXME: Remove this line as soon as eglGetDisplay() autodetection
    * gets fixed. Currently it is incorrectly detecting wl_display and
    * returning _EGL_PLATFORM_X11 instead of _EGL_PLATFORM_WAYLAND.
    *
    * See ticket #1972 for more info.
    */

   setenv("EGL_PLATFORM", "wayland", 1);

   gw->egl_disp = eglGetDisplay((EGLNativeDisplayType)gw->disp);
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

   if ((gw->rot == 0) || (gw->rot == 180))
     gw->win = wl_egl_window_create(gw->surface, gw->w, gw->h);
   else if ((gw->rot == 90) || (gw->rot == 270))
     gw->win = wl_egl_window_create(gw->surface, gw->h, gw->w);

   gw->egl_surface[0] = 
     eglCreateWindowSurface(gw->egl_disp, gw->egl_config,
                            (EGLNativeWindowType)gw->win, NULL);
   if (gw->egl_surface[0] == EGL_NO_SURFACE)
     {
        ERR("eglCreateWindowSurface() fail for %p. code=%#x", 
            gw->win, eglGetError());
        eng_window_free(gw);
        return NULL;
     }

   gw->egl_context[0] = 
     eglCreateContext(gw->egl_disp, gw->egl_config, context, context_attrs);
   if (gw->egl_context[0] == EGL_NO_CONTEXT)
     {
        ERR("eglCreateContext() fail. code=%#x", eglGetError());
        eng_window_free(gw);
        return NULL;
     }

   if (context == EGL_NO_CONTEXT) context = gw->egl_context[0];

   if (eglMakeCurrent(gw->egl_disp, gw->egl_surface[0],
                      gw->egl_surface[0], gw->egl_context[0]) == EGL_FALSE)
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

   if (strstr((const char *)vendor, "Mesa Project"))
     {
        if (strstr((const char *)renderer, "Software Rasterizer"))
          blacklist = EINA_TRUE;
     }
   if (strstr((const char *)renderer, "softpipe"))
     blacklist = EINA_TRUE;
   if (strstr((const char *)renderer, "llvmpipe"))
     blacklist = EINA_TRUE;
   if ((blacklist) && (!getenv("EVAS_GL_NO_BLACKLIST")))
     {
        ERR("OpenGL Driver blacklisted:");
        ERR("Vendor: %s", (const char *)vendor);
        ERR("Renderer: %s", (const char *)renderer);
        ERR("Version: %s", (const char *)version);
        eng_window_free(gw);
        return NULL;
     }

   if (!(gw->gl_context = glsym_evas_gl_common_context_new()))
     {
        eng_window_free(gw);
        return NULL;
     }

   gw->gl_context->egldisp = gw->egl_disp;
   gw->gl_context->eglctxt = gw->egl_context[0];

   eng_window_use(gw);
   glsym_evas_gl_common_context_resize(gw->gl_context, w, h, gw->rot);

   gw->surf = EINA_TRUE;

   return gw;
}

void 
eng_window_free(Outbuf *gw)
{
   int ref = 0;

   win_count--;
   eng_window_use(gw);

   if (gw == _evas_gl_wl_window) _evas_gl_wl_window = NULL;

   if (gw->gl_context)
     {
        ref = gw->gl_context->references - 1;
        glsym_evas_gl_common_context_free(gw->gl_context);
     }

   eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE, 
                  EGL_NO_SURFACE, EGL_NO_CONTEXT);

   if (gw->egl_context[0] != context)
     eglDestroyContext(gw->egl_disp, gw->egl_context[0]);

   if (gw->egl_surface[0] != EGL_NO_SURFACE)
     eglDestroySurface(gw->egl_disp, gw->egl_surface[0]);

   if (gw->win) wl_egl_window_destroy(gw->win);

   if (ref == 0)
     {
        if (context) eglDestroyContext(gw->egl_disp, context);
        eglTerminate(gw->egl_disp);
        eglReleaseThread();
        context = EGL_NO_CONTEXT;
     }

   free(gw);
}

void 
eng_window_use(Outbuf *gw)
{
   Eina_Bool force = EINA_FALSE;

   glsym_evas_gl_preload_render_lock(eng_preload_make_current, gw);

   if (_evas_gl_wl_window)
     {
        if (eglGetCurrentContext() != _evas_gl_wl_window->egl_context[0])
          force = EINA_TRUE;
     }

   if ((_evas_gl_wl_window != gw) || (force))
     {
        if (_evas_gl_wl_window)
          {
             glsym_evas_gl_common_context_use(_evas_gl_wl_window->gl_context);
             glsym_evas_gl_common_context_flush(_evas_gl_wl_window->gl_context);
          }

        _evas_gl_wl_window = gw;

        if (gw)
          {
             if (gw->egl_surface[0] != EGL_NO_SURFACE)
               {
                  if (eglMakeCurrent(gw->egl_disp, gw->egl_surface[0],
                                     gw->egl_surface[0],
                                     gw->egl_context[0]) == EGL_FALSE)
                    ERR("eglMakeCurrent() failed!");
               }
          }
     }

   if (gw)
     {
        glsym_evas_gl_common_context_use(gw->gl_context);
        glsym_evas_gl_common_context_resize(gw->gl_context, gw->w, gw->h, gw->rot);
     }
}

void 
eng_window_unsurf(Outbuf *gw)
{
   if (!gw->surf) return;
   if (!getenv("EVAS_GL_WIN_RESURF")) return;
   if (getenv("EVAS_GL_INFO")) printf("unsurf %p\n", gw);

   if (_evas_gl_wl_window)
     glsym_evas_gl_common_context_flush(_evas_gl_wl_window->gl_context);

   if (_evas_gl_wl_window == gw)
     {
        eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE, 
                       EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (gw->egl_surface[0] != EGL_NO_SURFACE)
          eglDestroySurface(gw->egl_disp, gw->egl_surface[0]);
        gw->egl_surface[0] = EGL_NO_SURFACE;

        _evas_gl_wl_window = NULL;
     }

   gw->surf = EINA_FALSE;
}

void 
eng_window_resurf(Outbuf *gw)
{
   if (gw->surf) return;
   if (getenv("EVAS_GL_INFO")) printf("resurf %p\n", gw);

   gw->egl_surface[0] = 
     eglCreateWindowSurface(gw->egl_disp, gw->egl_config,
                            (EGLNativeWindowType)gw->win, NULL);

   if (gw->egl_surface[0] == EGL_NO_SURFACE)
     {
        ERR("eglCreateWindowSurface() fail for %p. code=%#x",
            gw->win, eglGetError());
        return;
     }

   if (eglMakeCurrent(gw->egl_disp, gw->egl_surface[0], gw->egl_surface[0],
                      gw->egl_context[0]) == EGL_FALSE)
     ERR("eglMakeCurrent() failed!");

   gw->surf = EINA_TRUE;
}

void 
eng_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth EINA_UNUSED)
{
   ob->w = w;
   ob->h = h;
   ob->rot = rot;
   eng_window_use(ob);
   glsym_evas_gl_common_context_resize(ob->gl_context, w, h, rot);

   if (ob->win)
     {
        int aw, ah, dx = 0, dy = 0;

        if ((ob->rot == 90) || (ob->rot == 270))
          wl_egl_window_get_attached_size(ob->win, &ah, &aw);
        else
          wl_egl_window_get_attached_size(ob->win, &aw, &ah);

        if (ob->info->info.edges & 4) // resize from left
          {
             if ((ob->rot == 90) || (ob->rot == 270))
               dx = ah - h;
             else
               dx = aw - w;
          }

        if (ob->info->info.edges & 1) // resize from top
          {
             if ((ob->rot == 90) || (ob->rot == 270))
               dy = aw - w;
             else
               dy = ah - h;
          }

        if ((ob->rot == 90) || (ob->rot == 270))
          wl_egl_window_resize(ob->win, h, w, dx, dy);
        else
          wl_egl_window_resize(ob->win, w, h, dx, dy);
     }
}

int 
eng_outbuf_rotation_get(Outbuf *ob)
{
   return ob->rot;
}

Render_Engine_Swap_Mode 
eng_outbuf_swap_mode_get(Outbuf *ob)
{
   if ((ob->swap_mode == MODE_AUTO) && (extn_have_buffer_age))
     {
        Render_Engine_Swap_Mode swap_mode;
        EGLint age = 0;

        if (!eglQuerySurface(ob->egl_disp, ob->egl_surface[0], 
                             EGL_BUFFER_AGE_EXT, &age))
          age = 0;

        if (age == 1) swap_mode = MODE_COPY;
        else if (age == 2) swap_mode = MODE_DOUBLE;
        else if (age == 3) swap_mode = MODE_TRIPLE;
        else if (age == 4) swap_mode = MODE_QUADRUPLE;
        else swap_mode = MODE_FULL;

        if ((int)age != ob->prev_age) swap_mode = MODE_FULL;
        ob->prev_age = age;
 
        return swap_mode;
     }

   return ob->swap_mode;
}

Eina_Bool 
eng_outbuf_region_first_rect(Outbuf *ob)
{
   glsym_evas_gl_preload_render_lock(eng_preload_make_current, ob);

   eng_window_use(ob);
   if (!_re_wincheck(ob)) return EINA_TRUE;

   glsym_evas_gl_common_context_flush(ob->gl_context);
   glsym_evas_gl_common_context_newframe(ob->gl_context);

   return EINA_FALSE;
}

void *
eng_outbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx EINA_UNUSED, int *cy EINA_UNUSED, int *cw EINA_UNUSED, int *ch EINA_UNUSED)
{
   if ((ob->w == w) && (ob->h == h))
     ob->gl_context->master_clip.enabled = EINA_FALSE;
   else
     {
        ob->gl_context->master_clip.enabled = EINA_TRUE;
        ob->gl_context->master_clip.x = x;
        ob->gl_context->master_clip.y = y;
        ob->gl_context->master_clip.w = w;
        ob->gl_context->master_clip.h = h;
     }

   return ob->gl_context->def_surface;
}

void 
eng_outbuf_update_region_free(Outbuf *ob EINA_UNUSED, RGBA_Image *update EINA_UNUSED)
{

}

void 
eng_outbuf_update_region_push(Outbuf *ob, RGBA_Image *update EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   if (!_re_wincheck(ob)) return;
   ob->draw.drew = EINA_TRUE;
   glsym_evas_gl_common_context_flush(ob->gl_context);
}

void 
eng_outbuf_flush(Outbuf *ob, Tilebuf_Rect *rects, Evas_Render_Mode render_mode)
{
   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) goto end;

   if (!_re_wincheck(ob)) goto end;
   if (!ob->draw.drew) goto end;

   ob->draw.drew = EINA_FALSE;
   eng_window_use(ob);
   glsym_evas_gl_common_context_done(ob->gl_context);

   if (!ob->vsync)
     {
        if (ob->info->vsync) eglSwapInterval(ob->egl_disp, 1);
        else eglSwapInterval(ob->egl_disp, 0);
        ob->vsync = EINA_TRUE;
     }

   if (ob->info->callback.pre_swap)
     ob->info->callback.pre_swap(ob->info->callback.data, ob->evas);

   if ((glsym_eglSwapBuffersWithDamage) && (ob->swap_mode != MODE_FULL))
     {
        EGLint num = 0, *result = NULL, i = 0;
        Tilebuf_Rect *r;

        // if partial swaps can be done use re->rects
        num = eina_inlist_count(EINA_INLIST_GET(rects));
        if (num > 0)
          {
             result = alloca(sizeof(EGLint) * 4 * num);
             EINA_INLIST_FOREACH(EINA_INLIST_GET(rects), r)
               {
                  int gw, gh;

                  gw = ob->gl_context->w;
                  gh = ob->gl_context->h;
                  switch (ob->rot)
                    {
                     case 0:
                       result[i + 0] = r->x;
                       result[i + 1] = gh - (r->y + r->h);
                       result[i + 2] = r->w;
                       result[i + 3] = r->h;
                       break;
                     case 90:
                       result[i + 0] = r->y;
                       result[i + 1] = r->x;
                       result[i + 2] = r->h;
                       result[i + 3] = r->w;
                       break;
                     case 180:
                       result[i + 0] = gw - (r->x + r->w);
                       result[i + 1] = r->y;
                       result[i + 2] = r->w;
                       result[i + 3] = r->h;
                       break;
                     case 270:
                       result[i + 0] = gh - (r->y + r->h);
                       result[i + 1] = gw - (r->x + r->w);
                       result[i + 2] = r->h;
                       result[i + 3] = r->w;
                       break;
                     default:
                       result[i + 0] = r->x;
                       result[i + 1] = gh - (r->y + r->h);
                       result[i + 2] = r->w;
                       result[i + 3] = r->h;
                       break;
                    }
                  i += 4;
               }
             glsym_eglSwapBuffersWithDamage(ob->egl_disp, ob->egl_surface[0],
                                            result, num);
          }
     }
   else
      eglSwapBuffers(ob->egl_disp, ob->egl_surface[0]);

   if (ob->info->callback.post_swap)
     ob->info->callback.post_swap(ob->info->callback.data, ob->evas);

   ob->frame_cnt++;

 end:
   glsym_evas_gl_preload_render_unlock(eng_preload_make_current, ob);
}

Evas_Engine_GL_Context *
eng_outbuf_gl_context_get(Outbuf *ob)
{
   return ob->gl_context;
}

void *
eng_outbuf_egl_display_get(Outbuf *ob)
{
   return ob->egl_disp;
}

Context_3D *
eng_gl_context_new(Outbuf *ob)
{
   Context_3D *ctx;
   int attrs[3];

   if (!ob) return NULL;

   attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   attrs[1] = 2;
   attrs[2] = EGL_NONE;

   if (!(ctx = calloc(1, sizeof(Context_3D)))) return NULL;

   ctx->context = 
     eglCreateContext(ob->egl_disp, ob->egl_config, ob->egl_context[0], attrs);
   if (!ctx->context)
     {
        ERR("Could not create egl context %#x", eglGetError());
        goto err;
     }

   ctx->display = ob->egl_disp;
   ctx->surface = ob->egl_surface[0];

   return ctx;

err:
   free(ctx);
   return NULL;
}

void 
eng_gl_context_free(Context_3D *ctx)
{
   eglDestroyContext(ctx->display, ctx->context);
   free(ctx);
}

void 
eng_gl_context_use(Context_3D *ctx)
{
   if (eglMakeCurrent(ctx->display, ctx->surface, 
                      ctx->surface, ctx->context) == EGL_FALSE)
     {
        ERR("eglMakeCurrent Failed: %#x", eglGetError());
     }
}
