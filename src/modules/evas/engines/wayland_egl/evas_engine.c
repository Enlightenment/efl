#include "evas_common_private.h"
#include "evas_engine.h"
#include "evas_gl_core_private.h"

#ifdef HAVE_DLSYM
# include <dlfcn.h>
#endif

#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

enum
{
   MERGE_BOUNDING,
   MERGE_FULL
};

enum 
{
   MODE_FULL,
   MODE_COPY,
   MODE_DOUBLE,
   MODE_TRIPLE
};

/* local structures */
typedef struct _Render_Engine Render_Engine;
struct _Render_Engine
{
   Tilebuf *tb;
   Tilebuf_Rect *rects;
   Tilebuf_Rect *rects_prev[3];
   Eina_Inlist *cur_rect;

   Evas_GL_Wl_Window *win;
   Evas_Engine_Info_Wayland_Egl *info;
   Evas *evas;

   int w, h, mode, vsync, prev_age;

   Eina_Bool lost_back : 1;
   Eina_Bool end : 1;
   Eina_Bool evgl_initted : 1;
   int       frame_cnt;

   struct 
     {
        Evas_Object_Image_Pixels_Get_Cb pixels_get;
        void *pixels_data_get;
        Evas_Object *obj;
     } func;
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

static Eina_Bool evgl_init(Render_Engine *re);
static void evgl_symbols(void);
static void evgl_extn_veto(Render_Engine *re);
static void *evgl_eng_display_get(void *data);
static void *evgl_eng_evas_surface_get(void *data);
static int evgl_eng_make_current(void *data, void *surface, void *context, int flush);
static void *evgl_eng_native_window_create(void *data);
static int evgl_eng_native_window_destroy(void *data, void *native_window);
static void *evgl_eng_window_surface_create(void *data, void *native_window);
static int evgl_eng_window_surface_destroy(void *data, void *surface);
static void *evgl_eng_context_create(void *data, void *share_context);
static int evgl_eng_context_destroy(void *data, void *context);
static const char *evgl_eng_string_get(void *data);
static void *evgl_eng_proc_address_get(const char *name);
static int evgl_eng_rotation_angle_get(void *data);

static Eina_Bool _re_wincheck(Render_Engine *re);
static void _re_winfree(Render_Engine *re);
static Tilebuf_Rect *_merge_rects(Tilebuf *tb, Tilebuf_Rect *r1, Tilebuf_Rect *r2, Tilebuf_Rect *r3);
static void _native_bind_cb(void *data EINA_UNUSED, void *image);
static void _native_unbind_cb(void *data EINA_UNUSED, void *image);
static void _native_free_cb(void *data, void *image);

static int eng_image_colorspace_get(void *data EINA_UNUSED, void *image);
static int eng_image_alpha_get(void *data EINA_UNUSED, void *image);

static Eina_Bool eng_gl_preload_make_current(void *data, void *doit);

#define EVGLINIT(_re, _ret) if (!evgl_init(_re)) return _ret;

/* local variables */
static Eina_Bool initted = EINA_FALSE;
static int gl_wins = 0;
static Eina_Bool extn_have_buffer_age = EINA_TRUE;
static int safe_native = -1;
static int partial_rect_union_mode = -1;

static int swap_buffer_debug_mode = -1;
static int swap_buffer_debug = 0;

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;
static EVGL_Interface evgl_funcs = 
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

/* external variables */
int _evas_engine_wl_egl_log_dom = -1;

#ifdef GL_GLES

# ifndef EGL_NATIVE_PIXMAP_KHR
#  define EGL_NATIVE_PIXMAP_KHR 0x30b0
# endif

# ifndef EGL_BUFFER_AGE_EXT
#  define EGL_BUFFER_AGE_EXT 0x313d
# endif

_eng_fn (*glsym_eglGetProcAddress) (const char *a) = NULL;
void *(*glsym_eglCreateImage) (EGLDisplay a, EGLContext b, EGLenum c, EGLClientBuffer d, const int *e) = NULL;
void (*glsym_eglDestroyImage) (EGLDisplay a, void *b) = NULL;
void (*glsym_glEGLImageTargetTexture2DOES) (int a, void *b)  = NULL;
void *(*glsym_eglMapImageSEC) (void *a, void *b, int c, int d) = NULL;
unsigned int (*glsym_eglUnmapImageSEC) (void *a, void *b, int c) = NULL;
unsigned int (*glsym_eglSwapBuffersWithDamage) (EGLDisplay a, void *b, const EGLint *d, EGLint c) = NULL;

#endif

/* local functions */
static Eina_Bool 
evgl_init(Render_Engine *re)
{
   if (!re) return EINA_FALSE;
   if (re->evgl_initted) return EINA_TRUE;
   if (!evgl_engine_init(re, &evgl_funcs)) return EINA_FALSE;
   re->evgl_initted = EINA_TRUE;
   return EINA_TRUE;
}

static void 
evgl_symbols(void)
{
   static Eina_Bool done = EINA_FALSE;

   if (done) return;

#define FINDSYM(dst, sym, typ) \
   if (glsym_eglGetProcAddress) { \
      if (!dst) dst = (typ)glsym_eglGetProcAddress(sym); \
   } \
   if (!dst) dst = (typ)dlsym(RTLD_DEFAULT, sym);

   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressKHR", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressEXT", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressARB", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddress", glsym_func_eng_fn);

   FINDSYM(glsym_eglCreateImage, "eglCreateImageKHR", glsym_func_void_ptr);
   FINDSYM(glsym_eglCreateImage, "eglCreateImageEXT", glsym_func_void_ptr);
   FINDSYM(glsym_eglCreateImage, "eglCreateImageARB", glsym_func_void_ptr);
   FINDSYM(glsym_eglCreateImage, "eglCreateImage", glsym_func_void_ptr);

   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageKHR", glsym_func_void);
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageEXT", glsym_func_void);
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageARB", glsym_func_void);
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImage", glsym_func_void);

   FINDSYM(glsym_glEGLImageTargetTexture2DOES, 
           "glEGLImageTargetTexture2DOES", glsym_func_void);

   FINDSYM(glsym_eglMapImageSEC, "eglMapImageSEC", glsym_func_void_ptr);
   FINDSYM(glsym_eglUnmapImageSEC, "eglUnmapImageSEC", glsym_func_uint);

   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamageEXT", 
           glsym_func_uint);
   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamageINTEL", 
           glsym_func_uint);
   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamage", 
           glsym_func_uint);

   done = EINA_TRUE;
}

static void 
evgl_extn_veto(Render_Engine *re)
{
   const char *str = NULL;

   if ((!re) || (!re->win)) return;

   str = eglQueryString(re->win->egl_disp, EGL_EXTENSIONS);
   if (str)
     {
        if (getenv("EVAS_GL_INFO")) printf("EGL EXTENSION:\n%s\n", str);
        if (!strstr(str, "EGL_EXT_buffer_age"))
          extn_have_buffer_age = EINA_FALSE;
        /* if (!strstr(str, "swap_buffers_with_damage")) */
        /*   glsym_eglSwapBuffersWithDamage = NULL; */
     }
   else
     {
        if (getenv("EVAS_GL_INFO")) printf("NO EGL EXTENSIONS !!\n");
        extn_have_buffer_age = EINA_FALSE;
        glsym_eglSwapBuffersWithDamage = NULL;
     }
}

static void *
evgl_eng_display_get(void *data)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return NULL;

   EVGLINIT(re, NULL);
   if (re->win) return re->win->egl_disp;
   return NULL;
}

static void *
evgl_eng_evas_surface_get(void *data)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return NULL;

   EVGLINIT(re, NULL);
   if (re->win) return re->win->egl_surface[0];
   return NULL;
}

static int 
evgl_eng_make_current(void *data, void *surface, void *context, int flush)
{
   Render_Engine *re;
   EGLContext ctx;
   EGLSurface sfc;
   EGLDisplay dpy;
   int ret = 0;

   if (!(re = (Render_Engine *)data)) return 0;

   EVGLINIT(re, 0);

   dpy = re->win->egl_disp;
   ctx = (EGLContext)context;
   sfc = (EGLSurface)surface;

   if ((!context) && (!surface))
     {
        ret = eglMakeCurrent(dpy, EGL_NO_SURFACE, 
                             EGL_NO_SURFACE, EGL_NO_CONTEXT);
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
}

static void *
evgl_eng_native_window_create(void *data)
{
   Render_Engine *re;
   struct wl_egl_window *win;

   if (!(re = (Render_Engine *)data)) return NULL;
   if (!re->info) return NULL;
//   if (!re->info->info.surface) return NULL;

   EVGLINIT(re, NULL);

   win = wl_egl_window_create(re->info->info.surface, 1, 1);

   return win;
}

static int 
evgl_eng_native_window_destroy(void *data, void *native_window)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return 0;

   EVGLINIT(re, 0);

   wl_egl_window_destroy((struct wl_egl_window *)native_window);

   return 1;
}

static void *
evgl_eng_window_surface_create(void *data, void *native_window)
{
   Render_Engine *re;
   EGLSurface surface = EGL_NO_SURFACE;

   if (!(re = (Render_Engine *)data)) return NULL;

   EVGLINIT(re, NULL);

   surface = 
     eglCreateWindowSurface(re->win->egl_disp, re->win->egl_config,
                            (EGLNativeWindowType)native_window, NULL);
   if (!surface)
     {
        ERR("Creating window surface failed. Error: %#x.", eglGetError());
        return NULL;
     }

   return surface;
}

static int 
evgl_eng_window_surface_destroy(void *data, void *surface)
{
   Render_Engine *re;
   EGLBoolean ret = EGL_FALSE;

   if (!(re = (Render_Engine *)data)) return 0;
   if (!surface) return 0;

   ret = eglDestroySurface(re->win->egl_disp, (EGLSurface)surface);
   if (ret == EGL_TRUE) return 1;

   return 0;
}

static void *
evgl_eng_context_create(void *data, void *share_context)
{
   Render_Engine *re;
   EGLContext context = EGL_NO_CONTEXT;
   int context_attrs[3];

   if (!(re = (Render_Engine *)data)) return NULL;

   EVGLINIT(re, NULL);

   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = 2;
   context_attrs[2] = EGL_NONE;

   // Share context already assumes that it's sharing with evas' context
   if (share_context)
     {
        context = eglCreateContext(re->win->egl_disp, re->win->egl_config,
                                   (EGLContext)share_context, context_attrs);
     }
   else
     {
        context = eglCreateContext(re->win->egl_disp, re->win->egl_config,
                                   re->win->egl_context[0], context_attrs);
     }

   if (!context)
     {
        ERR("Engine Context Creations Failed. Error: %#x.", eglGetError());
        return NULL;
     }

   return (void*)context;
}

static int 
evgl_eng_context_destroy(void *data, void *context)
{
   Render_Engine *re;
   EGLBoolean ret = EGL_FALSE;

   if (!(re = (Render_Engine *)data)) return 0;

   EVGLINIT(re, 0);

   if (!context) return 0;

   ret = eglDestroyContext(re->win->egl_disp, (EGLContext)context);
   if (ret == EGL_TRUE) return 1;

   return 0;
}

static const char *
evgl_eng_string_get(void *data)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return NULL;

   EVGLINIT(re, NULL);

   return eglQueryString(re->win->egl_disp, EGL_EXTENSIONS);
}

static void *
evgl_eng_proc_address_get(const char *name)
{
   if (glsym_eglGetProcAddress) return glsym_eglGetProcAddress(name);
   return dlsym(RTLD_DEFAULT, name);
}

static int 
evgl_eng_rotation_angle_get(void *data)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return 0;

   EVGLINIT(re, 0);

   if ((re->win) && (re->win->gl_context))
     return re->win->gl_context->rot;

   return 0;
}

static Eina_Bool 
_re_wincheck(Render_Engine *re)
{
   if (!re) return EINA_FALSE;
   if (re->win)
     {
        if (re->win->surf) return EINA_TRUE;

        eng_window_resurf(re->win);
        re->lost_back = EINA_TRUE;

        if (!re->win->surf)
          ERR("EGL engine can't re-create window surface!");
     }

   return EINA_FALSE;
}

static void 
_re_winfree(Render_Engine *re)
{
   if ((!re) || (!re->win)) return;
   if (!re->win->surf) return;
   evas_gl_preload_render_relax(eng_gl_preload_make_current, re);
   eng_window_unsurf(re->win);
}

static Tilebuf_Rect *
_merge_rects(Tilebuf *tb, Tilebuf_Rect *r1, Tilebuf_Rect *r2, Tilebuf_Rect *r3)
{
   Tilebuf_Rect *r, *rects;
   Evas_Point p1, p2;

   if (r1)
     {
        EINA_INLIST_FOREACH(EINA_INLIST_GET(r1), r)
          evas_common_tilebuf_add_redraw(tb, r->x, r->y, r->w, r->h);
     }
   if (r2)
     {
        EINA_INLIST_FOREACH(EINA_INLIST_GET(r2), r)
          evas_common_tilebuf_add_redraw(tb, r->x, r->y, r->w, r->h);
     }
   if (r3)
     {
        EINA_INLIST_FOREACH(EINA_INLIST_GET(r3), r)
          evas_common_tilebuf_add_redraw(tb, r->x, r->y, r->w, r->h);
     }

   rects = evas_common_tilebuf_get_render_rects(tb);
   
   if (partial_rect_union_mode == -1)
     {
        const char *s;

        if ((s = getenv("EVAS_GL_PARTIAL_MERGE")))
          {
             if ((!strcmp(s, "bounding")) || (!strcmp(s, "b")))
               partial_rect_union_mode = MERGE_BOUNDING;
             else if ((!strcmp(s, "full")) || (!strcmp(s, "f")))
               partial_rect_union_mode = MERGE_FULL;
          }
        else
          partial_rect_union_mode = MERGE_BOUNDING;
     }

   if (partial_rect_union_mode == MERGE_BOUNDING)
     {
// bounding box -> make a bounding box single region update of all regions.
// yes we could try and be smart and figure out size of regions, how far
// apart etc. etc. to try and figure out an optimal "set". this is a tradeoff
// between multiple update regions to render and total pixels to render.
        if (rects)
          {
             p1.x = rects->x; p1.y = rects->y;
             p2.x = rects->x + rects->w; p2.y = rects->y + rects->h;
             EINA_INLIST_FOREACH(EINA_INLIST_GET(rects), r)
               {
                  if (r->x < p1.x) p1.x = r->x;
                  if (r->y < p1.y) p1.y = r->y;
                  if ((r->x + r->w) > p2.x) p2.x = r->x + r->w;
                  if ((r->y + r->h) > p2.y) p2.y = r->y + r->h;
               }
             evas_common_tilebuf_free_render_rects(rects);
             rects = calloc(1, sizeof(Tilebuf_Rect));
             if (rects)
               {
                  rects->x = p1.x;
                  rects->y = p1.y;
                  rects->w = p2.x - p1.x;
                  rects->h = p2.y - p1.y;
               }
          }
     }

   evas_common_tilebuf_clear(tb);
   return rects;
}

static void 
_native_bind_cb(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *im;
   Native *n;

   if (!(im = image)) return;
   if (!(n = im->native.data)) return;

   if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
     glBindTexture(GL_TEXTURE_2D, n->ns.data.opengl.texture_id);
}

static void 
_native_unbind_cb(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *im;
   Native *n;

   if (!(im = image)) return;
   if (!(n = im->native.data)) return;
   if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
     glBindTexture(GL_TEXTURE_2D, 0);
}

static void 
_native_free_cb(void *data, void *image)
{
   Render_Engine *re;
   Evas_GL_Image *im;
   Native *n;
   unsigned int texid;

   if (!(re = (Render_Engine *)data)) return;
   if (!(im = image)) return;
   if (!(n = im->native.data)) return;

   if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
     {
        texid = n->ns.data.opengl.texture_id;
        eina_hash_del(re->win->gl_context->shared->native_tex_hash, 
                      &texid, im);
     }

   im->native.data = NULL;
   im->native.func.data = NULL;
   im->native.func.bind = NULL;
   im->native.func.unbind = NULL;
   im->native.func.free = NULL;
   free(n);
}

/* engine specific override functions */
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

   inf = (Evas_Engine_Info_Wayland_Egl *)info;
   free(inf);
}

static int 
eng_setup(Evas *evas, void *info)
{
   Evas_Engine_Info_Wayland_Egl *inf;
   Evas_Public_Data *epd;
   Render_Engine *re;
   const char *s;

   inf = (Evas_Engine_Info_Wayland_Egl *)info;
   epd = eo_data_scope_get(evas, EVAS_CLASS);

   /* check for existing engine output */
   if (!epd->engine.data.output)
     {
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

        /* set some properties of the render engine */
        re->info = inf;
        re->evas = evas;
        re->w = epd->output.w;
        re->h = epd->output.h;

        /* if we have not initialize gl & evas, do it */
        if (!initted)
          {
             evgl_symbols();
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
             evas_gl_preload_init();
             evgl_extn_veto(re);
//             evgl_engine_init(re, &evgl_funcs);
             initted = EINA_TRUE;
          }

        /* try to create a new window */
        re->win = eng_window_new(inf->info.display, inf->info.surface, 
                                 inf->info.screen, inf->info.depth, 
                                 re->w, re->h, inf->indirect, 
                                 inf->info.destination_alpha, 
                                 inf->info.rotation);
        if (!re->win)
          {
             free(re);
             return 0;
          }

        /* tell the engine to use this render_engine for output */
        epd->engine.data.output = re;
        gl_wins++;
     }
   else
     {
        re = epd->engine.data.output;
        if (_re_wincheck(re))
          {
             re->info = inf;
             if ((re->info->info.display != re->win->disp) || 
                 (re->info->info.surface != re->win->surface) || 
                 (re->info->info.screen != re->win->screen) || 
                 (re->info->info.depth != re->win->depth) || 
                 (re->info->info.rotation != re->win->rot) || 
                 (re->info->info.destination_alpha != re->win->alpha))
               {
                  Eina_Bool inc = EINA_FALSE;

                  if (re->win)
                    {
                       re->win->gl_context->references++;
                       eng_window_free(re->win);
                       inc = EINA_TRUE;
                       gl_wins--;
                    }

                  re->w = epd->output.w;
                  re->h = epd->output.h;

                  re->win = eng_window_new(re->info->info.display, 
                                           re->info->info.surface, 
                                           re->info->info.screen, 
                                           re->info->info.depth, 
                                           epd->output.w, epd->output.h,
                                           re->info->indirect, 
                                           re->info->info.destination_alpha, 
                                           re->info->info.rotation);

                  eng_window_use(re->win);
                  if (re->win) gl_wins++;
                  if ((re->win) && (inc))
                    re->win->gl_context->references--;
               }
             else if ((re->win->w != epd->output.w) || 
                      (re->win->h != epd->output.h))
               {
                  re->w = epd->output.w;
                  re->h = epd->output.h;
                  re->win->w = epd->output.w;
                  re->win->h = epd->output.h;
                  eng_window_use(re->win);
                  evas_gl_common_context_resize(re->win->gl_context, 
                                                re->win->w, re->win->h, 
                                                re->win->rot);
               }
          }
     }

   if ((s = getenv("EVAS_GL_SWAP_MODE")))
     {
        if ((!strcasecmp(s, "full")) || (!strcasecmp(s, "f")))
          re->mode = MODE_FULL;
        else if ((!strcasecmp(s, "copy")) || (!strcasecmp(s, "c")))
          re->mode = MODE_COPY;
        else if ((!strcasecmp(s, "double")) || (!strcasecmp(s, "d")) || 
                 (!strcasecmp(s, "2")))
          re->mode = MODE_DOUBLE;
        else if ((!strcasecmp(s, "triple")) || (!strcasecmp(s, "t")) || 
                 (!strcasecmp(s, "3")))
          re->mode = MODE_TRIPLE;
     }
   else
     {
// in most gl implementations - egl and glx here that we care about the TEND
// to either swap or copy backbuffer and front buffer, but strictly that is
// not true. technically backbuffer content is totally undefined after a swap
// and thus you MUST re-render all of it, thus MODE_FULL
        re->mode = MODE_FULL;
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
             re->mode = MODE_FULL;
             break;
           case EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_COPY:
             re->mode = MODE_COPY;
             break;
           case EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_DOUBLE:
             re->mode = MODE_DOUBLE;
             break;
           case EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_TRIPLE:
             re->mode = MODE_TRIPLE;
             break;
           default:
             break;
          }
     }

   if (!re->win)
     {
        free(re);
        epd->engine.data.output = NULL;
        return 0;
     }

   if (!epd->engine.data.output)
     {
        if (re->win)
          {
             eng_window_free(re->win);
             gl_wins--;
          }
        free(re);
        epd->engine.data.output = NULL;
        return 0;
     }

   if (re->tb) evas_common_tilebuf_free(re->tb);
   re->tb = evas_common_tilebuf_new(re->win->w, re->win->h);
   if (!re->tb)
     {
        if (re->win)
          {
             eng_window_free(re->win);
             gl_wins--;
          }
        free(re);
        epd->engine.data.output = NULL;
        return 0;
     }

   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
   evas_common_tilebuf_tile_strict_set(re->tb, EINA_TRUE);

   if (!epd->engine.data.context)
     {
        epd->engine.data.context =
          epd->engine.func->context_new(epd->engine.data.output);
     }

   eng_window_use(re->win);

   re->vsync = 0;

   return 1;
}

static Eina_Bool 
eng_canvas_alpha_get(void *data, void *info EINA_UNUSED)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) 
     return EINA_FALSE;

   if (re->win) 
     return re->win->alpha;
   else if (re->info) 
     return re->info->info.destination_alpha;

   return EINA_FALSE;
}

static void 
eng_output_free(void *data)
{
   Render_Engine *re;

   if ((re = (Render_Engine *)data))
     {
        evas_gl_preload_render_relax(eng_gl_preload_make_current, re);

        if (re->win)
          {
             if (gl_wins == 1) evgl_engine_shutdown(re);

             eng_window_free(re->win);
             gl_wins--;
          }

        evas_common_tilebuf_free(re->tb);
        if (re->rects) 
          evas_common_tilebuf_free_render_rects(re->rects);
        if (re->rects_prev[0]) 
          evas_common_tilebuf_free_render_rects(re->rects_prev[0]);
        if (re->rects_prev[1]) 
          evas_common_tilebuf_free_render_rects(re->rects_prev[1]);
        if (re->rects_prev[2]) 
          evas_common_tilebuf_free_render_rects(re->rects_prev[2]);

        free(re);
     }

   if ((initted) && (!gl_wins))
     {
        evas_gl_preload_shutdown();
        evas_common_image_shutdown();
        evas_common_font_shutdown();
        initted = EINA_FALSE;
     }
}

static void 
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;

   if (re->win)
     {
        re->win->w = w;
        re->win->h = h;
        eng_window_use(re->win);
     }

   if (re->win->win)
     {
        int aw, ah, dx = 0, dy = 0;

        if ((re->win->rot == 90) || (re->win->rot == 270))
          wl_egl_window_get_attached_size(re->win->win, &ah, &aw);
        else
          wl_egl_window_get_attached_size(re->win->win, &aw, &ah);

        if (re->info->info.edges & 4) // resize from left
          {
             if ((re->win->rot == 90) || (re->win->rot == 270))
               dx = ah - h;
             else
               dx = aw - w;
          }

        if (re->info->info.edges & 1) // resize from top
          {
             if ((re->win->rot == 90) || (re->win->rot == 270))
               dy = aw - w;
             else
               dy = ah - h;
          }

        if ((re->win->rot == 90) || (re->win->rot == 270))
          wl_egl_window_resize(re->win->win, h, w, dx, dy);
        else
          wl_egl_window_resize(re->win->win, w, h, dx, dy);
     }

   if (re->win)
     evas_gl_common_context_resize(re->win->gl_context, w, h, re->win->rot);

   evas_common_tilebuf_free(re->tb);
   if ((re->tb = evas_common_tilebuf_new(w, h)))
     {
        evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
        evas_common_tilebuf_tile_strict_set(re->tb, EINA_TRUE);
     }
}

static void 
eng_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

static void 
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;
   if (re->win)
     {
        eng_window_use(re->win);
        evas_gl_common_context_resize(re->win->gl_context, 
                                      re->win->w, re->win->h, re->win->rot);
     }
   if (re->tb) evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);
}

static void 
eng_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

static void 
eng_output_redraws_clear(void *data)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_clear(re->tb);
}

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;
   Tilebuf_Rect *rect;
   Eina_Bool first_rect = EINA_FALSE;

#define CLEAR_PREV_RECTS(x) \
   do { \
      if (re->rects_prev[x]) \
        evas_common_tilebuf_free_render_rects(re->rects_prev[x]); \
      re->rects_prev[x] = NULL; \
   } while (0)

   if (!(re = (Render_Engine *)data)) return NULL;

   if (re->end)
     {
        re->end = EINA_FALSE;
        return NULL;
     }

   if (!re->rects)
     {
        re->rects = evas_common_tilebuf_get_render_rects(re->tb);
        if (re->rects)
          {
             if (re->info->swap_mode == EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_AUTO)
               {
                  if (extn_have_buffer_age)
                    {
                       EGLint age = 0;

                       if (!eglQuerySurface(re->win->egl_disp, 
                                            re->win->egl_surface[0], 
                                            EGL_BUFFER_AGE_EXT, &age))
                         age = 0;

                       if (age == 1) re->mode = MODE_COPY;
                       else if (age == 2) re->mode = MODE_DOUBLE;
                       else if (age == 3) re->mode = MODE_TRIPLE;
                       else re->mode = MODE_FULL;
                       if ((int)age != re->prev_age) re->mode = MODE_FULL;
                       re->prev_age = age;
                    }
               }

             /* NB: Disabled this OR for now as it causes some very strange things in the GLView/Simple tests */
             if ((re->lost_back))// || (re->mode == MODE_FULL))
               {
                  re->lost_back = EINA_FALSE;
                  evas_common_tilebuf_add_redraw(re->tb, 0, 0, 
                                                 re->win->w, re->win->h);
                  evas_common_tilebuf_free_render_rects(re->rects);
                  re->rects = evas_common_tilebuf_get_render_rects(re->tb);
               }

             evas_common_tilebuf_clear(re->tb);
             CLEAR_PREV_RECTS(2);
             re->rects_prev[2] = re->rects_prev[1];
             re->rects_prev[1] = re->rects_prev[0];
             re->rects_prev[0] = re->rects;
             re->rects = NULL;
             switch (re->mode)
               {
                case MODE_FULL:
                case MODE_COPY: // no prev rects needed
                  re->rects = 
                    _merge_rects(re->tb, re->rects_prev[0], NULL, NULL);
                  break;
                case MODE_DOUBLE: // double mode - only 1 level of prev rect
                  re->rects = 
                    _merge_rects(re->tb, re->rects_prev[0], 
                                 re->rects_prev[1], NULL);
                  break;
                case MODE_TRIPLE: // keep all
                  re->rects = 
                    _merge_rects(re->tb, re->rects_prev[0], 
                                 re->rects_prev[1], re->rects_prev[2]);
                  break;
                default:
                  break;
               }
             first_rect = EINA_TRUE;
          }
        evas_common_tilebuf_clear(re->tb);
        re->cur_rect = EINA_INLIST_GET(re->rects);
     }

   if (!re->cur_rect) return NULL;

   rect = (Tilebuf_Rect *)re->cur_rect;
   if (re->rects)
     {
        switch (re->mode)
          {
           case MODE_COPY:
           case MODE_DOUBLE:
           case MODE_TRIPLE:
             rect = (Tilebuf_Rect *)re->cur_rect;
             *x = rect->x;
             *y = rect->y;
             *w = rect->w;
             *h = rect->h;
             *cx = rect->x;
             *cy = rect->y;
             *cw = rect->w;
             *ch = rect->h;
             re->cur_rect = re->cur_rect->next;
             re->win->gl_context->master_clip.enabled = EINA_TRUE;
             re->win->gl_context->master_clip.x = rect->x;
             re->win->gl_context->master_clip.y = rect->y;
             re->win->gl_context->master_clip.w = rect->w;
             re->win->gl_context->master_clip.h = rect->h;
             break;
           case MODE_FULL:
             re->cur_rect = NULL;
             if (x) *x = 0;
             if (y) *y = 0;
             if (w) *w = re->win->w;
             if (h) *h = re->win->h;
             if (cx) *cx = 0;
             if (cy) *cy = 0;
             if (cw) *cw = re->win->w;
             if (ch) *ch = re->win->h;
             re->win->gl_context->master_clip.enabled = EINA_FALSE;
             break;
           default:
             break;
          }

        if (first_rect)
          {
             evas_gl_preload_render_lock(eng_gl_preload_make_current, re);

             eng_window_use(re->win);
             if (!_re_wincheck(re)) return NULL;
             
             evas_gl_common_context_flush(re->win->gl_context);
             evas_gl_common_context_newframe(re->win->gl_context);
          }

        if (!re->cur_rect) re->end = EINA_TRUE;

        return re->win->gl_context->def_surface;
     }

   return NULL;
}

static void 
eng_output_redraws_next_update_push(void *data, void *surface EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED, Evas_Render_Mode render_mode)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   if (!_re_wincheck(re)) return;

   re->win->draw.drew = EINA_TRUE;
   if (re->win->gl_context) 
     evas_gl_common_context_flush(re->win->gl_context);

   if (safe_native == -1)
     {
        const char *s;

        safe_native = 0;
        if ((s = getenv("EVAS_GL_SAFE_NATIVE")))
          safe_native = atoi(s);
        else
          {
             if ((s = (const char *)glGetString(GL_RENDERER)))
               {
                  if (strstr(s, "PowerVR SGX 540") || strstr(s, "Mali-400 MP"))
                    safe_native = 1;
               }
          }
     }
}

static void 
eng_output_flush(void *data, Evas_Render_Mode render_mode)
{
   Render_Engine *re;
   static char *dname = NULL;

   if (!(re = (Render_Engine *)data)) return;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) goto end;

   if (!_re_wincheck(re)) goto end;

   if (!re->win->draw.drew) goto end;
   re->win->draw.drew = EINA_FALSE;
   eng_window_use(re->win);

   evas_gl_common_context_done(re->win->gl_context);

   // Save contents of the framebuffer to a file
   if (swap_buffer_debug_mode == -1)
     {
        if ((dname = getenv("EVAS_GL_SWAP_BUFFER_DEBUG_DIR")))
          {
             int stat;
             // Create a directory with 0775 permission
             stat = mkdir(dname, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
             if ((!stat) || errno == EEXIST) swap_buffer_debug_mode = 1;
          }
        else
           swap_buffer_debug_mode = 0;
     }

   if (swap_buffer_debug_mode == 1)
     {
        // Set this env var to dump files every frame
        // Or set the global var in gdb to 1|0 to turn it on and off
        if (getenv("EVAS_GL_SWAP_BUFFER_DEBUG_ALWAYS"))
           swap_buffer_debug = 1;

        if (swap_buffer_debug)
          {
             char fname[100];
             int ret = 0;
             sprintf(fname, "%p", (void*)re->win);

             ret = evas_gl_common_buffer_dump(re->win->gl_context,
                                              (const char*)dname,
                                              (const char*)fname,
                                              re->frame_cnt,
                                              NULL);
             if (!ret) swap_buffer_debug_mode = 0;
          }
     }

   if (!re->vsync)
     {
        if (re->info->vsync) eglSwapInterval(re->win->egl_disp, 1);
        else eglSwapInterval(re->win->egl_disp, 0);
        re->vsync = 1;
     }

   if (re->info->callback.pre_swap)
     re->info->callback.pre_swap(re->info->callback.data, re->evas);

   if ((glsym_eglSwapBuffersWithDamage) && (re->mode != MODE_FULL))
     {
        EGLint num = 0, *rects = NULL, i = 0;
        Tilebuf_Rect *r;
        
        // if partial swaps can be done use re->rects
        EINA_INLIST_FOREACH(EINA_INLIST_GET(re->rects), r) num++;
        if (num > 0)
          {
             rects = alloca(sizeof(EGLint) * 4 * num);
             EINA_INLIST_FOREACH(EINA_INLIST_GET(re->rects), r)
               {
                  int gw, gh;
                  
                  gw = re->win->gl_context->w;
                  gh = re->win->gl_context->h;
                  switch (re->win->rot)
                    {
                     case 0:
                       rects[i + 0] = r->x;
                       rects[i + 1] = gh - (r->y + r->h);
                       rects[i + 2] = r->w;
                       rects[i + 3] = r->h;
                       break;
                     case 90:
                       rects[i + 0] = r->y;
                       rects[i + 1] = r->x;
                       rects[i + 2] = r->h;
                       rects[i + 3] = r->w;
                       break;
                     case 180:
                       rects[i + 0] = gw - (r->x + r->w);
                       rects[i + 1] = r->y;
                       rects[i + 2] = r->w;
                       rects[i + 3] = r->h;
                       break;
                     case 270:
                       rects[i + 0] = gh - (r->y + r->h);
                       rects[i + 1] = gw - (r->x + r->w);
                       rects[i + 2] = r->h;
                       rects[i + 3] = r->w;
                       break;
                     default:
                       rects[i + 0] = r->x;
                       rects[i + 1] = gh - (r->y + r->h);
                       rects[i + 2] = r->w;
                       rects[i + 3] = r->h;
                       break;
                    }
                  i += 4;
               }
             glsym_eglSwapBuffersWithDamage(re->win->egl_disp,
                                            re->win->egl_surface[0],
                                            rects, num);
          }
     }
   else
     eglSwapBuffers(re->win->egl_disp, re->win->egl_surface[0]);

   if (re->info->callback.post_swap)
     re->info->callback.post_swap(re->info->callback.data, re->evas);

   if (re->rects)
     {
        evas_common_tilebuf_free_render_rects(re->rects);
        re->rects = NULL;
     }

   re->frame_cnt++;
   
end:
   evas_gl_preload_render_unlock(eng_gl_preload_make_current, re);
}

static void 
eng_output_idle_flush(void *data EINA_UNUSED)
{

}

static void 
eng_output_dump(void *data)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;

   evas_common_image_image_all_unload();
   evas_common_font_font_all_unload();
   if (re->win) evas_gl_common_image_all_unload(re->win->gl_context);
   _re_winfree(re);
}

static void 
eng_context_cutout_add(void *data EINA_UNUSED, void *context, int x, int y, int w, int h)
{
   evas_common_draw_context_add_cutout(context, x, y, w, h);
}

static void 
eng_context_cutout_clear(void *data EINA_UNUSED, void *context)
{
   evas_common_draw_context_clear_cutouts(context);
}

static void 
eng_context_flush(void *data)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;
   if (re->win)
     {
        eng_window_use(re->win);
        evas_gl_common_context_flush(re->win->gl_context);
     }
}

static void *
eng_gl_surface_create(void *data, void *config, int w, int h)
{
   Evas_GL_Config *cfg;

   cfg = (Evas_GL_Config *)config;
   EVGLINIT(data, NULL);
   return evgl_surface_create(data, cfg, w, h);
}

static int 
eng_gl_surface_destroy(void *data, void *surface)
{
   EVGL_Surface *sfc;

   sfc = (EVGL_Surface *)surface;
   EVGLINIT(data, 0);
   return evgl_surface_destroy(data, sfc);
}

static void *
eng_gl_context_create(void *data, void *share_context)
{
   EVGL_Context *sctx;

   sctx = (EVGL_Context *)share_context;
   EVGLINIT(data, NULL);
   return evgl_context_create(data, sctx);
}

static int 
eng_gl_context_destroy(void *data, void *context)
{
   EVGL_Context *sctx;

   sctx = (EVGL_Context *)context;
   EVGLINIT(data, 0);
   return evgl_context_destroy(data, sctx);
}

static int 
eng_gl_make_current(void *data, void *surface, void *context)
{
   EVGL_Surface *sfc;
   EVGL_Context *ctx;

   sfc = (EVGL_Surface *)surface;
   ctx = (EVGL_Context *)context;
   EVGLINIT(data, 0);
   return evgl_make_current(data, sfc, ctx);
}

static void *
eng_gl_string_query(void *data, int name)
{
   EVGLINIT(data, NULL);
   return (void *)evgl_string_query(name);
}

// Need to deprecate this function..
static void *
eng_gl_proc_address_get(void *data EINA_UNUSED, const char *name EINA_UNUSED)
{
   return NULL;
}

static int 
eng_gl_native_surface_get(void *data EINA_UNUSED, void *surface, void *native_surface)
{
   EVGL_Surface *sfc;
   Evas_Native_Surface *ns;

   sfc = (EVGL_Surface *)surface;
   ns = (Evas_Native_Surface *)native_surface;
   return evgl_native_surface_get(sfc, ns);
}

static void *
eng_gl_api_get(void *data)
{
   EVGLINIT(data, NULL);
   return evgl_api_get();
}

static void
eng_gl_direct_override_get(void *data, int *override, int *force_off)
{
   EVGLINIT(data, );
   evgl_direct_override_get(override, force_off);
}

static void
eng_gl_get_pixels_set(void *data, void *get_pixels, void *get_pixels_data, void *obj)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;

   EVGLINIT(data, );
   re->func.pixels_get = get_pixels;
   re->func.pixels_data_get = get_pixels_data;
   re->func.obj = (Evas_Object*)obj;
}

static Eina_Bool 
eng_gl_preload_make_current(void *data, void *doit)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) 
     return EINA_FALSE;

   if (doit)
     {
        if (!eglMakeCurrent(re->win->egl_disp, re->win->egl_surface[0], 
                            re->win->egl_surface[0], re->win->egl_context[0]))
          return EINA_FALSE;
     }
   else
     {
        if (!eglMakeCurrent(re->win->egl_disp, EGL_NO_SURFACE, 
                            EGL_NO_SURFACE, EGL_NO_CONTEXT))
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void 
eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h, Eina_Bool do_async EINA_UNUSED)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_rect_draw(re->win->gl_context, x, y, w, h);
}

static void
eng_line_draw(void *data, void *context, void *surface, int p1x, int p1y, int p2x, int p2y, Eina_Bool do_async EINA_UNUSED)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_line_draw(re->win->gl_context, p1x, p1y, p2x, p2y);
}

static Eina_Bool
eng_font_draw(void *data, void *context, void *surface, Evas_Font_Set *font EINA_UNUSED, int x, int y, int w EINA_UNUSED, int h EINA_UNUSED, int ow EINA_UNUSED, int oh EINA_UNUSED, Evas_Text_Props *intl_props, Eina_Bool do_async EINA_UNUSED)
{
   Render_Engine *re;
   static RGBA_Image *im = NULL;

   if (!(re = (Render_Engine *)data)) return EINA_FALSE;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;

   // FIXME: put im into context so we can free it

   if (!im)
     im = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());

   im->cache_entry.w = re->win->gl_context->shared->w;
   im->cache_entry.h = re->win->gl_context->shared->h;

   evas_common_draw_context_font_ext_set(context, re->win->gl_context,
                                         evas_gl_font_texture_new,
                                         evas_gl_font_texture_free,
                                         evas_gl_font_texture_draw);
   evas_common_font_draw_prepare(intl_props);
   evas_common_font_draw(im, context, x, y, intl_props->glyphs);
   evas_common_draw_context_font_ext_set(context, NULL, NULL, NULL, NULL);

   return EINA_FALSE;
}

static void
eng_polygon_draw(void *data, void *context, void *surface EINA_UNUSED, void *polygon, int x, int y, Eina_Bool do_async EINA_UNUSED)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_poly_draw(re->win->gl_context, polygon, x, y);
}

static void *
eng_polygon_point_add(void *data EINA_UNUSED, void *context EINA_UNUSED, void *polygon, int x, int y)
{
   return evas_gl_common_poly_point_add(polygon, x, y);
}

static void *
eng_polygon_points_clear(void *data EINA_UNUSED, void *context EINA_UNUSED, void *polygon)
{
   return evas_gl_common_poly_points_clear(polygon);
}

static void *
eng_image_load(void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return NULL;
   *error = EVAS_LOAD_ERROR_NONE;
   eng_window_use(re->win);
   return evas_gl_common_image_load(re->win->gl_context, file, key, lo, error);
}

static void *
eng_image_mmap(void *data, Eina_File *f, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return NULL;
   *error = EVAS_LOAD_ERROR_NONE;
   eng_window_use(re->win);
   return evas_gl_common_image_mmap(re->win->gl_context, f, key, lo, error);
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return NULL;
   eng_window_use(re->win);
   return evas_gl_common_image_new_from_data(re->win->gl_context, w, h, image_data, alpha, cspace);
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return NULL;
   eng_window_use(re->win);
   return evas_gl_common_image_new_from_copied_data(re->win->gl_context, w, h, image_data, alpha, cspace);
}

static void
eng_image_free(void *data, void *image)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;
   if (!image) return;
   eng_window_use(re->win);
   evas_gl_common_image_free(image);
}

static void
eng_image_size_get(void *data EINA_UNUSED, void *image, int *w, int *h)
{
   if (!image)
     {
        *w = 0;
        *h = 0;
        return;
     }
   if (w) *w = ((Evas_GL_Image *)image)->w;
   if (h) *h = ((Evas_GL_Image *)image)->h;
}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   Render_Engine *re;
   Evas_GL_Image *im, *im_old;

   if (!(re = (Render_Engine *)data)) return NULL;
   if (!(im = image)) return NULL;

   if (im->native.data)
     {
        im->w = w;
        im->h = h;
        return image;
     }
   eng_window_use(re->win);
   if ((im->tex) && (im->tex->pt->dyn.img))
     {
        evas_gl_common_texture_free(im->tex, EINA_TRUE);
        im->tex = NULL;
        im->w = w;
        im->h = h;
        im->tex = evas_gl_common_texture_dynamic_new(im->gc, im);
        return image;
     }
   im_old = image;

   switch (eng_image_colorspace_get(data, image))
     {
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
         w &= ~0x1;
         break;
     }

   evas_gl_common_image_alloc_ensure(im);
   if ((im_old->im) &&
       ((int)im_old->im->cache_entry.w == w) &&
       ((int)im_old->im->cache_entry.h == h))
     return image;

   if (im_old)
     {
        im = evas_gl_common_image_new(re->win->gl_context, w, h,
                                      eng_image_alpha_get(data, image),
                                      eng_image_colorspace_get(data, image));
        evas_gl_common_image_free(im_old);
     }
   else
     im = evas_gl_common_image_new(re->win->gl_context, w, h, 
                                   1, EVAS_COLORSPACE_ARGB8888);
   return im;
}

static void *
eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   if (!(re = (Render_Engine *)data)) return NULL;
   if (!(im = image)) return NULL;
   if (im->native.data) return image;
   eng_window_use(re->win);
   evas_gl_common_image_dirty(image, x, y, w, h);
   return image;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data, int *err)
{
   Render_Engine *re;
   Evas_GL_Image *im;
   int error;

   if (!(re = (Render_Engine *)data)) return NULL;
   if (!image)
     {
        *image_data = NULL;
        if (err) *err = EVAS_LOAD_ERROR_GENERIC;
        return NULL;
     }
   im = image;
   if (im->native.data)
     {
        *image_data = NULL;
        if (err) *err = EVAS_LOAD_ERROR_NONE;
        return im;
     }

#ifdef GL_GLES
   eng_window_use(re->win);

   if ((im->tex) && (im->tex->pt) && (im->tex->pt->dyn.img) && 
       (im->cs.space == EVAS_COLORSPACE_ARGB8888))
     {
        if (im->tex->pt->dyn.checked_out > 0)
          {
             im->tex->pt->dyn.checked_out++;
             *image_data = im->tex->pt->dyn.data;
             if (err) *err = EVAS_LOAD_ERROR_NONE;
             return im;
          }
        *image_data = im->tex->pt->dyn.data = 
          glsym_eglMapImageSEC(re->win->egl_disp, 
                               im->tex->pt->dyn.img, 
                               EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC, 
                               EGL_MAP_GL_TEXTURE_OPTION_WRITE_SEC);

        if (!im->tex->pt->dyn.data)
          {
             if (err) *err = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             return im;
          }
        im->tex->pt->dyn.checked_out++;

        if (err) *err = EVAS_LOAD_ERROR_NONE;
        return im;
     }
#endif

   /* Engine can be fail to create texture after cache drop like eng_image_content_hint_set function,
    so it is need to add code which check im->im's NULL value*/

   if (!im->im)
     {
        *image_data = NULL;
        if (err) *err = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }

   error = evas_cache_image_load_data(&im->im->cache_entry);
   evas_gl_common_image_alloc_ensure(im);
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
         if (to_write)
           {
              if (im->references > 1)
                {
                   Evas_GL_Image *im_new;

                   im_new = evas_gl_common_image_new_from_copied_data
                     (im->gc, im->im->cache_entry.w, im->im->cache_entry.h,
                         im->im->image.data,
                         eng_image_alpha_get(data, image),
                         eng_image_colorspace_get(data, image));
                   if (!im_new)
                     {
                        *image_data = NULL;
                        if (err) 
                          *err = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                        return NULL;
                     }
                   evas_gl_common_image_free(im);
                   im = im_new;
                }
              else
                evas_gl_common_image_dirty(im, 0, 0, 0, 0);
           }
         *image_data = im->im->image.data;
         break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
         *image_data = im->cs.data;
         break;
      default:
         abort();
         break;
     }
   if (err) *err = error;
   return im;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   Render_Engine *re;
   Evas_GL_Image *im, *im2;

   if (!(re = (Render_Engine *)data)) return NULL;
   if (!(im = image)) return NULL;
   if (im->native.data) return image;
   eng_window_use(re->win);
   evas_gl_common_image_alloc_ensure(im);
   if ((im->tex) && (im->tex->pt)
       && (im->tex->pt->dyn.data)
       && (im->cs.space == EVAS_COLORSPACE_ARGB8888))
     {
        if (im->tex->pt->dyn.data == image_data)
          {
             if (im->tex->pt->dyn.checked_out > 0)
               {
                 im->tex->pt->dyn.checked_out--;
#ifdef GL_GLES
                 if (im->tex->pt->dyn.checked_out == 0)
                   glsym_eglUnmapImageSEC(re->win->egl_disp, 
                                          im->tex->pt->dyn.img, 
                                          EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC);
#endif
               }

             return image;
          }
        im2 = eng_image_new_from_data(data, im->w, im->h, image_data,
                                      eng_image_alpha_get(data, image),
                                      eng_image_colorspace_get(data, image));
        if (!im2) return im;
        evas_gl_common_image_free(im);
        im = im2;
        evas_gl_common_image_dirty(im, 0, 0, 0, 0);
        return im;
     }
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
         if ((!im->im) || (image_data != im->im->image.data))
           {
              im2 = eng_image_new_from_data(data, im->w, im->h, image_data,
                                            eng_image_alpha_get(data, image),
                                            eng_image_colorspace_get(data, image));
              if (!im2) return im;
              evas_gl_common_image_free(im);
              im = im2;
              evas_gl_common_image_dirty(im, 0, 0, 0, 0);
           }
         break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
         if (image_data != im->cs.data)
           {
              if (im->cs.data)
                {
                   if (!im->cs.no_free) free(im->cs.data);
                }
              im->cs.data = image_data;
           }
         evas_gl_common_image_dirty(im, 0, 0, 0, 0);
         break;
      default:
         abort();
         break;
     }
   return im;
}

static void
eng_image_data_preload_request(void *data, void *image, const Eo *target)
{
   Evas_GL_Image *gim;
   RGBA_Image *im;
   Render_Engine *re;

   if (!(gim = image)) return;
   if (gim->native.data) return;
   im = (RGBA_Image *)gim->im;
   if (!im) return;
   evas_cache_image_preload_data(&im->cache_entry, target, NULL, NULL, NULL);
   if (!(re = (Render_Engine *)data)) return;
   if (!gim->tex)
     gim->tex = evas_gl_common_texture_new(re->win->gl_context, gim->im);
   evas_gl_preload_target_register(gim->tex, (Eo *)target);
}

static void
eng_image_data_preload_cancel(void *data EINA_UNUSED, void *image, const Eo *target)
{
   Evas_GL_Image *gim;
   RGBA_Image *im;

   if (!(gim = image)) return;
   if (gim->native.data) return;
   im = (RGBA_Image *)gim->im;
   if (!im) return;
   evas_cache_image_preload_cancel(&im->cache_entry, target);
   evas_gl_preload_target_unregister(gim->tex, (Eo *)target);
}

static void *
eng_image_alpha_set(void *data, void *image, int has_alpha)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   if (!(re = (Render_Engine *)data)) return NULL;
   if (!(im = image)) return NULL;
   if (im->alpha == has_alpha) return image;
   if (im->native.data)
     {
        im->alpha = has_alpha;
        return image;
     }
   eng_window_use(re->win);
   if ((im->tex) && (im->tex->pt->dyn.img))
     {
        im->alpha = has_alpha;
        im->tex->alpha = im->alpha;
        return image;
     }
   /* FIXME: can move to gl_common */
   if (im->cs.space != EVAS_COLORSPACE_ARGB8888) return im;
   if ((has_alpha) && (im->im->cache_entry.flags.alpha)) return image;
   else if ((!has_alpha) && (!im->im->cache_entry.flags.alpha)) return image;
   if (im->references > 1)
     {
        Evas_GL_Image *im_new;

        if (!im->im->image.data)
          evas_cache_image_load_data(&im->im->cache_entry);
        evas_gl_common_image_alloc_ensure(im);
        im_new = evas_gl_common_image_new_from_copied_data
          (im->gc, im->im->cache_entry.w, im->im->cache_entry.h,
              im->im->image.data,
              eng_image_alpha_get(data, image),
              eng_image_colorspace_get(data, image));
        if (!im_new) return im;
        evas_gl_common_image_free(im);
        im = im_new;
     }
   else
     evas_gl_common_image_dirty(im, 0, 0, 0, 0);
   return evas_gl_common_image_alpha_set(im, has_alpha ? 1 : 0);
//   im->im->cache_entry.flags.alpha = has_alpha ? 1 : 0;
//   return image;
}

static int
eng_image_alpha_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *im;

   if (!(im = image)) return 1;
   return im->alpha;
}

static void *
eng_image_border_set(void *data EINA_UNUSED, void *image, int l EINA_UNUSED, int r EINA_UNUSED, int t EINA_UNUSED, int b EINA_UNUSED)
{
   return image;
}

static void
eng_image_border_get(void *data EINA_UNUSED, void *image EINA_UNUSED, int *l EINA_UNUSED, int *r EINA_UNUSED, int *t EINA_UNUSED, int *b EINA_UNUSED)
{
}

static Eina_Bool
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth, Eina_Bool do_async EINA_UNUSED)
{
   Render_Engine *re;
   Evas_GL_Image *im;
   Native *n;

   if (!(re = (Render_Engine *)data)) return EINA_FALSE;
   if (!(im = image)) return EINA_FALSE;
   n = im->native.data;

   if ((n) && (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL) &&
       (n->ns.data.opengl.framebuffer_id == 0) &&
       (re->func.pixels_get))
     {
        DBG("Rendering Directly to the window: %p", data);

        re->win->gl_context->dc = context;

        // Set necessary info for direct rendering
        evgl_direct_info_set(re->win->gl_context->w,
                             re->win->gl_context->h,
                             re->win->gl_context->rot,
                             dst_x, dst_y, dst_w, dst_h,
                             re->win->gl_context->dc->clip.x,
                             re->win->gl_context->dc->clip.y,
                             re->win->gl_context->dc->clip.w,
                             re->win->gl_context->dc->clip.h);

        // Call pixel get function
        re->func.pixels_get(re->func.pixels_data_get, re->func.obj);

        // Clear direct rendering info
        evgl_direct_info_clear();
     }
   else
     {
        eng_window_use(re->win);
        evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
        re->win->gl_context->dc = context;
        evas_gl_common_image_draw(re->win->gl_context, image,
                                  src_x, src_y, src_w, src_h,
                                  dst_x, dst_y, dst_w, dst_h,
                                  smooth);
     }

   return EINA_FALSE;
}

static char *
eng_image_comment_get(void *data EINA_UNUSED, void *image, char *key EINA_UNUSED)
{
   Evas_GL_Image *im;

   if (!(im = image)) return NULL;
   if (!im->im) return NULL;
   return im->im->info.comment;
}

static char *
eng_image_format_get(void *data EINA_UNUSED, void *image EINA_UNUSED)
{
   return NULL;
}

static void
eng_image_colorspace_set(void *data, void *image, int cspace)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   if (!(re = (Render_Engine *)data)) return;
   if (!(im = image)) return;
   if (im->native.data) return;
   /* FIXME: can move to gl_common */
   if (im->cs.space == cspace) return;
   eng_window_use(re->win);
   evas_gl_common_image_alloc_ensure(im);
   evas_cache_image_colorspace(&im->im->cache_entry, cspace);
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
         if (im->cs.data)
           {
              if (!im->cs.no_free) free(im->cs.data);
              im->cs.data = NULL;
              im->cs.no_free = 0;
           }
         break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
         if (im->tex) evas_gl_common_texture_free(im->tex, EINA_TRUE);
         im->tex = NULL;
         if (im->cs.data)
           {
              if (!im->cs.no_free) free(im->cs.data);
           }
         if (im->im->cache_entry.h > 0)
           im->cs.data =
              calloc(1, im->im->cache_entry.h * sizeof(unsigned char *) * 2);
         else
           im->cs.data = NULL;
         im->cs.no_free = 0;
         break;
      default:
         abort();
         break;
     }
   im->cs.space = cspace;
}

static int
eng_image_colorspace_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *im;

   if (!(im = image)) return EVAS_COLORSPACE_ARGB8888;
   return im->cs.space;
}

static Eina_Bool
eng_image_can_region_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim;
   Image_Entry *im;

   if (!(gim = image)) return EINA_FALSE;
   if (!(im = (Image_Entry *)gim->im)) return EINA_FALSE;
   return ((Evas_Image_Load_Func*) im->info.loader)->do_region;
}

static void *
eng_image_native_set(void *data, void *image, void *native)
{
   Render_Engine *re;
   Evas_Native_Surface *ns;
   Evas_GL_Image *im, *im2 = NULL;
   Native *n = NULL;
   unsigned int texid;
   unsigned int tex = 0;
   unsigned int fbo = 0;

   if (!(re = (Render_Engine *)data)) return NULL;

   ns = native;
   im = image;
   if (!im)
     {
        if ((ns) && (ns->type == EVAS_NATIVE_SURFACE_OPENGL))
          {
             im = evas_gl_common_image_new_from_data(re->win->gl_context,
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
        if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
          {
             tex = ns->data.opengl.texture_id;
             fbo = ns->data.opengl.framebuffer_id;
             if (im->native.data)
               {
                  Evas_Native_Surface *ens;

                  ens = im->native.data;
                  if ((ens->data.opengl.texture_id == tex) &&
                      (ens->data.opengl.framebuffer_id == fbo))
                    return im;
               }
          }
     }

   if ((!ns) && (!im->native.data)) return im;

   eng_window_use(re->win);

   if (im->native.data)
     {
        if (im->native.func.free)
          im->native.func.free(im->native.func.data, im);
        evas_gl_common_image_native_disable(im);
     }

   if (!ns) return im;

   if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
     {
        texid = tex;
        im2 = eina_hash_find(re->win->gl_context->shared->native_tex_hash, &texid);
        if (im2 == im) return im;
        if (im2)
          {
             n = im2->native.data;
             if (n)
               {
                  evas_gl_common_image_ref(im2);
                  evas_gl_common_image_free(im);
                  return im2;
               }
          }
     }

   im2 = evas_gl_common_image_new_from_data(re->win->gl_context,
                                            im->w, im->h, NULL, im->alpha,
                                            EVAS_COLORSPACE_ARGB8888);
   evas_gl_common_image_free(im);
   if (!(im = im2)) return NULL;

   if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
     {
        if (native)
          {
             n = calloc(1, sizeof(Native));
             if (n)
               {
                  memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));

                  eina_hash_add(re->win->gl_context->shared->native_tex_hash, 
                                &texid, im);

                  n->egl_surface = 0;
                  im->native.yinvert = 0;
                  im->native.loose = 0;
                  im->native.data = n;
                  im->native.func.data = re;
                  im->native.func.bind = _native_bind_cb;
                  im->native.func.unbind = _native_unbind_cb;
                  im->native.func.free = _native_free_cb;
                  im->native.target = GL_TEXTURE_2D;
                  im->native.mipmap = 0;

                  // FIXME: need to implement mapping sub texture regions
                  // x, y, w, h for possible texture atlasing

                  evas_gl_common_image_native_enable(im);
               }
          }
    }

   return im;
}

static void *
eng_image_native_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *im;
   Native *n;

   if (!(im = image)) return NULL;
   if (!(n = im->native.data)) return NULL;
   return &(n->ns);
}

static void
eng_image_scale_hint_set(void *data EINA_UNUSED, void *image, int hint)
{
   if (image) evas_gl_common_image_scale_hint_set(image, hint);
}

static int
eng_image_scale_hint_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim;

   if (!(gim = image)) return EVAS_IMAGE_SCALE_HINT_NONE;
   return gim->scale_hint;
}

static void
eng_image_stride_get(void *data EINA_UNUSED, void *image, int *stride)
{
   Evas_GL_Image *im;

   if (!(im = image)) return;
   if ((im->tex) && (im->tex->pt->dyn.img))
     {
        if (stride) *stride = im->tex->pt->dyn.stride;
     }
   else
     {
        if (stride) *stride = im->w * 4;
     }
}

static void
eng_image_content_hint_set(void *data, void *image, int hint)
{
   Render_Engine *re;
   re = (Render_Engine *)data;

   if (re) eng_window_use(re->win);
   if (image) evas_gl_common_image_content_hint_set(image, hint);
}

static int
eng_image_content_hint_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim;

   if (!(gim = image)) return EVAS_IMAGE_CONTENT_HINT_NONE;
   return gim->content_hint;
}

static void
eng_image_cache_flush(void *data)
{
   Render_Engine *re;
   int tmp_size;

   re = (Render_Engine *)data;

   if (re) eng_window_use(re->win);

   tmp_size = evas_common_image_get_cache();
   evas_common_image_set_cache(0);
   evas_common_rgba_image_scalecache_flush();
   evas_gl_common_image_cache_flush(re->win->gl_context);
//   if ((re = (Render_Engine *)data))
//     evas_gl_common_image_cache_flush(re->win->gl_context);
   evas_common_image_set_cache(tmp_size);
}

static void
eng_image_cache_set(void *data, int bytes)
{
   Render_Engine *re;

   re = (Render_Engine *)data;

   if (re) eng_window_use(re->win);

   evas_common_image_set_cache(bytes);
   evas_common_rgba_image_scalecache_size_set(bytes);
   evas_gl_common_image_cache_flush(re->win->gl_context);
//   if (!(re = (Render_Engine *)data)) return;
//   if (re->win) evas_gl_common_image_cache_flush(re->win->gl_context);
}

static int
eng_image_cache_get(void *data EINA_UNUSED)
{
   return evas_common_image_get_cache();
}

static int
eng_image_load_error_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *im;

   if (!(im = image)) return EVAS_LOAD_ERROR_NONE;
   return im->im->cache_entry.load_error;
}

static void
eng_image_max_size_get(void *data, int *maxw, int *maxh)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return;
   if (maxw) *maxw = re->win->gl_context->shared->info.max_texture_size;
   if (maxh) *maxh = re->win->gl_context->shared->info.max_texture_size;
}

static Eina_Bool
eng_image_map_draw(void *data, void *context, void *surface, void *image, RGBA_Map *m, int smooth, int level, Eina_Bool do_async)
{
   Evas_GL_Image *gim;
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return EINA_FALSE;
   if (!(gim = image)) return EINA_FALSE;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   if (m->count != 4)
     {
        // FIXME: nash - you didn't fix this
        abort();
     }
   if ((m->pts[0].x == m->pts[3].x) &&
       (m->pts[1].x == m->pts[2].x) &&
       (m->pts[0].y == m->pts[1].y) &&
       (m->pts[3].y == m->pts[2].y) &&
       (m->pts[0].x <= m->pts[1].x) &&
       (m->pts[0].y <= m->pts[2].y) &&
       (m->pts[0].u == 0) &&
       (m->pts[0].v == 0) &&
       (m->pts[1].u == (gim->w << FP)) &&
       (m->pts[1].v == 0) &&
       (m->pts[2].u == (gim->w << FP)) &&
       (m->pts[2].v == (gim->h << FP)) &&
       (m->pts[3].u == 0) &&
       (m->pts[3].v == (gim->h << FP)) &&
       (m->pts[0].col == 0xffffffff) &&
       (m->pts[1].col == 0xffffffff) &&
       (m->pts[2].col == 0xffffffff) &&
       (m->pts[3].col == 0xffffffff))
     {
        int dx, dy, dw, dh;

        dx = m->pts[0].x >> FP;
        dy = m->pts[0].y >> FP;
        dw = (m->pts[2].x >> FP) - dx;
        dh = (m->pts[2].y >> FP) - dy;
        eng_image_draw(data, context, surface, image,
                       0, 0, gim->w, gim->h, dx, dy, dw, dh, smooth, do_async);
     }
   else
     evas_gl_common_image_map_draw(re->win->gl_context, image, m->count, 
                                   &m->pts[0], smooth, level);

   return EINA_FALSE;
}

static void *
eng_image_map_surface_new(void *data, int w, int h, int alpha)
{
   Render_Engine *re;

   if (!(re = (Render_Engine *)data)) return NULL;
   eng_window_use(re->win);
   return evas_gl_common_image_surface_new(re->win->gl_context, w, h, alpha);
}

static void
eng_image_map_surface_free(void *data, void *surface)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_image_free(surface);
}

static void
eng_image_map_clean(void *data EINA_UNUSED, RGBA_Map *m EINA_UNUSED)
{
}

static Eina_Bool
eng_image_animated_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim;
   Image_Entry *im;

   if (!(gim = image)) return EINA_FALSE;
   if (!(im = (Image_Entry *)gim->im)) return EINA_FALSE;
   return im->animated.animated;
}

static int
eng_image_animated_frame_count_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim;
   Image_Entry *im;

   if (!(gim = image)) return -1;
   if (!(im = (Image_Entry *)gim->im)) return -1;
   if (!im->animated.animated) return -1;
   return im->animated.frame_count;
}

static Evas_Image_Animated_Loop_Hint
eng_image_animated_loop_type_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim;
   Image_Entry *im;

   if (!(gim = image)) return EVAS_IMAGE_ANIMATED_HINT_NONE;
   if (!(im = (Image_Entry *)gim->im)) return EVAS_IMAGE_ANIMATED_HINT_NONE;
   if (!im->animated.animated) return EVAS_IMAGE_ANIMATED_HINT_NONE;
   return im->animated.loop_hint;
}

static int
eng_image_animated_loop_count_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim;
   Image_Entry *im;

   if (!(gim = image)) return -1;
   if (!(im = (Image_Entry *)gim->im)) return -1;
   if (!im->animated.animated) return -1;
   return im->animated.loop_count;
}

static double
eng_image_animated_frame_duration_get(void *data EINA_UNUSED, void *image, int start_frame, int frame_num)
{
   Evas_GL_Image *gim;
   Image_Entry *im;

   if (!(gim = image)) return -1;
   if (!(im = (Image_Entry *)gim->im)) return -1;
   if (!im->animated.animated) return -1;
   return evas_common_load_rgba_image_frame_duration_from_file(im, start_frame, frame_num);
}

static Eina_Bool
eng_image_animated_frame_set(void *data EINA_UNUSED, void *image, int frame_index)
{
   Evas_GL_Image *gim;
   Image_Entry *im;

   if (!(gim = image)) return EINA_FALSE;
   if (!(im = (Image_Entry *)gim->im)) return EINA_FALSE;
   if (!im->animated.animated) return EINA_FALSE;
   if (im->animated.cur_frame == frame_index) return EINA_FALSE;

   im->animated.cur_frame = frame_index;
   return EINA_TRUE;
}

static Eina_Bool
eng_pixel_alpha_get(void *image, int x, int y, DATA8 *alpha, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h)
{
   Evas_GL_Image *im;
   int px, py, dx, dy, sx, sy, src_w, src_h;
   double scale_w, scale_h;

   if (!(im = image)) return EINA_FALSE;

   if ((dst_region_x > x) || (x >= (dst_region_x + dst_region_w)) ||
       (dst_region_y > y) || (y >= (dst_region_y + dst_region_h)))
     {
        *alpha = 0;
        return EINA_FALSE;
     }

   evas_gl_common_image_alloc_ensure(im);
   src_w = im->im->cache_entry.w;
   src_h = im->im->cache_entry.h;
   if ((src_w == 0) || (src_h == 0))
     {
        *alpha = 0;
        return EINA_TRUE;
     }

   EINA_SAFETY_ON_TRUE_GOTO(src_region_x < 0, error_oob);
   EINA_SAFETY_ON_TRUE_GOTO(src_region_y < 0, error_oob);
   EINA_SAFETY_ON_TRUE_GOTO(src_region_x + src_region_w > src_w, error_oob);
   EINA_SAFETY_ON_TRUE_GOTO(src_region_y + src_region_h > src_h, error_oob);

   scale_w = (double)dst_region_w / (double)src_region_w;
   scale_h = (double)dst_region_h / (double)src_region_h;

   /* point at destination */
   dx = x - dst_region_x;
   dy = y - dst_region_y;

   /* point at source */
   sx = dx / scale_w;
   sy = dy / scale_h;

   /* pixel point (translated) */
   px = src_region_x + sx;
   py = src_region_y + sy;
   EINA_SAFETY_ON_TRUE_GOTO(px >= src_w, error_oob);
   EINA_SAFETY_ON_TRUE_GOTO(py >= src_h, error_oob);

   switch (im->im->cache_entry.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
          {
             DATA32 *pixel;

             evas_cache_image_load_data(&im->im->cache_entry);
             if (!im->im->cache_entry.flags.loaded)
               {
                  ERR("im %p has no pixels loaded yet", im);
                  return EINA_FALSE;
               }

             pixel = im->im->image.data;
             pixel += ((py * src_w) + px);
             *alpha = ((*pixel) >> 24) & 0xff;
          }
        break;

      default:
        ERR("Colorspace %d not supported.", im->im->cache_entry.space);
        *alpha = 0;
     }

   return EINA_TRUE;

error_oob:
   ERR("Invalid region src=(%d, %d, %d, %d), dst=(%d, %d, %d, %d), image=%dx%d",
       src_region_x, src_region_y, src_region_w, src_region_h,
       dst_region_x, dst_region_y, dst_region_w, dst_region_h,
       src_w, src_h);
   *alpha = 0;
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
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;

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
   ORD(output_tile_size_set);
   ORD(output_redraws_rect_add);
   ORD(output_redraws_rect_del);
   ORD(output_redraws_clear);
   ORD(output_redraws_next_update_get);
   ORD(output_redraws_next_update_push);
   ORD(output_flush);
   ORD(output_idle_flush);
   ORD(output_dump);

   ORD(context_cutout_add);
   ORD(context_cutout_clear);
   ORD(context_flush);

   ORD(gl_surface_create);
   ORD(gl_surface_destroy);
   ORD(gl_context_create);
   ORD(gl_context_destroy);
   ORD(gl_make_current);
   ORD(gl_string_query);
   ORD(gl_proc_address_get);
   ORD(gl_native_surface_get);
   ORD(gl_api_get);
   ORD(gl_direct_override_get);
   ORD(gl_get_pixels_set);

   ORD(rectangle_draw);
   ORD(line_draw);
   ORD(font_draw);

   ORD(polygon_draw);
   ORD(polygon_point_add);
   ORD(polygon_points_clear);

   ORD(image_load);
   ORD(image_mmap);
   ORD(image_new_from_data);
   ORD(image_new_from_copied_data);
   ORD(image_free);
   ORD(image_size_get);
   ORD(image_size_set);
   ORD(image_dirty_region);
   ORD(image_data_get);
   ORD(image_data_put);
   ORD(image_data_preload_request);
   ORD(image_data_preload_cancel);
   ORD(image_alpha_set);
   ORD(image_alpha_get);
   ORD(image_border_set);
   ORD(image_border_get);
   ORD(image_draw);
   ORD(image_comment_get);
   ORD(image_format_get);
   ORD(image_colorspace_set);
   ORD(image_colorspace_get);
   ORD(image_can_region_get);
   ORD(image_native_set);
   ORD(image_native_get);
   ORD(image_scale_hint_set);
   ORD(image_scale_hint_get);
   ORD(image_stride_get);
   ORD(image_content_hint_set);
   ORD(image_content_hint_get);
   ORD(image_cache_flush);
   ORD(image_cache_set);
   ORD(image_cache_get);
   ORD(image_load_error_get);
   ORD(image_max_size_get);

   ORD(image_map_draw);
   ORD(image_map_surface_new);
   ORD(image_map_surface_free);
   ORD(image_map_clean);

   ORD(image_animated_get);
   ORD(image_animated_frame_count_get);
   ORD(image_animated_loop_type_get);
   ORD(image_animated_loop_count_get);
   ORD(image_animated_frame_duration_get);
   ORD(image_animated_frame_set);

   ORD(pixel_alpha_get);

   /* advertise out which functions we support */
   em->functions = (void *)(&func);

   return 1;
}

static void 
module_close(Evas_Module *em EINA_UNUSED)
{
   eina_log_domain_unregister(_evas_engine_wl_egl_log_dom);
   evas_gl_common_module_close();
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION, "wayland_egl", "none", {module_open, module_close}
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, wayland_egl);

#ifndef EVAS_STATIC_BUILD_WAYLAND_EGL
EVAS_EINA_MODULE_DEFINE(engine, wayland_egl);
#endif
