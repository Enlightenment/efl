#include "evas_engine.h"

/* local variables */
static Outbuf *_evas_gl_drm_window = NULL;
static EGLContext context = EGL_NO_CONTEXT;
static int win_count = 0;

static void
_evas_outbuf_gbm_surface_destroy(Outbuf *ob)
{
   if (!ob) return;
   if (ob->surface)
     {
        gbm_surface_destroy(ob->surface);
        ob->surface = NULL;
     }
}

static void
_evas_outbuf_gbm_surface_create(Outbuf *ob, int w, int h)
{
   if (!ob) return;

   ob->surface =
     gbm_surface_create(ob->info->info.gbm, w, h,
                        ob->info->info.format, ob->info->info.flags);

   if (!ob->surface) ERR("Failed to create gbm surface: %m");
}

static void
_evas_outbuf_fb_cb_destroy(struct gbm_bo *bo, void *data)
{
   Ecore_Drm_Fb *fb;

   fb = data;
   if (fb)
     {
        struct gbm_device *gbm;

        gbm = gbm_bo_get_device(bo);
        drmModeRmFB(gbm_device_get_fd(gbm), fb->id);
        free(fb);
     }
}

static Ecore_Drm_Fb *
_evas_outbuf_fb_get(Ecore_Drm_Device *dev, struct gbm_bo *bo)
{
   int ret;
   Ecore_Drm_Fb *fb;
   uint32_t format;
   uint32_t handles[4], pitches[4], offsets[4];

   fb = gbm_bo_get_user_data(bo);
   if (fb) return fb;

   if (!(fb = calloc(1, sizeof(Ecore_Drm_Fb)))) return NULL;

   format = gbm_bo_get_format(bo);

   fb->w = gbm_bo_get_width(bo);
   fb->h = gbm_bo_get_height(bo);
   fb->hdl = gbm_bo_get_handle(bo).u32;
   fb->stride = gbm_bo_get_stride(bo);
   fb->size = fb->stride * fb->h;

   handles[0] = fb->hdl;
   pitches[0] = fb->stride;
   offsets[0] = 0;

   ret = drmModeAddFB2(dev->drm.fd, fb->w, fb->h, format,
                       handles, pitches, offsets, &(fb->id), 0);
   if (ret)
     ret = drmModeAddFB(dev->drm.fd, fb->w, fb->h, 24, 32,
                        fb->stride, fb->hdl, &(fb->id));

   if (ret) ERR("FAILED TO ADD FB: %m");

   gbm_bo_set_user_data(bo, fb, _evas_outbuf_fb_cb_destroy);

   return fb;
}

static void
_evas_outbuf_cb_pageflip(void *data)
{
   Outbuf *ob;
   Ecore_Drm_Fb *fb;
   struct gbm_bo *bo;

   if (!(ob = data)) return;

   bo = ob->priv.bo[ob->priv.curr];
   if (!bo) return;

   fb = _evas_outbuf_fb_get(ob->info->info.dev, bo);
   if (fb) fb->pending_flip = EINA_FALSE;

   gbm_surface_release_buffer(ob->surface, bo);

   ob->priv.last = ob->priv.curr;
   ob->priv.curr = (ob->priv.curr + 1) % ob->priv.num;
}

static void
_evas_outbuf_buffer_swap(Outbuf *ob, Eina_Rectangle *rects, unsigned int count)
{
   Ecore_Drm_Fb *fb;

   ob->priv.bo[ob->priv.curr] = gbm_surface_lock_front_buffer(ob->surface);
   if (!ob->priv.bo[ob->priv.curr])
     {
        WRN("Could not lock front buffer: %m");
        return;
     }

   fb = _evas_outbuf_fb_get(ob->info->info.dev, ob->priv.bo[ob->priv.curr]);
   if (fb)
     {
        ecore_drm_fb_dirty(fb, rects, count);
        ecore_drm_fb_set(ob->info->info.dev, fb);
        ecore_drm_fb_send(ob->info->info.dev, fb, _evas_outbuf_cb_pageflip, ob);
     }
}

static Eina_Bool
_evas_outbuf_make_current(void *data, void *doit)
{
   Outbuf *ob;

   if (!(ob = data)) return EINA_FALSE;

   if (doit)
     {
        if (!eglMakeCurrent(ob->egl.disp, ob->egl.surface[0],
                            ob->egl.surface[0], ob->egl.context[0]))
          return EINA_FALSE;
     }
   else
     {
        if (!eglMakeCurrent(ob->egl.disp, EGL_NO_SURFACE,
                            EGL_NO_SURFACE, EGL_NO_CONTEXT))
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_evas_outbuf_egl_setup(Outbuf *ob)
{
   int ctx_attr[3];
   int cfg_attr[40];
   int maj = 0, min = 0, n = 0;
   EGLint ncfg;
   const GLubyte *vendor, *renderer, *version, *glslversion;
   Eina_Bool blacklist = EINA_FALSE;

   /* setup gbm egl surface */
   ctx_attr[0] = EGL_CONTEXT_CLIENT_VERSION;
   ctx_attr[1] = 2;
   ctx_attr[2] = EGL_NONE;

   /* cfg_attr[n++] = EGL_BUFFER_SIZE; */
   /* cfg_attr[n++] = 32; */
   cfg_attr[n++] = EGL_SURFACE_TYPE;
   cfg_attr[n++] = EGL_WINDOW_BIT;
   cfg_attr[n++] = EGL_RENDERABLE_TYPE;
   cfg_attr[n++] = EGL_OPENGL_ES2_BIT;
#if 0
   cfg_attr[n++] = EGL_RED_SIZE;
   cfg_attr[n++] = 1;
   cfg_attr[n++] = EGL_GREEN_SIZE;
   cfg_attr[n++] = 1;
   cfg_attr[n++] = EGL_BLUE_SIZE;
   cfg_attr[n++] = 1;
#endif

   cfg_attr[n++] = EGL_ALPHA_SIZE;
   if (ob->destination_alpha) cfg_attr[n++] = 1;
   else cfg_attr[n++] = 0;
   cfg_attr[n++] = EGL_NONE;

   ob->egl.disp = eglGetDisplay((EGLNativeDisplayType)ob->info->info.gbm);
   if (ob->egl.disp  == EGL_NO_DISPLAY)
     {
        ERR("eglGetDisplay() fail. code=%#x", eglGetError());
        return EINA_FALSE;
     }

   if (!eglInitialize(ob->egl.disp, &maj, &min))
     {
        ERR("eglInitialize() fail. code=%#x", eglGetError());
        return EINA_FALSE;
     }

   eglBindAPI(EGL_OPENGL_ES_API);
   if (eglGetError() != EGL_SUCCESS)
     {
        ERR("eglBindAPI() fail. code=%#x", eglGetError());
        return EINA_FALSE;
     }

   if (!eglChooseConfig(ob->egl.disp, cfg_attr, &ob->egl.config,
                        1, &ncfg) || (ncfg != 1))
     {
        ERR("eglChooseConfig() fail. code=%#x", eglGetError());
        return EINA_FALSE;
     }

   ob->egl.surface[0] =
     eglCreateWindowSurface(ob->egl.disp, ob->egl.config,
                            (EGLNativeWindowType)ob->surface, NULL);
   if (ob->egl.surface[0] == EGL_NO_SURFACE)
     {
        ERR("eglCreateWindowSurface() fail for %p. code=%#x",
            ob->surface, eglGetError());
        return EINA_FALSE;
     }

   ob->egl.context[0] =
     eglCreateContext(ob->egl.disp, ob->egl.config, context, ctx_attr);
   if (ob->egl.context[0] == EGL_NO_CONTEXT)
     {
        ERR("eglCreateContext() fail. code=%#x", eglGetError());
        return EINA_FALSE;
     }

   if (context == EGL_NO_CONTEXT) context = ob->egl.context[0];

   if (eglMakeCurrent(ob->egl.disp, ob->egl.surface[0],
                      ob->egl.surface[0], ob->egl.context[0]) == EGL_FALSE)
     {
        ERR("eglMakeCurrent() fail. code=%#x", eglGetError());
        return EINA_FALSE;
     }

   vendor = glGetString(GL_VENDOR);
   renderer = glGetString(GL_RENDERER);
   version = glGetString(GL_VERSION);
   glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
   if (!vendor)   vendor   = (unsigned char *)"-UNKNOWN-";
   if (!renderer) renderer = (unsigned char *)"-UNKNOWN-";
   if (!version)  version  = (unsigned char *)"-UNKNOWN-";
   if (!glslversion) glslversion = (unsigned char *)"-UNKNOWN-";
   if (getenv("EVAS_GL_INFO"))
     {
        fprintf(stderr, "vendor  : %s\n", vendor);
        fprintf(stderr, "renderer: %s\n", renderer);
        fprintf(stderr, "version : %s\n", version);
        fprintf(stderr, "glsl ver: %s\n", glslversion);
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
        return EINA_FALSE;
     }

   ob->gl_context = glsym_evas_gl_common_context_new();
   if (!ob->gl_context) return EINA_FALSE;

#ifdef GL_GLES
   ob->gl_context->egldisp = ob->egl.disp;
   ob->gl_context->eglctxt = ob->egl.context[0];
#endif

   evas_outbuf_use(ob);
   glsym_evas_gl_common_context_resize(ob->gl_context,
                                       ob->w, ob->h, ob->rotation);

   ob->surf = EINA_TRUE;

   return EINA_TRUE;
}

Outbuf *
evas_outbuf_new(Evas_Engine_Info_GL_Drm *info, int w, int h, Render_Engine_Swap_Mode swap_mode)
{
   Outbuf *ob;
   char *num;

   if (!info) return NULL;

   /* try to allocate space for outbuf */
   if (!(ob = calloc(1, sizeof(Outbuf)))) return NULL;

   win_count++;

   ob->w = w;
   ob->h = h;
   ob->info = info;
   ob->depth = info->info.depth;
   ob->rotation = info->info.rotation;
   ob->destination_alpha = info->info.destination_alpha;
   /* ob->vsync = info->info.vsync; */
   ob->swap_mode = swap_mode;
   ob->priv.num = 2;

   if ((num = getenv("EVAS_GL_DRM_BUFFERS")))
     {
        ob->priv.num = atoi(num);
        if (ob->priv.num <= 0) ob->priv.num = 1;
        else if (ob->priv.num > 4) ob->priv.num = 4;
     }

   if ((num = getenv("EVAS_GL_DRM_VSYNC")))
     ob->vsync = atoi(num);

   if ((ob->rotation == 0) || (ob->rotation == 180))
     _evas_outbuf_gbm_surface_create(ob, w, h);
   else if ((ob->rotation == 90) || (ob->rotation == 270))
     _evas_outbuf_gbm_surface_create(ob, h, w);

   if (!_evas_outbuf_egl_setup(ob))
     {
        evas_outbuf_free(ob);
        return NULL;
     }

   return ob;
}

void
evas_outbuf_free(Outbuf *ob)
{
   int ref = 0;

   win_count--;
   evas_outbuf_use(ob);

   if (ob == _evas_gl_drm_window) _evas_gl_drm_window = NULL;

   if (ob->gl_context)
     {
        ref = ob->gl_context->references - 1;
        glsym_evas_gl_common_context_free(ob->gl_context);
     }

   eglMakeCurrent(ob->egl.disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

   if (ob->egl.context[0] != context)
     eglDestroyContext(ob->egl.disp, ob->egl.context[0]);

   if (ob->egl.surface[0] != EGL_NO_SURFACE)
     eglDestroySurface(ob->egl.disp, ob->egl.surface[0]);

   _evas_outbuf_gbm_surface_destroy(ob);

   if (ref == 0)
     {
        if (context) eglDestroyContext(ob->egl.disp, context);
        eglTerminate(ob->egl.disp);
        eglReleaseThread();
        context = EGL_NO_CONTEXT;
     }

   free(ob);
}

void
evas_outbuf_use(Outbuf *ob)
{
   Eina_Bool force = EINA_FALSE;

   glsym_evas_gl_preload_render_lock(_evas_outbuf_make_current, ob);

   if (_evas_gl_drm_window)
     {
        if (eglGetCurrentContext() != _evas_gl_drm_window->egl.context[0])
          force = EINA_TRUE;
     }

   if ((_evas_gl_drm_window != ob) || (force))
     {
        if (_evas_gl_drm_window)
          {
             glsym_evas_gl_common_context_use(_evas_gl_drm_window->gl_context);
             glsym_evas_gl_common_context_flush(_evas_gl_drm_window->gl_context);
          }

        _evas_gl_drm_window = ob;

        if (ob)
          {
             if (ob->egl.surface[0] != EGL_NO_SURFACE)
               {
                  if (eglMakeCurrent(ob->egl.disp, ob->egl.surface[0],
                                     ob->egl.surface[0],
                                     ob->egl.context[0]) == EGL_FALSE)
                    ERR("eglMakeCurrent() failed!");
               }
          }
     }

   if (ob) glsym_evas_gl_common_context_use(ob->gl_context);
}

void
evas_outbuf_resurf(Outbuf *ob)
{
   if (ob->surf) return;
   if (getenv("EVAS_GL_INFO")) printf("resurf %p\n", ob);

   ob->egl.surface[0] =
     eglCreateWindowSurface(ob->egl.disp, ob->egl.config,
                            (EGLNativeWindowType)ob->surface, NULL);

   if (ob->egl.surface[0] == EGL_NO_SURFACE)
     {
        ERR("eglCreateWindowSurface() fail for %p. code=%#x",
            ob->surface, eglGetError());
        return;
     }

   if (eglMakeCurrent(ob->egl.disp, ob->egl.surface[0], ob->egl.surface[0],
                      ob->egl.context[0]) == EGL_FALSE)
     ERR("eglMakeCurrent() failed!");

   ob->surf = EINA_TRUE;
}

void
evas_outbuf_unsurf(Outbuf *ob)
{
   if (!ob->surf) return;
   if (!getenv("EVAS_GL_WIN_RESURF")) return;
   if (getenv("EVAS_GL_INFO")) printf("unsurf %p\n", ob);

   if (_evas_gl_drm_window)
      glsym_evas_gl_common_context_flush(_evas_gl_drm_window->gl_context);
   if (_evas_gl_drm_window == ob)
     {
        eglMakeCurrent(ob->egl.disp, EGL_NO_SURFACE,
                       EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (ob->egl.surface[0] != EGL_NO_SURFACE)
           eglDestroySurface(ob->egl.disp, ob->egl.surface[0]);
        ob->egl.surface[0] = EGL_NO_SURFACE;

        _evas_gl_drm_window = NULL;
     }

   ob->surf = EINA_FALSE;
}

void
evas_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth)
{
   if (depth == OUTBUF_DEPTH_INHERIT) depth = ob->depth;

   /* check for changes */
   /* if ((ob->w == w) && (ob->h == h) && */
   /*     (ob->destination_alpha == ob->info->info.destination_alpha) && */
   /*     ((int)ob->rotation == rot) && (ob->depth == depth)) */
   /*   return; */

   ob->w = w;
   ob->h = h;
   ob->depth = depth;
   ob->rotation = rot;

   /* _evas_outbuf_gbm_surface_destroy(ob); */

   /* if ((ob->rotation == 0) || (ob->rotation == 180)) */
   /*   _evas_outbuf_gbm_surface_create(ob, w, h); */
   /* else if ((ob->rotation == 90) || (ob->rotation == 270)) */
   /*   _evas_outbuf_gbm_surface_create(ob, h, w); */

   evas_outbuf_use(ob);
   glsym_evas_gl_common_context_resize(ob->gl_context, w, h, rot);
}

Render_Engine_Swap_Mode
evas_outbuf_buffer_state_get(Outbuf *ob)
{
   /* check for valid output buffer */
   if (!ob) return MODE_FULL;

   if (ob->swap_mode == MODE_AUTO && _extn_have_buffer_age)
     {
        Render_Engine_Swap_Mode swap_mode;
        EGLint age = 0;

        if (!eglQuerySurface(ob->egl.disp, ob->egl.surface[0],
                             EGL_BUFFER_AGE_EXT, &age))
          age = 0;

        if (age == 1) swap_mode = MODE_COPY;
        else if (age == 2) swap_mode = MODE_DOUBLE;
        else if (age == 3) swap_mode = MODE_TRIPLE;
        else if (age == 4) swap_mode = MODE_QUADRUPLE;
        else swap_mode = MODE_FULL;
        if ((int)age != ob->priv.prev_age) swap_mode = MODE_FULL;
        ob->priv.prev_age = age;

        return swap_mode;
     }
   else
     {
        int delta;

        delta = (ob->priv.last - ob->priv.curr +
                 (ob->priv.last > ob->priv.last ?
                     0 : ob->priv.num)) % ob->priv.num;

        switch (delta)
          {
           case 0:
             return MODE_COPY;
           case 1:
             return MODE_DOUBLE;
           case 2:
             return MODE_TRIPLE;
           case 3:
             return MODE_QUADRUPLE;
           default:
             return MODE_FULL;
          }
     }

   return ob->swap_mode;
}

int
evas_outbuf_rot_get(Outbuf *ob)
{
   return ob->rotation;
}

Eina_Bool
evas_outbuf_update_region_first_rect(Outbuf *ob)
{
   /* ob->gl_context->preserve_bit = GL_COLOR_BUFFER_BIT0_QCOM; */

   glsym_evas_gl_preload_render_lock(_evas_outbuf_make_current, ob);
   evas_outbuf_use(ob);

   if (!_re_wincheck(ob)) return EINA_TRUE;

   /* glsym_evas_gl_common_context_resize(ob->gl_context, ob->w, ob->h, ob->rotation); */
   glsym_evas_gl_common_context_flush(ob->gl_context);
   glsym_evas_gl_common_context_newframe(ob->gl_context);

   return EINA_FALSE;
}

void *
evas_outbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx EINA_UNUSED, int *cy EINA_UNUSED, int *cw EINA_UNUSED, int *ch EINA_UNUSED)
{
   if ((w == ob->w) && (h == ob->h))
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
evas_outbuf_update_region_push(Outbuf *ob, RGBA_Image *update EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   /* Is it really necessary to flush per region ? Shouldn't we be able to
      still do that for the full canvas when doing partial update */
   if (!_re_wincheck(ob)) return;
   ob->drew = EINA_TRUE;
   glsym_evas_gl_common_context_flush(ob->gl_context);
}

void
evas_outbuf_update_region_free(Outbuf *ob EINA_UNUSED, RGBA_Image *update EINA_UNUSED)
{
   /* Nothing to do here as we don't really create an image per area */
}

void
evas_outbuf_flush(Outbuf *ob, Tilebuf_Rect *rects EINA_UNUSED, Evas_Render_Mode render_mode)
{
   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) goto end;

   if (!_re_wincheck(ob)) goto end;
   if (!ob->drew) goto end;

   ob->drew = EINA_FALSE;
   evas_outbuf_use(ob);
   glsym_evas_gl_common_context_done(ob->gl_context);

   if (!ob->vsync)
     {
        if (ob->info->info.vsync) eglSwapInterval(ob->egl.disp, 1);
        else eglSwapInterval(ob->egl.disp, 0);
        ob->vsync = 1;
     }

   if (ob->info->callback.pre_swap)
     ob->info->callback.pre_swap(ob->info->callback.data, ob->evas);

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
                  switch (ob->rotation)
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
             glsym_eglSwapBuffersWithDamage(ob->egl.disp, ob->egl.surface[0],
                                            result, num);
          }
     }
   else
#endif
      eglSwapBuffers(ob->egl.disp, ob->egl.surface[0]);

   if (ob->info->callback.post_swap)
     ob->info->callback.post_swap(ob->info->callback.data, ob->evas);

   //Flush GL Surface data to Framebuffer
   _evas_outbuf_buffer_swap(ob, NULL, 0);

   ob->priv.frame_cnt++;

end:
   //TODO: Need render unlock after drm page flip?
   glsym_evas_gl_preload_render_unlock(_evas_outbuf_make_current, ob);
}

Evas_Engine_GL_Context *
evas_outbuf_gl_context_get(Outbuf *ob)
{
   return ob->gl_context;
}

void *
evas_outbuf_egl_display_get(Outbuf *ob)
{
   return ob->egl.disp;
}

Context_3D *
evas_outbuf_gl_context_new(Outbuf *ob)
{
   Context_3D *ctx;
   int context_attrs[3] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

   if (!ob) return NULL;

   ctx = calloc(1, sizeof(Context_3D));
   if (!ctx) return NULL;

   ctx->context = eglCreateContext(ob->egl.disp, ob->egl.config,
                                   ob->egl.context[0], context_attrs);

   if (!ctx->context)
     {
        ERR("EGL context creation failed.");
        goto error;
     }

   ctx->display = ob->egl.disp;
   ctx->surface = ob->egl.surface[0];

   return ctx;

error:
   free(ctx);
   return NULL;
}

void
evas_outbuf_gl_context_free(Context_3D *ctx)
{
   eglDestroyContext(ctx->display, ctx->context);
   free(ctx);
}

void
evas_outbuf_gl_context_use(Context_3D *ctx)
{
   if (eglMakeCurrent(ctx->display, ctx->surface,
                      ctx->surface, ctx->context) == EGL_FALSE)
     ERR("eglMakeCurrent() failed.");
}
