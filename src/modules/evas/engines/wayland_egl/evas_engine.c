#include "evas_common_private.h"
#include "evas_engine.h"

#ifdef HAVE_DLSYM
# include <dlfcn.h>
#endif

#ifdef EVAS_CSERVE2
# include "evas_cs2_private.h"
#endif

#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

#define EVAS_GL_UPDATE_TILE_SIZE 16

#ifndef EGL_NATIVE_PIXMAP_KHR
# define EGL_NATIVE_PIXMAP_KHR 0x30b0
#endif

#ifndef EGL_Y_INVERTED_NOK
# define EGL_Y_INVERTED_NOK 0x307F
#endif

/* local structures */
typedef struct _Render_Engine Render_Engine;
struct _Render_Engine
{
   Render_Engine_GL_Generic generic;
};

typedef struct _Native Native;
struct _Native
{
   Evas_Native_Surface ns;
   void *egl_surface;
};

/* local function prototypes */
typedef void (*_eng_fn) (void);
typedef _eng_fn (*glsym_func_eng_fn) ();
typedef void (*glsym_func_void) ();
typedef void *(*glsym_func_void_ptr) ();
typedef int (*glsym_func_int) ();
typedef unsigned int (*glsym_func_uint) ();
typedef const char *(*glsym_func_const_char_ptr) ();

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

_eng_fn (*glsym_eglGetProcAddress) (const char *a) = NULL;
void *(*glsym_eglCreateImage) (EGLDisplay a, EGLContext b, EGLenum c, EGLClientBuffer d, const int *e) = NULL;
void (*glsym_eglDestroyImage) (EGLDisplay a, void *b) = NULL;
void (*glsym_glEGLImageTargetTexture2DOES) (int a, void *b)  = NULL;
unsigned int (*glsym_eglSwapBuffersWithDamage) (EGLDisplay a, void *b, const EGLint *d, EGLint c) = NULL;

/* local variables */
static int safe_native = -1;
static Eina_Bool initted = EINA_FALSE;
static int gl_wins = 0;
static Evas_Func func, pfunc;

/* external variables */
int _evas_engine_wl_egl_log_dom = -1;
Eina_Bool extn_have_buffer_age = EINA_TRUE;
Eina_Bool extn_have_y_inverted = EINA_TRUE;

/* local functions */
static inline Outbuf *
eng_get_ob(Render_Engine *re)
{
   return re->generic.software.ob;
}

static void 
gl_symbols(void)
{
   static Eina_Bool done = EINA_FALSE;

   if (done) return;

   /* FIXME: Remove this line as soon as eglGetDisplay() autodetection
    * gets fixed. Currently it is incorrectly detecting wl_display and
    * returning _EGL_PLATFORM_X11 instead of _EGL_PLATFORM_WAYLAND.
    *
    * See ticket #1972 for more info.
    */
   setenv("EGL_PLATFORM", "wayland", 1);

#define LINK2GENERIC(sym) \
   glsym_##sym = dlsym(RTLD_DEFAULT, #sym);

   // Get function pointer to evas_gl_common now provided through GL_Generic.
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

   FINDSYM(glsym_glEGLImageTargetTexture2DOES, "glEGLImageTargetTexture2DOES", 
           glsym_func_void);

   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamageEXT", 
           glsym_func_uint);
   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamageINTEL", 
           glsym_func_uint);
   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamage", 
           glsym_func_uint);

   done = EINA_TRUE;
}

static void
gl_extn_veto(Render_Engine *re)
{
   const char *str = NULL;

   str = eglQueryString(eng_get_ob(re)->egl_disp, EGL_EXTENSIONS);
   if (str)
     {
        if (getenv("EVAS_GL_INFO"))
          printf("EGL EXTN:\n%s\n", str);
        if (!strstr(str, "EGL_EXT_buffer_age"))
          {
             extn_have_buffer_age = EINA_FALSE;
          }
        if (!strstr(str, "EGL_NOK_texture_from_pixmap"))
          {
             extn_have_y_inverted = EINA_FALSE;
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
               extn_have_y_inverted = EINA_FALSE;
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
        extn_have_buffer_age = EINA_FALSE;
     }
}

static void 
_re_winfree(Render_Engine *re)
{
   Outbuf *ob;

   if (!(ob = eng_get_ob(re))) return;
   if (!ob->surf) return;
   glsym_evas_gl_preload_render_relax(eng_preload_make_current, ob);
   eng_window_unsurf(ob);
}

static void *
evgl_eng_display_get(void *data)
{
   Render_Engine *re;
   Outbuf *ob;

   if (!(re = (Render_Engine *)data)) return NULL;
   if (!(ob = eng_get_ob(re))) return NULL;
   return (void *)ob->egl_disp;
}

static void *
evgl_eng_evas_surface_get(void *data)
{
   Render_Engine *re;
   Outbuf *ob;

   if (!(re = (Render_Engine *)data)) return NULL;
   if (!(ob = eng_get_ob(re))) return NULL;
   return (void *)ob->egl_surface[0];
}

static void *
evgl_eng_native_window_create(void *data)
{
   Render_Engine *re;
   Outbuf *ob;
   struct wl_egl_window *win;

   if (!(re = (Render_Engine *)data)) return NULL;
   if (!(ob = eng_get_ob(re))) return NULL;

   if (!(win = wl_egl_window_create(ob->info->info.surface, 1, 1)))
     {
        ERR("Could not create wl_egl window: %m");
        return NULL;
     }

   return (void *)win;
}

static int 
evgl_eng_native_window_destroy(void *data, void *win)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return 0;
   if (!win) return 0;

   wl_egl_window_destroy((struct wl_egl_window *)win);
   win = NULL;

   return 1;
}

static void *
evgl_eng_window_surface_create(void *data, void *win)
{
   Render_Engine *re;
   Outbuf *ob;
   EGLSurface surface = EGL_NO_SURFACE;

   if (!(re = (Render_Engine *)data)) return NULL;
   if (!(ob = eng_get_ob(re))) return NULL;

   surface = eglCreateWindowSurface(ob->egl_disp, ob->egl_config, 
                                    (EGLNativeWindowType)win, NULL);
   if (!surface)
     {
        ERR("Could not create egl window surface: %#x", eglGetError());
        return NULL;
     }

   return (void *)surface;
}

static int 
evgl_eng_window_surface_destroy(void *data, void *surface)
{
   Render_Engine *re;
   Outbuf *ob;

   if (!(re = (Render_Engine *)data)) return 0;
   if (!(ob = eng_get_ob(re))) return 0;
   if (!surface) return 0;

   eglDestroySurface(ob->egl_disp, (EGLSurface)surface);
   return 1;
}

static void *
evgl_eng_context_create(void *data, void *ctxt)
{
   Render_Engine *re;
   Outbuf *ob;
   EGLContext context = EGL_NO_CONTEXT;
   int attrs[3];

   if (!(re = (Render_Engine *)data)) return NULL;
   if (!(ob = eng_get_ob(re))) return NULL;

   attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   attrs[1] = 2;
   attrs[2] = EGL_NONE;

   if (ctxt)
     {
        context = 
          eglCreateContext(ob->egl_disp, ob->egl_config, 
                           (EGLContext)ctxt, attrs);
     }
   else
     {
        context = 
          eglCreateContext(ob->egl_disp, ob->egl_config, 
                           ob->egl_context[0], attrs);
     }

   if (!context)
     {
        ERR("Failed to create egl context: %#x", eglGetError());
        return NULL;
     }

   return (void *)context;
}

static int 
evgl_eng_context_destroy(void *data, void *ctxt)
{
   Render_Engine *re;
   Outbuf *ob;

   if (!(re = (Render_Engine *)data)) return 0;
   if (!(ob = eng_get_ob(re))) return 0;
   if (!ctxt) return 0;

   eglDestroyContext(ob->egl_disp, (EGLContext)ctxt);
   return 1;
}

static int 
evgl_eng_make_current(void *data, void *surface, void *ctxt, int flush)
{
   Render_Engine *re;
   Outbuf *ob;
   EGLContext ctx;
   EGLSurface surf;
   int ret = 0;

   if (!(re = (Render_Engine *)data)) return 0;
   if (!(ob = eng_get_ob(re))) return 0;

   ctx = (EGLContext)ctxt;
   surf = (EGLSurface)surface;

   if ((!ctxt) && (!surface))
     {
        ret = 
          eglMakeCurrent(ob->egl_disp, EGL_NO_SURFACE, 
                         EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (!ret)
          {
             ERR("eglMakeCurrent Failed: %#x", eglGetError());
             return 0;
          }
        return 1;
     }

   if ((eglGetCurrentContext() != ctx) || 
       (eglGetCurrentSurface(EGL_READ) != surf) || 
       (eglGetCurrentSurface(EGL_DRAW) != surf))
     {
        if (flush) eng_window_use(NULL);

        ret = eglMakeCurrent(ob->egl_disp, surf, surf, ctx);
        if (!ret)
          {
             ERR("eglMakeCurrent Failed: %#x", eglGetError());
             return 0;
          }
     }

   return 1;
}

static void *
evgl_eng_proc_address_get(const char *name)
{
   if (glsym_eglGetProcAddress) return glsym_eglGetProcAddress(name);
   return dlsym(RTLD_DEFAULT, name);
}

static const char *
evgl_eng_string_get(void *data)
{
   Render_Engine *re;
   Outbuf *ob;

   if (!(re = (Render_Engine *)data)) return NULL;
   if (!(ob = eng_get_ob(re))) return NULL;

   return eglQueryString(ob->egl_disp, EGL_EXTENSIONS);
}

static int 
evgl_eng_rotation_angle_get(void *data)
{
   Render_Engine *re;
   Outbuf *ob;

   if (!(re = (Render_Engine *)data)) return 0;
   if (!(ob = eng_get_ob(re))) return 0;

   if (ob->gl_context)
     return ob->gl_context->rot;

   return 0;
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

/* engine functions */
static void *
eng_info(Evas *evas EINA_UNUSED)
{
   Evas_Engine_Info_Wayland_Egl *info;

   /* try to allocate space for our engine info */
   if (!(info = calloc(1, sizeof(Evas_Engine_Info_Wayland_Egl))))
     return NULL;

   info->magic.magic = rand();
   info->render_mode = EVAS_RENDER_MODE_BLOCKING;

   return info;
}

static void 
eng_info_free(Evas *evas EINA_UNUSED, void *info)
{
   Evas_Engine_Info_Wayland_Egl *inf;

   if ((inf = (Evas_Engine_Info_Wayland_Egl *)info))
     free(inf);
}

static int 
eng_setup(Evas *evas, void *info)
{
   Render_Engine_Swap_Mode swap_mode = MODE_FULL;
   Evas_Engine_Info_Wayland_Egl *inf;
   Evas_Public_Data *epd;
   Render_Engine *re;
   Outbuf *ob;
   const char *s;

   inf = (Evas_Engine_Info_Wayland_Egl *)info;
   epd = eo_data_scope_get(evas, EVAS_CANVAS_CLASS);

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
        switch (inf->swap_mode)
          {
           case EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_FULL:
             swap_mode = MODE_FULL;
             break;
           case EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_COPY:
             swap_mode = MODE_COPY;
             break;
           case EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_DOUBLE:
             swap_mode = MODE_DOUBLE;
             break;
           case EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_TRIPLE:
             swap_mode = MODE_TRIPLE;
             break;
           case EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_QUADRUPLE:
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

   if (!(re = epd->engine.data.output))
     {
        Render_Engine_Merge_Mode merge = MERGE_BOUNDING;

        /* FIXME: Remove this line as soon as eglGetDisplay() autodetection
         * gets fixed. Currently it is incorrectly detecting wl_display and
         * returning _EGL_PLATFORM_X11 instead of _EGL_PLATFORM_WAYLAND.
         *
         * See ticket #1972 for more info.
         */
        setenv("EGL_PLATFORM", "wayland", 1);

        /* try to allocate space for a new render engine */
        if (!(re = calloc(1, sizeof(Render_Engine))))
          return 0;

        /* if we have not initialize gl & evas, do it */
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

        ob = eng_window_new(evas, inf, epd->output.w, epd->output.h, swap_mode);
        if (!ob) goto ob_err;

        if (!evas_render_engine_gl_generic_init(&re->generic, ob, 
                                                eng_outbuf_swap_mode_get, 
                                                eng_outbuf_rotation_get, 
                                                eng_outbuf_reconfigure, 
                                                eng_outbuf_region_first_rect, 
                                                eng_outbuf_update_region_new, 
                                                eng_outbuf_update_region_push,
                                                eng_outbuf_update_region_free, 
                                                NULL, 
                                                eng_outbuf_flush, 
                                                eng_window_free, 
                                                eng_window_use, 
                                                eng_outbuf_gl_context_get, 
                                                eng_outbuf_egl_display_get, 
                                                eng_gl_context_new, 
                                                eng_gl_context_use, 
                                                &evgl_funcs, 
                                                epd->output.w, epd->output.h))
          {
             /* TODO: free Outbuf */
             goto ob_err;
          }

        epd->engine.data.output = re;
        gl_wins++;

        if ((s = getenv("EVAS_GL_PARTIAL_MERGE")))
          {
             if ((!strcmp(s, "bounding")) || (!strcmp(s, "b")))
               merge = MERGE_BOUNDING;
             else if ((!strcmp(s, "full")) || (!strcmp(s, "f")))
               merge = MERGE_FULL;
          }

        evas_render_engine_software_generic_merge_mode_set(&re->generic.software, merge);

        if (!initted)
          {
             gl_extn_veto(re);
             initted = EINA_TRUE;
          }
     }
   else
     {
        re = epd->engine.data.output;
        ob = eng_get_ob(re);
        if ((ob) && (_re_wincheck(ob)))
          {
             if ((ob->info->info.display != ob->disp) || 
                 (ob->info->info.surface != ob->surface) || 
                 (ob->info->info.win != ob->win) || 
                 (ob->info->info.depth != ob->depth) || 
                 (ob->info->info.screen != ob->screen) || 
                 (ob->info->info.destination_alpha != ob->alpha))
               {
                  ob->gl_context->references++;
                  gl_wins--;

                  ob = eng_window_new(evas, inf, epd->output.w, epd->output.h, swap_mode);
                  if (!ob) goto ob_err;

                  eng_window_free(eng_get_ob(re));
                  re->generic.software.ob = NULL;
 
                  eng_window_use(ob);

                  evas_render_engine_software_generic_update(&re->generic.software, ob, 
                                                             epd->output.w, epd->output.h);
                  gl_wins++;
                  eng_get_ob(re)->gl_context->references--;
               }
             else if ((ob->w != epd->output.w) || (ob->h != epd->output.h) || 
                      (ob->info->info.rotation != ob->rot))
               {
                  eng_outbuf_reconfigure(ob, epd->output.w, epd->output.h, 
                                         ob->info->info.rotation, 0);
                  if (re->generic.software.tb)
                    evas_common_tilebuf_free(re->generic.software.tb);
                  re->generic.software.tb = 
                    evas_common_tilebuf_new(epd->output.w, epd->output.h);
                  if (re->generic.software.tb)
                    evas_common_tilebuf_set_tile_size(re->generic.software.tb, 
                                                      TILESIZE, TILESIZE);
               }
          }
     }

   if (!eng_get_ob(re)) goto ob_err;

   if (!epd->engine.data.output)
     {
        if (eng_get_ob(re))
          {
             eng_window_free(eng_get_ob(re));
             gl_wins--;
          }
        goto ob_err;
     }

   evas_render_engine_software_generic_tile_strict_set(&re->generic.software, EINA_TRUE);

   if (!epd->engine.data.context)
     {
        epd->engine.data.context = 
          epd->engine.func->context_new(epd->engine.data.output);
     }

   eng_window_use(eng_get_ob(re));

   return 1;

ob_err:
   free(re);
   return 0;
}

static Eina_Bool 
eng_canvas_alpha_get(void *data, void *info EINA_UNUSED)
{
   Render_Engine *re;

   if ((re = (Render_Engine *)data))
     return re->generic.software.ob->alpha;

   return EINA_FALSE;
}

static void 
eng_output_free(void *data)
{
   Render_Engine *re;

   if ((re = (Render_Engine *)data))
     {
        glsym_evas_gl_preload_render_relax(eng_preload_make_current, eng_get_ob(re));

        if (gl_wins == 1) glsym_evgl_engine_shutdown(re);

        evas_render_engine_software_generic_clean(&re->generic.software);

        gl_wins--;

        free(re);
     }

   if ((initted == EINA_TRUE) && (gl_wins == 0))
     {
        glsym_evas_gl_preload_shutdown();
        evas_common_image_shutdown();
        evas_common_font_shutdown();
        initted = EINA_FALSE;
     }
}

static void 
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;
   Outbuf *ob;

   if (!(re = (Render_Engine *)data)) return;
   if (!(ob = eng_get_ob(re))) return;

   eng_window_use(ob);

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

        glsym_evas_gl_common_context_resize(ob->gl_context, w, h, ob->rot);
     }

   if (re->generic.software.tb)
     evas_common_tilebuf_free(re->generic.software.tb);

   re->generic.software.tb = evas_common_tilebuf_new(w, h);
   if (re->generic.software.tb)
     {
        evas_common_tilebuf_set_tile_size(re->generic.software.tb, 
                                          TILESIZE, TILESIZE);
        evas_common_tilebuf_tile_strict_set(re->generic.software.tb, EINA_TRUE);
     }
}

static void 
eng_output_dump(void *data)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;

   evas_common_image_image_all_unload();
   evas_common_font_font_all_unload();
   glsym_evas_gl_common_image_all_unload(eng_get_ob(re)->gl_context);
   _re_winfree(re);
}

static void 
_native_cb_bind(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *img;
   Native *n;

   if (!(img = image)) return;
   if (!(n = img->native.data)) return;

   if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
     {
        glBindTexture(GL_TEXTURE_2D, n->ns.data.opengl.texture_id);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
}

static void 
_native_cb_unbind(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *img;
   Native *n;

   if (!(img = image)) return;
   if (!(n = img->native.data)) return;

   if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
     {
        glBindTexture(GL_TEXTURE_2D, 0);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
}

static void 
_native_cb_free(void *data, void *image)
{
   Render_Engine *re;
   Outbuf *ob;
   Evas_GL_Image *img;
   Native *n;
   uint32_t texid;

   if (!(re = (Render_Engine *)data)) return;
   if (!(img = image)) return;
   if (!(n = img->native.data)) return;
   if (!(ob = eng_get_ob(re))) return;

   if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
     {
        texid = n->ns.data.opengl.texture_id;
        eina_hash_del(ob->gl_context->shared->native_tex_hash, &texid, img);
     }

   img->native.data = NULL;
   img->native.func.data = NULL;
   img->native.func.bind = NULL;
   img->native.func.unbind = NULL;
   img->native.func.free = NULL;

   free(n);
}

static void *
eng_image_native_set(void *data, void *image, void *native)
{
   Render_Engine *re;
   Outbuf *ob;
   Native *n;
   Evas_Native_Surface *ns;
   Evas_GL_Image *img, *img2;
   unsigned int tex = 0, fbo = 0;
   uint32_t texid;

   if (!(re = (Render_Engine *)data)) return NULL;
   if (!(ob = eng_get_ob(re))) return NULL;

   ns = native;

   if (!(img = image))
     {
        if ((ns) && (ns->type == EVAS_NATIVE_SURFACE_OPENGL))
          {
             img = 
               glsym_evas_gl_common_image_new_from_data(ob->gl_context, 
                                                        ns->data.opengl.w, 
                                                        ns->data.opengl.h, 
                                                        NULL, 1, 
                                                        EVAS_COLORSPACE_ARGB8888);
          }
        else
          return NULL;
     }

   if ((ns) && (ns->type == EVAS_NATIVE_SURFACE_OPENGL))
     {
        tex = ns->data.opengl.texture_id;
        fbo = ns->data.opengl.framebuffer_id;
        if (img->native.data)
          {
             Evas_Native_Surface *ens;

             ens = img->native.data;
             if ((ens->data.opengl.texture_id == tex) && 
                 (ens->data.opengl.framebuffer_id == fbo))
               return img;
          }
     }

   if ((!ns) && (!img->native.data)) return img;

   eng_window_use(ob);

   if (img->native.data)
     {
        if (img->native.func.free)
          img->native.func.free(img->native.func.data, img);
        glsym_evas_gl_common_image_native_disable(img);
     }

   if (!ns) return img;

   if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
     {
        texid = tex;
        img2 = eina_hash_find(ob->gl_context->shared->native_tex_hash, &texid);
        if (img2 == img) return img;
        if (img2)
          { 
             if ((n = img2->native.data))
               {
                  glsym_evas_gl_common_image_ref(img2);
                  glsym_evas_gl_common_image_free(img);
                  return img2;
               }
          }
     }

   img2 = glsym_evas_gl_common_image_new_from_data(ob->gl_context, img->w, 
                                                   img->h, NULL, img->alpha, 
                                                   EVAS_COLORSPACE_ARGB8888);
   glsym_evas_gl_common_image_free(img);

   if (!(img = img2)) return NULL;

   if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
     {
        if (native)
          {
             if ((n = calloc(1, sizeof(Native))))
               {
                  memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
                  eina_hash_add(ob->gl_context->shared->native_tex_hash, &texid, img);

                  n->egl_surface = 0;

                  img->native.yinvert = 0;
                  img->native.loose = 0;
                  img->native.data = n;
                  img->native.func.data = re;
                  img->native.func.bind = _native_cb_bind;
                  img->native.func.unbind = _native_cb_unbind;
                  img->native.func.free = _native_cb_free;
                  img->native.target = GL_TEXTURE_2D;
                  img->native.mipmap = 0;

                  glsym_evas_gl_common_image_native_enable(img);
               }
          }
     }

   return img;
}

Eina_Bool 
eng_preload_make_current(void *data, void *doit)
{
   Outbuf *ob;

   if (!(ob = data)) return EINA_FALSE;

   if (doit)
     {
        if (!eglMakeCurrent(ob->egl_disp, ob->egl_surface[0], 
                            ob->egl_surface[0], ob->egl_context[0]))
          return EINA_FALSE;
     }
   else
     {
        if (!eglMakeCurrent(ob->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, 
                            EGL_NO_CONTEXT))
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

/* evas module functions */
static int 
module_open(Evas_Module *em)
{
   /* check for valid module */
   if (!em) return 0;

   /* try to init evas_gl_common */
   if (!evas_gl_common_module_open()) return 0;

   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "gl_generic")) return 0;

   /* setup logging domain */
   if (_evas_engine_wl_egl_log_dom < 0)
     {
        _evas_engine_wl_egl_log_dom = 
          eina_log_domain_register("evas-wayland_egl", EVAS_DEFAULT_LOG_COLOR);
     }

   if (_evas_engine_wl_egl_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   /* store functions for later use */
   func = pfunc;

#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)

   ORD(info);
   ORD(info_free);
   ORD(setup);
   ORD(canvas_alpha_get);

   ORD(output_free);
   ORD(output_resize);
   ORD(output_dump);

   ORD(image_native_set);

   gl_symbols();

   /* advertise out which functions we support */
   em->functions = (void *)(&func);

   return 1;
}

static void 
module_close(Evas_Module *em EINA_UNUSED)
{
   eina_log_domain_unregister(_evas_engine_wl_egl_log_dom);
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION, "wayland_egl", "none", {module_open, module_close}
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, wayland_egl);

#ifndef EVAS_STATIC_BUILD_WAYLAND_EGL
EVAS_EINA_MODULE_DEFINE(engine, wayland_egl);
#endif
