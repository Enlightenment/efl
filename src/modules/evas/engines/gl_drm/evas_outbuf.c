#include "evas_engine.h"

/* local variables */
static Outbuf *_evas_gl_drm_window = NULL;
static EGLContext context = EGL_NO_CONTEXT;
static int win_count = 0;
static unsigned char gl_context_valid = 0;

#ifdef EGL_MESA_platform_gbm
static PFNEGLGETPLATFORMDISPLAYEXTPROC dlsym_eglGetPlatformDisplayEXT = NULL;
static PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC dlsym_eglCreatePlatformWindowSurfaceEXT = NULL;
#endif

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
   unsigned int format = GBM_FORMAT_XRGB8888;
   unsigned int flags = GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING;

   if (!ob) return;

   ob->surface =
     gbm_surface_create(ob->info->info.gbm, w, h, format, flags);

   if (!ob->surface) ERR("Failed to create gbm surface");
}

static void
_evas_outbuf_fb_cb_destroy(struct gbm_bo *bo EINA_UNUSED, void *data)
{
   Ecore_Drm2_Fb *fb;

   fb = data;
   if (fb) ecore_drm2_fb_discard(fb);
}

void
_evas_outbuf_release_fb(Ecore_Drm2_Fb *fb, Ecore_Drm2_Fb_Status status, void *data)
{
   struct gbm_bo *bo;
   Outbuf *ob;

   if (status != ECORE_DRM2_FB_STATUS_RELEASE) return;

   ob = data;
   bo = ecore_drm2_fb_bo_get(fb);
   if ((!ob->surface) || (!bo)) return;
   gbm_surface_release_buffer(ob->surface, bo);
}

static Ecore_Drm2_Fb *
_evas_outbuf_fb_get(Outbuf *ob, struct gbm_bo *bo)
{
   Ecore_Drm2_Fb *fb;
   uint32_t format, hdl, stride;
   int w, h;

   fb = gbm_bo_get_user_data(bo);
   if (fb) return fb;

   format = gbm_bo_get_format(bo);
   w = gbm_bo_get_width(bo);
   h = gbm_bo_get_height(bo);
   hdl = gbm_bo_get_handle(bo).u32;
   stride = gbm_bo_get_stride(bo);
   /* fb->size = fb->stride * fb->h; */

   fb =
     ecore_drm2_fb_gbm_create(ob->dev, w, h, ob->depth, ob->bpp,
                              format, hdl, stride, bo);
   if (!fb)
     {
        ERR("Failed to create FBO");
        return NULL;
     }

   ecore_drm2_fb_status_handler_set(fb, _evas_outbuf_release_fb, ob);

   gbm_bo_set_user_data(bo, fb, _evas_outbuf_fb_cb_destroy);

   return fb;
}

static void
_evas_outbuf_buffer_swap(Outbuf *ob)
{
   struct gbm_bo *bo;
   Ecore_Drm2_Fb *fb = NULL;

   bo = gbm_surface_lock_front_buffer(ob->surface);
   if (!bo)
     {
        ecore_drm2_fb_release(ob->priv.output, EINA_TRUE);
        bo = gbm_surface_lock_front_buffer(ob->surface);
     }
   if (bo) fb = _evas_outbuf_fb_get(ob, bo);

   if (fb)
     {
        if (!ob->priv.plane)
          ob->priv.plane = ecore_drm2_plane_assign(ob->priv.output, fb, 0, 0);
        else ecore_drm2_plane_fb_set(ob->priv.plane, fb);

        ecore_drm2_fb_flip(fb, ob->priv.output);
     }
   else
     WRN("Could not get FBO from Bo");
}

static Eina_Bool
_evas_outbuf_make_current(void *data, void *doit)
{
   Outbuf *ob;

   if (!(ob = data)) return EINA_FALSE;

   if (doit)
     {
        if (!eglMakeCurrent(ob->egl.disp, ob->egl.surface,
                            ob->egl.surface, ob->egl.context))
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
_evas_outbuf_init(void)
{
   static int _init = 0;

   if (_init) return EINA_TRUE;
#ifdef EGL_MESA_platform_gbm
   {
     const char *exts;

     exts = eglQueryString(NULL, EGL_EXTENSIONS);
     if (_ckext(exts, "EGL_EXT_platform_base"))
       {
          dlsym_eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)
                eglGetProcAddress("eglGetPlatformDisplayEXT");
          dlsym_eglCreatePlatformWindowSurfaceEXT = (PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC)
                eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT");
       }
   }
#endif
   _init = 1;
   return EINA_TRUE;
}

static Eina_Bool
_evas_outbuf_egl_setup(Outbuf *ob)
{
   const char *exts;
   int ctx_attr[5];
   int cfg_attr[40];
   int maj = 0, min = 0, n = 0, i = 0, cn = 0;
   EGLint ncfg = 0;
   EGLConfig *cfgs;
   const GLubyte *vendor, *renderer, *version, *glslversion;
   Eina_Bool blacklist = EINA_FALSE;

   if (!_evas_outbuf_init())
     {
        ERR("Could not initialize engine!");
        return EINA_FALSE;
     }

   /* setup gbm egl surface */
   ctx_attr[cn++] = EGL_CONTEXT_CLIENT_VERSION;
   ctx_attr[cn++] = 2;

   if (_extn_have_context_priority)
     {
        ctx_attr[cn++] = EGL_CONTEXT_PRIORITY_LEVEL_IMG;
        ctx_attr[cn++] = EGL_CONTEXT_PRIORITY_HIGH_IMG;
     }
   ctx_attr[cn++] = EGL_NONE;

   cfg_attr[n++] = EGL_RENDERABLE_TYPE;
   cfg_attr[n++] = EGL_OPENGL_ES2_BIT;
   cfg_attr[n++] = EGL_SURFACE_TYPE;
   cfg_attr[n++] = EGL_WINDOW_BIT;

   cfg_attr[n++] = EGL_RED_SIZE;
   cfg_attr[n++] = 1;
   cfg_attr[n++] = EGL_GREEN_SIZE;
   cfg_attr[n++] = 1;
   cfg_attr[n++] = EGL_BLUE_SIZE;
   cfg_attr[n++] = 1;


   cfg_attr[n++] = EGL_ALPHA_SIZE;
   if (ob->destination_alpha) cfg_attr[n++] = 1;
   else cfg_attr[n++] = 0;
   cfg_attr[n++] = EGL_NONE;

   ob->egl.disp = EGL_NO_DISPLAY;
#ifdef EGL_MESA_platform_gbm
   if (dlsym_eglGetPlatformDisplayEXT)
     ob->egl.disp = dlsym_eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_MESA,
                                                   ob->info->info.gbm,
                                                   NULL);
#endif
   if (ob->egl.disp == EGL_NO_DISPLAY)
     ob->egl.disp = eglGetDisplay((EGLNativeDisplayType)ob->info->info.gbm);
   if (ob->egl.disp == EGL_NO_DISPLAY)
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

   eng_egl_symbols(ob->egl.disp);

   if (!eglGetConfigs(ob->egl.disp, NULL, 0, &ncfg) || (ncfg == 0))
     {
        ERR("eglGetConfigs() fail. code=%#x", eglGetError());
        return EINA_FALSE;
     }

   cfgs = malloc(ncfg * sizeof(EGLConfig));
   if (!cfgs)
     {
        ERR("Failed to malloc space for egl configs");
        return EINA_FALSE;
     }

   if (!eglChooseConfig(ob->egl.disp, cfg_attr, cfgs,
                        ncfg, &ncfg) || (ncfg == 0))
     {
        ERR("eglChooseConfig() fail. code=%#x", eglGetError());
        goto err;
     }

   for (; i < ncfg; ++i)
     {
        EGLint format = 0;

        if (!eglGetConfigAttrib(ob->egl.disp, cfgs[i], EGL_NATIVE_VISUAL_ID,
                                &format))
          {
             ERR("eglGetConfigAttrib() fail. code=%#x", eglGetError());
             goto err;
          }

        if (format == (int)ob->info->info.format)
          {
             ob->egl.config = cfgs[i];
             break;
          }
     }

   ob->egl.surface = EGL_NO_SURFACE;
#ifdef EGL_MESA_platform_gbm
   if (dlsym_eglCreatePlatformWindowSurfaceEXT)
     ob->egl.surface =
       dlsym_eglCreatePlatformWindowSurfaceEXT(ob->egl.disp, ob->egl.config,
                                               ob->surface, NULL);
#endif
   if (ob->egl.surface == EGL_NO_SURFACE)
     ob->egl.surface = eglCreateWindowSurface(ob->egl.disp, ob->egl.config,
                                              (EGLNativeWindowType)ob->surface,
                                              NULL);
   if (ob->egl.surface == EGL_NO_SURFACE)
     {
        ERR("eglCreateWindowSurface() fail for %p. code=%#x",
            ob->surface, eglGetError());
        goto err;
     }

   ob->egl.context =
     eglCreateContext(ob->egl.disp, ob->egl.config, context, ctx_attr);
   if (ob->egl.context == EGL_NO_CONTEXT)
     {
        ERR("eglCreateContext() fail. code=%#x", eglGetError());
        goto err;
     }

   if (context == EGL_NO_CONTEXT) context = ob->egl.context;

   if (eglMakeCurrent(ob->egl.disp, ob->egl.surface,
                      ob->egl.surface, ob->egl.context) == EGL_FALSE)
     {
        ERR("eglMakeCurrent() fail. code=%#x", eglGetError());
        goto err;
     }

   exts = eglQueryString(ob->egl.disp, EGL_EXTENSIONS);
   glsym_evas_gl_symbols(glsym_eglGetProcAddress, exts);

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
        goto err;
     }

   ob->gl_context = glsym_evas_gl_common_context_new();
   if (!ob->gl_context) goto err;

#ifdef GL_GLES
   ob->gl_context->egldisp = ob->egl.disp;
   ob->gl_context->eglctxt = ob->egl.context;
#endif

   evas_outbuf_use(ob);
   glsym_evas_gl_common_context_resize(ob->gl_context,
                                       ob->w, ob->h, ob->rotation);

   ob->surf = EINA_TRUE;

   free(cfgs);
   return EINA_TRUE;

err:
   free(cfgs);
   return EINA_FALSE;
}

Outbuf *
evas_outbuf_new(Evas_Engine_Info_GL_Drm *info, int w, int h, Render_Output_Swap_Mode swap_mode)
{
   Outbuf *ob;

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

   ob->dev = info->info.dev;
   ob->bpp = info->info.bpp;
   ob->format = info->info.format;
   ob->priv.output = info->info.output;

   /* if ((num = getenv("EVAS_GL_DRM_VSYNC"))) */
   /*   ob->vsync = atoi(num); */

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

   if (win_count == 0) evas_common_font_ext_clear();

   if (ob == _evas_gl_drm_window) _evas_gl_drm_window = NULL;

   if (ob->gl_context)
     {
        ref = ob->gl_context->references - 1;
        glsym_evas_gl_common_context_free(ob->gl_context);
     }

   eglMakeCurrent(ob->egl.disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

   if (ob->egl.context != context)
     eglDestroyContext(ob->egl.disp, ob->egl.context);

   if (ob->egl.surface != EGL_NO_SURFACE)
     eglDestroySurface(ob->egl.disp, ob->egl.surface);

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

   if ((_evas_gl_drm_window) && (!gl_context_valid))
     {
        if (eglGetCurrentContext() != _evas_gl_drm_window->egl.context)
          force = EINA_TRUE;
        gl_context_valid = 1;
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
             if (ob->egl.surface != EGL_NO_SURFACE)
               {
                  if (eglMakeCurrent(ob->egl.disp, ob->egl.surface,
                                     ob->egl.surface,
                                     ob->egl.context) == EGL_FALSE)
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

   ob->egl.surface =
     eglCreateWindowSurface(ob->egl.disp, ob->egl.config,
                            (EGLNativeWindowType)ob->surface, NULL);

   if (ob->egl.surface == EGL_NO_SURFACE)
     {
        ERR("eglCreateWindowSurface() fail for %p. code=%#x",
            ob->surface, eglGetError());
        return;
     }

   if (eglMakeCurrent(ob->egl.disp, ob->egl.surface, ob->egl.surface,
                      ob->egl.context) == EGL_FALSE)
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
        if (ob->egl.surface != EGL_NO_SURFACE)
           eglDestroySurface(ob->egl.disp, ob->egl.surface);
        ob->egl.surface = EGL_NO_SURFACE;

        _evas_gl_drm_window = NULL;
     }

   ob->surf = EINA_FALSE;
}

void
evas_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth)
{
   if (depth == OUTBUF_DEPTH_INHERIT) depth = ob->depth;

   while (ecore_drm2_fb_release(ob->priv.output, EINA_TRUE));

   _evas_outbuf_gbm_surface_destroy(ob);
   if ((ob->rotation == 0) || (ob->rotation == 180))
     _evas_outbuf_gbm_surface_create(ob, w, h);
   else if ((ob->rotation == 90) || (ob->rotation == 270))
     _evas_outbuf_gbm_surface_create(ob, h, w);
   _evas_outbuf_egl_setup(ob);

   glsym_evas_gl_common_context_resize(ob->gl_context, w, h, rot);
}

Render_Output_Swap_Mode
evas_outbuf_buffer_state_get(Outbuf *ob)
{
   /* check for valid output buffer */
   if (!ob) return MODE_FULL;

   ecore_drm2_fb_release(ob->priv.output, EINA_FALSE);

   if (ob->swap_mode == MODE_AUTO && _extn_have_buffer_age)
     {
        Render_Output_Swap_Mode swap_mode;
        EGLint age = 0;

        eina_evlog("+gl_query_surf_swap_mode", ob, 0.0, NULL);
        if (!eglQuerySurface(ob->egl.disp, ob->egl.surface,
                             EGL_BUFFER_AGE_EXT, &age))
          age = 0;

        if (age == 1) swap_mode = MODE_COPY;
        else if (age == 2) swap_mode = MODE_DOUBLE;
        else if (age == 3) swap_mode = MODE_TRIPLE;
        else if (age == 4) swap_mode = MODE_QUADRUPLE;
        else swap_mode = MODE_FULL;
        if ((int)age != ob->prev_age)
          {
             char buf[16];
             snprintf(buf, sizeof(buf), "! %i", (int)age);
             eina_evlog("!gl_buffer_age", ob, 0.0, buf);
          }
        else
          {
             char buf[16];
             snprintf(buf, sizeof(buf), "%i", (int)age);
             eina_evlog("!gl_buffer_age", ob, 0.0, buf);
          }
        ob->prev_age = age;

        eina_evlog("-gl_query_surf_swap_mode", ob, 0.0, NULL);
        return swap_mode;
     }
   else
     return MODE_FULL;

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

static void
_glcoords_convert(int *result, Outbuf *ob, int x, int y, int w, int h)
{
   switch (ob->rotation)
     {
      case 0:
        result[0] = x;
        result[1] = ob->gl_context->h - (y + h);
        result[2] = w;
        result[3] = h;
        break;
      case 90:
        result[0] = y;
        result[1] = x;
        result[2] = h;
        result[3] = w;
        break;
      case 180:
        result[0] = ob->gl_context->w - (x + w);
        result[1] = y;
        result[2] = w;
        result[3] = h;
        break;
      case 270:
        result[0] = ob->gl_context->h - (y + h);
        result[1] = ob->gl_context->w - (x + w);
        result[2] = h;
        result[3] = w;
        break;
      default:
        result[0] = x;
        result[1] = ob->gl_context->h - (y + h);
        result[2] = w;
        result[3] = h;
        break;
     }
}

void
evas_outbuf_damage_region_set(Outbuf *ob, Tilebuf_Rect *damage)
{
   if (glsym_eglSetDamageRegionKHR)
     {
        Tilebuf_Rect *tr;
        int *rect, *rects, count;

        count = eina_inlist_count(EINA_INLIST_GET(damage));
        rects = alloca(sizeof(int) * 4 * count);
        rect = rects;
        EINA_INLIST_FOREACH(damage, tr)
          {
             _glcoords_convert(rect, ob, tr->x, tr->y, tr->w, tr->h);
             rect += 4;
          }
        glsym_eglSetDamageRegionKHR(ob->egl.disp, ob->egl.surface, rects, count);
     }
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
evas_outbuf_flush(Outbuf *ob, Tilebuf_Rect *surface_damage, Tilebuf_Rect *buffer_damage EINA_UNUSED, Evas_Render_Mode render_mode)
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

   /* if (ob->info->callback.pre_swap) */
   /*   ob->info->callback.pre_swap(ob->info->callback.data, ob->evas); */

   if ((glsym_eglSwapBuffersWithDamage) && (surface_damage) &&
       (ob->swap_mode != MODE_FULL))
     {
        EGLint num = 0, *result = NULL, i = 0;
        Tilebuf_Rect *r;

        // if partial swaps can be done use surface_damage
        num = eina_inlist_count(EINA_INLIST_GET(surface_damage));
        if (num > 0)
          {
             result = alloca(sizeof(EGLint) * 4 * num);
             EINA_INLIST_FOREACH(EINA_INLIST_GET(surface_damage), r)
               {
                  _glcoords_convert(&result[i], ob, r->x, r->y, r->w, r->h);
                  i += 4;
               }
             glsym_eglSwapBuffersWithDamage(ob->egl.disp, ob->egl.surface,
                                            result, num);
          }
     }
   else
      eglSwapBuffers(ob->egl.disp, ob->egl.surface);

   /* if (ob->info->callback.post_swap) */
   /*   ob->info->callback.post_swap(ob->info->callback.data, ob->evas); */

   _evas_outbuf_buffer_swap(ob);

end:
   //TODO: Need render unlock after drm page flip?
   glsym_evas_gl_preload_render_unlock(_evas_outbuf_make_current, ob);
   gl_context_valid = 0;
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
                                   ob->egl.context, context_attrs);

   if (!ctx->context)
     {
        ERR("EGL context creation failed.");
        goto error;
     }

   ctx->display = ob->egl.disp;
   ctx->surface = ob->egl.surface;

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
