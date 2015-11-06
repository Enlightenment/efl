#include "evas_engine.h"
#include "../gl_common/evas_gl_define.h"

#define SET_RESTORE_CONTEXT() \
   do { \
      if (glsym_evas_gl_context_restore_set) \
        glsym_evas_gl_context_restore_set(EINA_TRUE); } while(0)

static Eina_TLS _outbuf_key = 0;
static Eina_TLS _context_key = 0;

typedef EGLContext GLContext;
typedef EGLConfig GLConfig;
static int gles3_supported = -1;

static int _win_count = 0;
static Eina_Bool initted = EINA_FALSE;

static Eina_Bool
_eng_init(void)
{
   if (initted) return EINA_TRUE;

   if (!eina_tls_new(&_outbuf_key)) goto err;
   if (!eina_tls_new(&_context_key)) goto err;

   eina_tls_set(_outbuf_key, NULL);
   eina_tls_set(_context_key, NULL);

   initted = EINA_TRUE;
   return EINA_TRUE;

err:
   ERR("Could not create TLS key");
   return EINA_FALSE;
}

static inline Outbuf *
_tls_outbuf_get(void)
{
   if (!initted) _eng_init();
   return eina_tls_get(_outbuf_key);
}

static inline Eina_Bool
_tls_outbuf_set(Outbuf *ob)
{
   if (!initted) _eng_init();
   return eina_tls_set(_outbuf_key, ob);
}

static inline GLContext
_tls_context_get(void)
{
   if (!initted) _eng_init();
   return eina_tls_get(_context_key);
}

static inline Eina_Bool
_tls_context_set(GLContext ctx)
{
   if (!initted) _eng_init();
   return eina_tls_set(_context_key, ctx);
}

static Eina_Bool
_egl_cfg_setup(Outbuf *ob)
{
   EGLDisplay *disp;
   EGLConfig cfgs[200];
   int maj, min, i, num, cfg_attrs[40];
   Eina_Bool found = EINA_FALSE;

   disp = eglGetDisplay((EGLNativeDisplayType)ob->info->info.gbm);;
   if (!disp)
     {
        ERR("eglGetDisplay() Failed. Code=%#x", eglGetError());
        return EINA_FALSE;
     }

   ob->egl.disp = disp;

   if (!eglInitialize(disp, &maj, &min))
     {
        ERR("eglInitialize() Failed. Code=%#x", eglGetError());
        return EINA_FALSE;
     }

   if (!eglBindAPI(EGL_OPENGL_ES_API))
     {
        ERR("eglBindAPI() Failed. Code=%#x", eglGetError());
        return EINA_FALSE;
     }

   if (gles3_supported == -1)
     {
        const char *exts, *s;

        exts = eglQueryString(disp, EGL_EXTENSIONS);
        if ((exts) && (strstr(exts, "EGL_KHR_create_context")))
          {
             int k, ncfg, val;
             EGLConfig *eglcfgs;

             if ((eglGetConfigs(disp, NULL, 0, &ncfg)) && (ncfg > 0))
               {
                  eglcfgs = alloca(ncfg * sizeof(EGLConfig));
                  eglGetConfigs(disp, eglcfgs, ncfg, &ncfg);
                  for (k = 0; k < ncfg; k++)
                    {
                       val = 0;
                       if ((eglGetConfigAttrib(disp, eglcfgs[k],
                                               EGL_RENDERABLE_TYPE, &val)) &&
                           ((val & EGL_OPENGL_ES3_BIT_KHR) != 0) &&
                           (eglGetConfigAttrib(disp, eglcfgs[k],
                                               EGL_SURFACE_TYPE, &val)) &&
                           ((val & EGL_WINDOW_BIT) != 0))
                         {
                            INF("OpenGL ES 3.x is supported");
                            gles3_supported = EINA_TRUE;
                            break;
                         }
                    }
               }
          }

        if ((gles3_supported) &&
            ((s = getenv("EVAS_GL_DISABLE_GLES3")) && (atoi(s) == 1)))
          {
             INF("Disabling OpenGL ES 3.x support");
             gles3_supported = EINA_FALSE;
          }
     }

   ob->egl.gles3 = gles3_supported;

   i = 0;
   cfg_attrs[i++] = EGL_SURFACE_TYPE;
   cfg_attrs[i++] = EGL_WINDOW_BIT;
   cfg_attrs[i++] = EGL_RENDERABLE_TYPE;
   if (gles3_supported)
     cfg_attrs[i++] = EGL_OPENGL_ES3_BIT_KHR;
   else
     cfg_attrs[i++] = EGL_OPENGL_ES2_BIT;

   cfg_attrs[i++] = EGL_ALPHA_SIZE;
   if (ob->destination_alpha)
     cfg_attrs[i++] = 1;
   else
     cfg_attrs[i++] = 0;

   cfg_attrs[i++] = EGL_NONE;
   num = 0;

   if ((!eglChooseConfig(disp, cfg_attrs, cfgs, 200, &num)) || (num < 1))
     {
        ERR("eglChooseConfig cannot find any configs (gles%d, alpha %d",
            gles3_supported ? 3 : 2, ob->destination_alpha);
     }

   for (i = 0; (i < num) && (!found); i++)
     {
        EGLint format;

        if (!eglGetConfigAttrib(disp, cfgs[i], EGL_NATIVE_VISUAL_ID, &format))
          continue;

        if (format == (int)ob->info->info.format)
          {
             found = EINA_TRUE;
             ob->egl.config = cfgs[i];
             break;
          }
     }

   if (found) return EINA_TRUE;
   return EINA_FALSE;
}

static Eina_Bool
_egl_ctx_setup(Outbuf *ob)
{
   GLContext ctx;
   int ctx_attrs[3];

try_gles2:
   ctx_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   ctx_attrs[1] = ob->egl.gles3 ? 3 : 2;
   ctx_attrs[2] = EGL_NONE;

   ctx = _tls_context_get();
   ob->egl.context[0] =
     eglCreateContext(ob->egl.disp, ob->egl.config, ctx, ctx_attrs);
   if (ob->egl.context[0] == EGL_NO_CONTEXT)
     {
        ERR("eglCreateContext() Failed. Code=%#x", eglGetError());
        if (ob->egl.gles3)
          {
             ERR("Trying again with an OpenGL ES 2 context");
             ob->egl.gles3 = EINA_FALSE;
             goto try_gles2;
          }

        return EINA_FALSE;
     }

   if (ctx == EGL_NO_CONTEXT) _tls_context_set(ob->egl.context[0]);

   SET_RESTORE_CONTEXT();

   if (eglMakeCurrent(ob->egl.disp, ob->egl.surface[0],
                      ob->egl.surface[0], ob->egl.context[0]) == EGL_FALSE)
     {
        ERR("eglMakeCurrent() Failed. Code=%#x", eglGetError());
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Outbuf *
evas_outbuf_new(Evas_Engine_Info_GL_Drm *info, int w, int h, Render_Engine_Swap_Mode swap_mode)
{
   Outbuf *ob;
   const GLubyte *vendor, *renderer, *version, *glslversion;
   int blacklist = 0, i = 0;
   char *num;

   ob = calloc(1, sizeof(Outbuf));
   if (!ob) return NULL;

   ob->w = w;
   ob->h = h;
   ob->info = info;
   ob->depth = info->info.depth;
   ob->rotation = info->info.rotation;
   ob->destination_alpha = info->info.destination_alpha;
   ob->swap_mode = swap_mode;
   ob->priv.num = 2;

   if ((num = getenv("EVAS_GL_DRM_BUFFERS")))
     {
        ob->priv.num = atoi(num);
        if (ob->priv.num <= 0) ob->priv.num = 1;
        else if (ob->priv.num > 4) ob->priv.num = 4;
     }

   ob->surface =
     gbm_surface_create(info->info.gbm, w, h,
                        info->info.format, info->info.flags);
   if (!ob->surface)
     {
        ERR("Failed to create gbm surface: %m");
        evas_outbuf_free(ob);
        return NULL;
     }

   if (!_egl_cfg_setup(ob))
     {
        evas_outbuf_free(ob);
        return NULL;
     }

   ob->egl.surface[0] =
     eglCreateWindowSurface(ob->egl.disp, ob->egl.config,
                            (EGLNativeWindowType)ob->surface, NULL);
   if (ob->egl.surface[0] == EGL_NO_SURFACE)
     {
        int err;

        err = eglGetError();
        ERR("eglCreateWindowSurface() Failed. Code=%#x", err);
        evas_outbuf_free(ob);
        return NULL;
     }

   if (!_egl_ctx_setup(ob))
     {
        evas_outbuf_free(ob);
        return NULL;
     }

   /* for (; i < ob->priv.num; i++) */
   /*   { */
   /*      ob->priv.bo[i] = */
   /*        gbm_bo_create(info->info.gbm, w, h, info->info.format, */
   /*                      info->info.flags); */
   /*      if (!ob->priv.bo[i]) */
   /*        { */
   /*           ERR("Failed to create gbm bo: %m"); */
   /*           break; */
   /*        } */
   /*   } */

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
          blacklist = 1;
     }
   if (strstr((const char *)renderer, "softpipe"))
     blacklist = 1;
   if (strstr((const char *)renderer, "llvmpipe"))
     blacklist = 1;
   if ((blacklist) && (!getenv("EVAS_GL_NO_BLACKLIST")))
     {
        WRN("OpenGL Driver blacklisted:");
        WRN("Vendor: %s", (const char *)vendor);
        WRN("Renderer: %s", (const char *)renderer);
        WRN("Version: %s", (const char *)version);
        evas_outbuf_free(ob);
        return NULL;
     }

   eng_gl_symbols();
   ob->gl_context = glsym_evas_gl_common_context_new();
   if (!ob->gl_context)
     {
        ERR("Unable to get a new context");
        evas_outbuf_free(ob);
        return NULL;
     }

   ob->gl_context->egldisp = ob->egl.disp;
   ob->gl_context->eglctxt = ob->egl.context[0];

   evas_outbuf_use(ob);
   glsym_evas_gl_common_context_resize(ob->gl_context, w, h, ob->rotation);
   ob->surf = EINA_TRUE;

   return ob;
}

void
evas_outbuf_free(Outbuf *ob)
{
   Outbuf *tob;
   GLContext context;
   int ref = 0;

   _win_count--;
   evas_outbuf_use(ob);

   context = _tls_context_get();
   tob = _tls_outbuf_get();

   if (ob == tob) _tls_outbuf_set(NULL);
   if (ob->gl_context)
     {
        ref = ob->gl_context->references - 1;
        glsym_evas_gl_common_context_free(ob->gl_context);
     }

   SET_RESTORE_CONTEXT();

   eglMakeCurrent(ob->egl.disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

   if (ob->egl.surface[0] != EGL_NO_SURFACE)
     eglDestroySurface(ob->egl.disp, ob->egl.surface[0]);
   if (ob->egl.surface[1] != EGL_NO_SURFACE)
     eglDestroySurface(ob->egl.disp, ob->egl.surface[1]);

   if (ob->egl.context[0] != context)
     eglDestroyContext(ob->egl.disp, ob->egl.context[0]);

   if (ob->surface) gbm_surface_destroy(ob->surface);

   if (ref == 0)
     {
        if (context) eglDestroyContext(ob->egl.disp, context);
        eglTerminate(ob->egl.disp);
        eglReleaseThread();
        _tls_context_set(EGL_NO_CONTEXT);
     }

   free(ob);
}

static Eina_Bool
_evas_outbuf_make_current(void *data, void *doit)
{
   Outbuf *ob;

   ob = data;
   SET_RESTORE_CONTEXT();
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

void
evas_outbuf_use(Outbuf *ob)
{
   Eina_Bool force = EINA_FALSE;
   Outbuf *tob;

   tob = _tls_outbuf_get();

   glsym_evas_gl_preload_render_lock(_evas_outbuf_make_current, ob);
   if (tob)
     {
        if ((eglGetCurrentDisplay() != tob->egl.disp) ||
            (eglGetCurrentContext() != tob->egl.context[0]))
          force = EINA_TRUE;
     }

   if ((tob != ob) || (force))
     {
        if (tob)
          {
             glsym_evas_gl_common_context_use(tob->gl_context);
             glsym_evas_gl_common_context_flush(tob->gl_context);
          }

        _tls_outbuf_set(ob);

        if (ob)
          {
             if (ob->egl.surface[0] != EGL_NO_SURFACE)
               {
                  SET_RESTORE_CONTEXT();
                  if (eglMakeCurrent(ob->egl.disp, ob->egl.surface[0],
                                     ob->egl.surface[0],
                                     ob->egl.context[0]) == EGL_FALSE)
                    ERR("eglMakeCurrent() Failed. Code=%#x", eglGetError());
               }
          }
     }

   if (ob) glsym_evas_gl_common_context_use(ob->gl_context);
}

void
evas_outbuf_resurf(Outbuf *ob)
{
   if (ob->surf) return;
   ob->egl.surface[0] =
     eglCreateWindowSurface(ob->egl.disp, ob->egl.config,
                            (EGLNativeWindowType)ob->surface, NULL);
   if (ob->egl.surface[0] == EGL_NO_SURFACE)
     {
        ERR("eglCreateWindowSurface() Failed. Code=%#x", eglGetError());
        return;
     }

   SET_RESTORE_CONTEXT();

   if (eglMakeCurrent(ob->egl.disp, ob->egl.surface[0],
                      ob->egl.surface[0], ob->egl.context[0]) == EGL_FALSE)
     {
        ERR("eglMakeCurrent() Failed. Code=%#x", eglGetError());
     }

   ob->surf = EINA_TRUE;
}

void
evas_outbuf_unsurf(Outbuf *ob)
{
   Outbuf *tob;

   if (!ob->surf) return;
   if (!getenv("EVAS_GL_WIN_RESURF")) return;

   tob = _tls_outbuf_get();
   if (tob) glsym_evas_gl_common_context_flush(tob->gl_context);
   if (tob == ob)
     {
        SET_RESTORE_CONTEXT();
        eglMakeCurrent(ob->egl.disp, EGL_NO_SURFACE, EGL_NO_SURFACE,
                       EGL_NO_CONTEXT);
        if (ob->egl.surface[0] != EGL_NO_SURFACE)
          eglDestroySurface(ob->egl.disp, ob->egl.surface[0]);
        ob->egl.surface[0] = EGL_NO_SURFACE;
        if (ob->egl.surface[1] != EGL_NO_SURFACE)
          eglDestroySurface(ob->egl.disp, ob->egl.surface[1]);
        ob->egl.surface[1] = EGL_NO_SURFACE;
        _tls_outbuf_set(NULL);
     }
   ob->surf = EINA_FALSE;
}

void
evas_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth EINA_UNUSED)
{
   Evas_Public_Data *epd;
   Evas_Engine_Info_GL_Drm *einfo;
   Render_Engine *re;
   struct gbm_surface *osurface;
   Outbuf *nob;

   if (depth == OUTBUF_DEPTH_INHERIT) depth = ob->depth;

   epd = eo_data_scope_get(ob->evas, EVAS_CANVAS_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(epd);

   re = epd->engine.data.output;
   EINA_SAFETY_ON_NULL_RETURN(re);

   einfo = ob->info;
   osurface = ob->surface;

   if ((ob->rotation == 0) || (ob->rotation == 180))
     nob = evas_outbuf_new(einfo, w, h, ob->swap_mode);
   else
     nob = evas_outbuf_new(einfo, h, w, ob->swap_mode);

   if (!nob)
     {
        ERR("Could not create new Outbuf");
        return;
     }

   re->generic.software.ob->gl_context->references++;

   evas_outbuf_free(ob);
   re->generic.software.ob = NULL;

   evas_outbuf_use(nob);

   evas_render_engine_software_generic_update(&re->generic.software, nob, w, h);

   re->generic.software.ob->gl_context->references--;

   glsym_evas_gl_common_context_resize(nob->gl_context, w, h, rot);

   /* ob->w = w; */
   /* ob->h = h; */
   /* ob->rotation = rot; */

   /* if (ob->surface) gbm_surface_destroy(ob->surface); */

   /* ob->surface = */
   /*   gbm_surface_create(ob->info->info.gbm, w, h, */
   /*                      ob->info->info.format, ob->info->info.flags); */

   /* evas_outbuf_use(ob); */
   /* glsym_evas_gl_common_context_resize(ob->gl_context, w, h, rot); */
}

Render_Engine_Swap_Mode
evas_outbuf_buffer_state_get(Outbuf *ob)
{
   if ((ob->swap_mode == MODE_AUTO) && _extn_have_buffer_age)
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
   ob->gl_context->preserve_bit = GL_COLOR_BUFFER_BIT0_QCOM;

   glsym_evas_gl_preload_render_lock(_evas_outbuf_make_current, ob);

   evas_outbuf_use(ob);
   if (!_re_wincheck(ob)) return EINA_TRUE;

   glsym_evas_gl_common_context_resize(ob->gl_context, ob->w, ob->h,
                                       ob->rotation);
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

static void
_damage_rect_set(Outbuf *ob, int x, int y, int w, int h)
{
   int rects[4];

   if ((x == 0) && (y == 0) &&
       (((w == ob->gl_context->w) && (h == ob->gl_context->h)) ||
           ((h == ob->gl_context->w) && (w == ob->gl_context->h))))
     return;

   _glcoords_convert(rects, ob, x, y, w, h);
   glsym_eglSetDamageRegionKHR(ob->egl.disp, ob->egl.surface[0], rects, 1);
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

        if (glsym_eglSetDamageRegionKHR)
          _damage_rect_set(ob, x, y, w, h);
     }

   return ob->gl_context->def_surface;
}

void
evas_outbuf_update_region_push(Outbuf *ob, RGBA_Image *update EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   if (!_re_wincheck(ob)) return;
   ob->drew = EINA_TRUE;
   glsym_evas_gl_common_context_flush(ob->gl_context);
}

void
evas_outbuf_update_region_free(Outbuf *ob EINA_UNUSED, RGBA_Image *update EINA_UNUSED)
{

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

   fb = gbm_bo_get_user_data(bo);
   if (fb) fb->pending_flip = EINA_FALSE;

   /* fb = _evas_outbuf_fb_get(ob->info->info.dev, bo); */
   /* if (fb) fb->pending_flip = EINA_FALSE; */

   gbm_surface_release_buffer(ob->surface, bo);

   ob->priv.last = ob->priv.curr;
   ob->priv.curr = (ob->priv.curr + 1) % ob->priv.num;
}
static void
_evas_outbuf_buffer_swap(Outbuf *ob)
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
        ecore_drm_fb_dirty(fb, NULL, 0);
        ecore_drm_fb_set(ob->info->info.dev, fb);
        ecore_drm_fb_send(ob->info->info.dev, fb, _evas_outbuf_cb_pageflip, ob);
     }
}

void
evas_outbuf_flush(Outbuf *ob, Tilebuf_Rect *rects, Evas_Render_Mode render_mode)
{
   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) goto end;

   if (!_re_wincheck(ob)) goto end;
   if (!ob->drew) goto end;

   if (!gbm_surface_has_free_buffers(ob->surface))
     {
        INF("Gbm Surface has NO free buffers");
        gbm_surface_release_buffer(ob->surface, ob->priv.bo[ob->priv.curr]);
        goto end;
        /* return; */
     }

   ob->drew = EINA_FALSE;
   evas_outbuf_use(ob);
   glsym_evas_gl_common_context_done(ob->gl_context);

   if (!ob->vsync)
     {
        if (ob->info->info.vsync) eglSwapInterval(ob->egl.disp, 1);
        else eglSwapInterval(ob->egl.disp, 0);
        ob->vsync = EINA_TRUE;
     }

   if (ob->info->callback.pre_swap)
     ob->info->callback.pre_swap(ob->info->callback.data, ob->evas);

   if ((glsym_eglSwapBuffersWithDamage) && (rects) &&
       (ob->swap_mode != MODE_FULL))
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
                  _glcoords_convert(&result[i], ob, r->x, r->y, r->w, r->h);
                  i += 4;
               }
             glsym_eglSwapBuffersWithDamage(ob->egl.disp, ob->egl.surface[0],
                                            result, num);
          }
     }
   else
      eglSwapBuffers(ob->egl.disp, ob->egl.surface[0]);

   _evas_outbuf_buffer_swap(ob);

   if (ob->info->callback.post_swap)
     ob->info->callback.post_swap(ob->info->callback.data, ob->evas);

   ob->priv.frame_cnt++;

end:
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
   int ctx_attrs[3] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

   if (!ob) return NULL;

   ctx = calloc(1, sizeof(Context_3D));
   if (!ctx) return NULL;

   if (ob->egl.gles3) ctx_attrs[1] = 3;

   ctx->context =
     eglCreateContext(ob->egl.disp, ob->egl.config,
                      ob->egl.context[0], ctx_attrs);
   if (!ctx->context)
     {
        ERR("eglCreateContext() Failed. Code=%#x", eglGetError());
        goto err;
     }

   ctx->display = ob->egl.disp;
   ctx->surface = ob->egl.surface[0];

   return NULL;

err:
   free(ctx);
   return NULL;
}

void
evas_outbuf_gl_context_use(Context_3D *ctx)
{
   SET_RESTORE_CONTEXT();
   if (eglMakeCurrent(ctx->display, ctx->surface,
                      ctx->surface, ctx->context) == EGL_FALSE)
     ERR("eglMakeCurrent() Failed. Code=%#x", eglGetError());
}

void
evas_outbuf_gl_context_free(Context_3D *ctx)
{
   eglDestroyContext(ctx->display, ctx->context);
   free(ctx);
}
