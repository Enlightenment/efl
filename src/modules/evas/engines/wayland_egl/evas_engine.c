#include "evas_common.h"
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
   Evas *Evas;

   int w, h;
   int end, mode, vsync;
   int lost_back, prev_age;

   Eina_Bool evgl_initted : 1;
};

/* local function prototypes */
typedef void (*_eng_fn) (void);
typedef _eng_fn (*glsym_func_eng_fn) ();
typedef void (*glsym_func_void) ();
typedef void *(*glsym_func_void_ptr) ();
typedef int (*glsym_func_int) ();
typedef unsigned int (*glsym_func_uint) ();
typedef const char *(*glsym_func_const_char_ptr) ();

static int evgl_init(Render_Engine *re);
#define EVGLINIT(_re, _ret) if (!evgl_init(_re)) return _ret;

/* local variables */
static int initted = 0;
static int gl_wins = 0;
static int have_buffer_age = 1;

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

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
const char *(*glsym_eglQueryString) (EGLDisplay a, int name) = NULL;
void (*glsym_eglSwapBuffersRegion) (EGLDisplay a, void *b, EGLint c, const EGLint *d) = NULL;

#endif

/* local functions */

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

   inf = (Evas_Engine_Info_Wayland_Egl *)info;
   epd = eo_data_scope_get(evas, EVAS_CLASS);

   /* check for existing engine output */
   if (!epd->engine.data.output)
     {
        /* try to allocate space for a new render engine */
        if (!(re = calloc(1, sizeof(Render_Engine))))
          return 0;

        /* set some properties of the render engine */
        re->info = inf;
        re->evas = evas;
        re->w = epd->output.w;
        re->h = epd->output.h;

        /* try to create a new window */
        re->win = eng_window_new(inf->display, inf->surface, inf->screen, 
                                 inf->depth, re->w, re->h, inf->indirect, 
                                 inf->destination_alpha, inf->rotation);
        if (!re->win)
          {
             free(re);
             return 0;
          }

        /* tell the engine to use this render_engine for output */
        epd->engine.data.output = re;
        gl_wins++;

        /* if we have not initialize gl & evas, do it */
        if (!initted)
          {
             gl_symbols();
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
             gl_extn_veto(re);
             initted = 1;
          }
     }
   else
     {
        re = epd->engine.data.output;
        if (_re_wincheck(re))
          {
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
                                           re->w, re->h, 
                                           re->info->info.indirect, 
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
        switch (info->swap_mode)
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
        return 0;
     }

   re->tb = evas_common_tilebuf_new(re->win->w, re->win->h);
   if (!re->tb)
     {
        if (re->win)
          {
             eng_window_free(re->win);
             gl_wins--;
          }
        free(re);
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
