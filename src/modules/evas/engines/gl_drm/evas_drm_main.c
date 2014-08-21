#include "evas_engine.h"

/* local variables */
static Outbuf *_evas_gl_drm_window = NULL;
static EGLContext context = EGL_NO_CONTEXT;
static int win_count = 0;

/* local function prototypes */
static void _outbuf_buffer_swap(Outbuf *ob, Eina_Rectangle *rects, unsigned int count);
static void _outbuf_flush_famebuffer(Outbuf *ob);

/* local functions */
static void
_outbuf_buffer_swap(Outbuf *ob, Eina_Rectangle *rects EINA_UNUSED, unsigned int count EINA_UNUSED)
{
   Buffer *buff;

   buff = &(ob->priv.buffer[ob->priv.curr]);

   buff->bo = gbm_surface_lock_front_buffer(ob->surface);

   /* if this buffer is not valid, we need to set it */
   if (!buff->valid) evas_drm_outbuf_framebuffer_set(ob, buff);

   /* send this buffer to the crtc */
   evas_drm_framebuffer_send(ob, buff);
}

static void
_outbuf_flush_famebuffer(Outbuf *ob)
{
   Eina_Rectangle *rects = NULL;
   unsigned int n = 0;
   //TODO: add region flush routine for SwapBuffersWithDamage

   /* force a buffer swap */
   _outbuf_buffer_swap(ob, rects, n);
}

Outbuf *eng_window_new(Evas_Engine_Info_GL_Drm *info, Evas *e, struct gbm_device *gbm, struct gbm_surface *surface, int screen, int depth, int w, int h, int indirect EINA_UNUSED, int alpha, int rot, Render_Engine_Swap_Mode swap_mode)
{
   Outbuf *gw;
   int context_attrs[3];
   int config_attrs[40];
   int major_version, minor_version;
   int num_config, n = 0;
   const GLubyte *vendor, *renderer, *version;
   Eina_Bool blacklist = EINA_FALSE;
   char *num;

   /* try to allocate space for outbuf */
   gw = calloc(1, sizeof(Outbuf));
   if (!gw) return NULL;

   /* set properties of outbuf */
   win_count++;
   gw->gbm = gbm;
   gw->surface = surface;
   gw->screen = screen;
   gw->depth = depth;
   gw->w = w;
   gw->h = h;
   gw->alpha = alpha;
   gw->rot = rot;
   gw->swap_mode = swap_mode;
   gw->info = info;
   gw->evas = e;

   /* setup drm outbuf */
   /* set drm card fd */
   gw->priv.fd = info->info.fd;
   /* try to setup the drm card for this outbuf */
   if (!evas_drm_outbuf_setup(gw))
     {
        ERR("Could not setup drm outbuf");
        free(gw);
        return NULL;
     }

   if (gw->w < gw->priv.mode.hdisplay) gw->w = gw->priv.mode.hdisplay;
   if (gw->h < gw->priv.mode.vdisplay) gw->h = gw->priv.mode.vdisplay;

   info->info.output = gw->priv.fb;
   // TODO: change vsync for drm egl
   //gw->vsync = info->vsync;

   gw->priv.num = NUM_BUFFERS;
   /* check for buffer override */
   // TODO: change for gbm_bo related drm buffer number.
   if ((num = getenv("EVAS_GL_DRM_BUFFERS")))
     {
        gw->priv.num = atoi(num);

        /* cap maximum # of buffers */
        if (gw->priv.num <= 0) gw->priv.num = 1;
        else if (gw->priv.num > 3) gw->priv.num = 3;
     }
   /* end drm outbuf setup */

   /* setup gbm egl surface */
   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = 2;
   context_attrs[2] = EGL_NONE;

   config_attrs[n++] = EGL_SURFACE_TYPE;
   config_attrs[n++] = EGL_WINDOW_BIT;
   config_attrs[n++] = EGL_RED_SIZE;
   config_attrs[n++] = 1;
   config_attrs[n++] = EGL_GREEN_SIZE;
   config_attrs[n++] = 1;
   config_attrs[n++] = EGL_BLUE_SIZE;
   config_attrs[n++] = 1;
   config_attrs[n++] = EGL_ALPHA_SIZE;
   if (gw->alpha) config_attrs[n++] = 1;
   else config_attrs[n++] = 0;
   config_attrs[n++] = EGL_RENDERABLE_TYPE;
   config_attrs[n++] = EGL_OPENGL_ES2_BIT;
   config_attrs[n++] = EGL_NONE;

   DBG("GBM DEVICE: %x", (unsigned int)gbm);
   gw->egl_disp = eglGetDisplay((EGLNativeDisplayType)(gw->gbm));
   if (gw->egl_disp  == EGL_NO_DISPLAY)
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

   gw->egl_surface[0] =
     eglCreateWindowSurface(gw->egl_disp, gw->egl_config,
                            (EGLNativeWindowType)gw->surface, NULL);
   if (gw->egl_surface[0] == EGL_NO_SURFACE)
     {
        ERR("eglCreateWindowSurface() fail for %p. code=%#x",
            gw->surface, eglGetError());
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

   gw->gl_context = glsym_evas_gl_common_context_new();
   if (!gw->gl_context)
     {
        eng_window_free(gw);
        return NULL;
     }

   gw->gl_context->egldisp = gw->egl_disp;
   gw->gl_context->eglctxt = gw->egl_context[0];

   eng_window_use(gw);
   glsym_evas_gl_common_context_resize(gw->gl_context, w, h, rot);

   gw->surf = EINA_TRUE;

   return gw;
}

void
eng_window_free(Outbuf *gw)
{
   int ref = 0;

   win_count--;
   eng_window_use(gw);

   if (gw == _evas_gl_drm_window) _evas_gl_drm_window = NULL;

   if (gw->gl_context)
     {
        ref = gw->gl_context->references - 1;
        glsym_evas_gl_common_context_free(gw->gl_context);
     }

   eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

   if (gw->egl_context[0] != context)
     eglDestroyContext(gw->egl_disp, gw->egl_context[0]);

   if (gw->egl_surface[0] != EGL_NO_SURFACE)
     eglDestroySurface(gw->egl_disp, gw->egl_surface[0]);

//TODO: consider gbm_surface destroy or not.
#if 0
   if (gw->surface)
     {
        gbm_surface_destroy(gw->surface);
        gw->info->info.surface = NULL;
     }
#endif

   if (ref == 0)
     {
        if (context) eglDestroyContext(gw->egl_disp, context);
        eglTerminate(gw->egl_disp);
        eglReleaseThread();
        context = EGL_NO_CONTEXT;
     }
   free(gw);
}

Eina_Bool
eng_window_make_current(void *data, void *doit)
{
   Outbuf *gw;

   if (!(gw = data)) return EINA_FALSE;

   if (doit)
     {
        if (!eglMakeCurrent(gw->egl_disp, gw->egl_surface[0],
                            gw->egl_surface[0], gw->egl_context[0]))
          return EINA_FALSE;
     }
   else
     {
        if (!eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE,
                            EGL_NO_SURFACE, EGL_NO_CONTEXT))
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

void
eng_window_use(Outbuf *gw)
{
   Eina_Bool force = EINA_FALSE;

   glsym_evas_gl_preload_render_lock(eng_window_make_current, gw);

   if (_evas_gl_drm_window)
     {
        if (eglGetCurrentContext() != _evas_gl_drm_window->egl_context[0])
          force = EINA_TRUE;
     }

   if ((_evas_gl_drm_window != gw) || (force))
     {
        if (_evas_gl_drm_window)
          {
             glsym_evas_gl_common_context_use(_evas_gl_drm_window->gl_context);
             glsym_evas_gl_common_context_flush(_evas_gl_drm_window->gl_context);
          }

        _evas_gl_drm_window = gw;

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

   if (gw) glsym_evas_gl_common_context_use(gw->gl_context);
}

void
eng_window_unsurf(Outbuf *gw)
{
   if (!gw->surf) return;
   if (!getenv("EVAS_GL_WIN_RESURF")) return;
   if (getenv("EVAS_GL_INFO")) printf("unsurf %p\n", gw);

   if (_evas_gl_drm_window)
      glsym_evas_gl_common_context_flush(_evas_gl_drm_window->gl_context);
   if (_evas_gl_drm_window == gw)
     {
        eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE,
                       EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (gw->egl_surface[0] != EGL_NO_SURFACE)
           eglDestroySurface(gw->egl_disp, gw->egl_surface[0]);
        gw->egl_surface[0] = EGL_NO_SURFACE;

        _evas_gl_drm_window = NULL;
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
                            (EGLNativeWindowType)gw->surface, NULL);

   if (gw->egl_surface[0] == EGL_NO_SURFACE)
     {
        ERR("eglCreateWindowSurface() fail for %p. code=%#x",
            gw->surface, eglGetError());
        return;
     }

   if (eglMakeCurrent(gw->egl_disp, gw->egl_surface[0], gw->egl_surface[0],
                      gw->egl_context[0]) == EGL_FALSE)
     ERR("eglMakeCurrent() failed!");

   gw->surf = EINA_TRUE;
}

Context_3D *
eng_gl_context_new(Outbuf *gw)
{
   Context_3D *ctx;
   int context_attrs[3] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

   if (!gw) return NULL;

   ctx = calloc(1, sizeof(Context_3D));
   if (!ctx) return NULL;

   ctx->context = eglCreateContext(gw->egl_disp, gw->egl_config,
                                   gw->egl_context[0], context_attrs);

   if (!ctx->context)
     {
        ERR("EGL context creation failed.");
        goto error;
     }

   ctx->display = gw->egl_disp;
   ctx->surface = gw->egl_surface[0];

   return ctx;

error:
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
        ERR("eglMakeCurrent() failed.");
     }
}

void
eng_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth EINA_UNUSED)
{
   ob->w = w;
   ob->h = h;
   ob->rot = rot;
   eng_window_use(ob);
   glsym_evas_gl_common_context_resize(ob->gl_context, w, h, rot);

   //TODO: need drm gbm surface destroy & re-create.?
}

int
eng_outbuf_get_rot(Outbuf *ob)
{
   return ob->rot;
}

Render_Engine_Swap_Mode
eng_outbuf_swap_mode(Outbuf *ob)
{
   if (ob->swap_mode == MODE_AUTO && extn_have_buffer_age)
     {
        Render_Engine_Swap_Mode swap_mode;

        EGLint age = 0;

        if (!eglQuerySurface(ob->egl_disp,
                             ob->egl_surface[0],
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
   ob->gl_context->preserve_bit = GL_COLOR_BUFFER_BIT0_QCOM;

   glsym_evas_gl_preload_render_lock(eng_preload_make_current, ob);
   eng_window_use(ob);
   if (!_re_wincheck(ob)) return EINA_TRUE;

   glsym_evas_gl_common_context_resize(ob->gl_context,
                                       ob->w, ob->h,
                                       ob->rot);

   glsym_evas_gl_common_context_flush(ob->gl_context);
   glsym_evas_gl_common_context_newframe(ob->gl_context);

   return EINA_FALSE;
}

void*
eng_outbuf_new_region_for_update(Outbuf *ob, int x, int y, int w, int h, int *cx EINA_UNUSED, int *cy EINA_UNUSED, int *cw EINA_UNUSED, int *ch EINA_UNUSED)
{
   if (w == ob->w && h == ob->h)
     {
        ob->gl_context->master_clip.enabled = EINA_FALSE;
     }
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
eng_outbuf_push_updated_region(Outbuf *ob, RGBA_Image *update EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   /* Is it really necessary to flush per region ? Shouldn't we be able to
      still do that for the full canvas when doing partial update */
   if (!_re_wincheck(ob)) return;
   ob->draw.drew = EINA_TRUE;
   glsym_evas_gl_common_context_flush(ob->gl_context);
}

void
eng_outbuf_push_free_region_for_update(Outbuf *ob EINA_UNUSED, RGBA_Image *update EINA_UNUSED)
{
   /* Nothing to do here as we don't really create an image per area */
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
        ob->vsync = 1;
     }
   if (ob->info->callback.pre_swap)
     {
        ob->info->callback.pre_swap(ob->info->callback.data, ob->evas);
     }
// TODO: Check eglSwapBuffersWithDamage for gl_drm and apply
#if 0
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
             glsym_eglSwapBuffersWithDamage(ob->egl_disp,
                                            ob->egl_surface[0],
                                            result, num);
          }
     }
   else
#endif
      eglSwapBuffers(ob->egl_disp, ob->egl_surface[0]);

   if (ob->info->callback.post_swap)
     {
        ob->info->callback.post_swap(ob->info->callback.data, ob->evas);
     }

   //Flush GL Surface data to Framebuffer
   _outbuf_flush_famebuffer(ob);

   ob->frame_cnt++;

 end:
   //TODO: Need render unlock after drm page flip?
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
