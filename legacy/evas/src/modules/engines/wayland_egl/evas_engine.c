#include "evas_common.h"
#include "evas_engine.h"

#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

/* local structures */
typedef struct _Render_Engine Render_Engine;
struct _Render_Engine 
{
   Tilebuf *tb;
   Outbuf *ob;

   Eina_Bool end : 1;

   void (*outbuf_free)(Outbuf *ob);
   void (*outbuf_resize)(Outbuf *ob, int w, int h);
};

/* local function prototypes */
static void *_output_setup(struct wl_display *disp, struct wl_compositor *comp, struct wl_shell *shell, int w, int h, int rotation);

/* engine function prototypes */
static void *eng_info(Evas *evas __UNUSED__);
static void eng_info_free(Evas *evas __UNUSED__, void *info);
static int eng_setup(Evas *evas, void *info);
static void eng_output_free(void *data);
static void eng_output_resize(void *data, int w, int h);
static void eng_output_tile_size_set(void *data, int w, int h);
static void eng_output_redraws_rect_add(void *data, int x, int y, int w, int h);
static void eng_output_redraws_rect_del(void *data, int x, int y, int w, int h);
static void eng_output_redraws_clear(void *data);
static void *eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch);
static void eng_output_redraws_next_update_push(void *data, void *surface __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__);
static void eng_output_flush(void *data);
static void eng_output_idle_flush(void *data);
static void eng_output_dump(void *data);
static void *eng_gl_api_get(void *data);
static const GLubyte *evgl_glGetString(GLenum name);

/* local variables */
static Evas_Func func, pfunc;
static Evas_GL_API gl_funcs;

/* external variables */
int _evas_engine_way_egl_log_dom = -1;

/* local functions */
static void *
_output_setup(struct wl_display *disp, struct wl_compositor *comp, struct wl_shell *shell, int w, int h, int rotation) 
{
   Render_Engine *re = NULL;

   if (!(re = calloc(1, sizeof(Render_Engine)))) return NULL;

   if (!(re->ob = evas_outbuf_setup(disp, comp, shell, w, h, rotation))) 
     {
        free(re);
        return NULL;
     }

   if (!(re->tb = evas_common_tilebuf_new(w, h))) 
     {
        evas_outbuf_free(re->ob);
        free(re);
        return NULL;
     }

   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
   return re;
}

/* engine functions */
static void *
eng_info(Evas *evas __UNUSED__) 
{
   Evas_Engine_Info_Wayland_Egl *info;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(info = calloc(1, sizeof(Evas_Engine_Info_Wayland_Egl))))
     return NULL;

   info->magic.magic = rand();
   info->info.debug = EINA_FALSE;
   info->render_mode = EVAS_RENDER_MODE_BLOCKING;

   return info;
}

static void 
eng_info_free(Evas *evas __UNUSED__, void *info) 
{
   Evas_Engine_Info_Wayland_Egl *in;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(in = (Evas_Engine_Info_Wayland_Egl *)info)) return;
   free(in);
}

static int 
eng_setup(Evas *evas, void *info) 
{
   Evas_Engine_Info_Wayland_Egl *in;
   Render_Engine *re = NULL;

   if (!(in = (Evas_Engine_Info_Wayland_Egl *)info)) return 0;

   if (!evas->engine.data.output) 
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

        re = _output_setup(in->info.disp, in->info.comp, in->info.shell, 
                           evas->output.w, evas->output.h, 
                           in->info.rotation);
        if (!re) return 0;

        re->outbuf_free = evas_outbuf_free;
        re->outbuf_resize = evas_outbuf_resize;
     }
   else 
     {
        if (!(re = (Render_Engine *)evas->engine.data.output)) return 0;
        if (re->ob) re->outbuf_free(re->ob);
        re->ob = evas_outbuf_setup(in->info.disp, in->info.comp, in->info.shell, 
                                   evas->output.w, evas->output.h, 
                                   in->info.rotation);
        if (re->tb) evas_common_tilebuf_free(re->tb);
        if ((re->tb = evas_common_tilebuf_new(evas->output.w, evas->output.h)))
          evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
     }

   evas->engine.data.output = re;
   if (!evas->engine.data.output) 
     {
        if (re) free(re);
        return 0;
     }

   if (!evas->engine.data.context) 
     {
        evas->engine.data.context = 
          evas->engine.func->context_new(evas->engine.data.output);
     }

   return 1;
}

static void 
eng_output_free(void *data) 
{
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((re = (Render_Engine *)data)) 
     {
        if (re->ob) re->outbuf_free(re->ob);
        if (re->tb) evas_common_tilebuf_free(re->tb);
        free(re);
     }
   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static void 
eng_output_resize(void *data, int w, int h) 
{
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(re = (Render_Engine *)data)) return;
   if (re->ob) re->outbuf_resize(re->ob, w, h);
   if (re->tb) evas_common_tilebuf_free(re->tb);
   if ((re->tb = evas_common_tilebuf_new(w, h)))
     evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
}

static void 
eng_output_tile_size_set(void *data, int w, int h) 
{
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

static void 
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h) 
{
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(re = (Render_Engine *)data)) return;
   if (re->ob->priv.gl.context) 
     evas_gl_common_context_resize(re->ob->priv.gl.context, 
                                   w, h, re->ob->rot);
   if (re->tb) evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);
}

static void 
eng_output_redraws_rect_del(void *data, int x, int y, int w, int h) 
{
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

static void 
eng_output_redraws_clear(void *data) 
{
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(re = (Render_Engine *)data)) return;
   if (re->tb) evas_common_tilebuf_clear(re->tb);
}

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch) 
{
   Render_Engine *re = NULL;
   Tilebuf_Rect *rects;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(re = (Render_Engine *)data)) return NULL;
   if ((rects = evas_common_tilebuf_get_render_rects(re->tb))) 
     {
        evas_common_tilebuf_free_render_rects(rects);
        evas_common_tilebuf_clear(re->tb);
        if (re->ob->priv.gl.context) 
          {
             evas_gl_common_context_flush(re->ob->priv.gl.context);
             evas_gl_common_context_newframe(re->ob->priv.gl.context);
          }
        if (x) *x = 0;
        if (y) *y = 0;
        if (w) *w = re->ob->w;
        if (h) *h = re->ob->h;
        if (cx) *cx = 0;
        if (cy) *cy = 0;
        if (cw) *cw = re->ob->w;
        if (ch) *ch = re->ob->h;
        return re->ob->priv.gl.context->def_surface;
     }

   /* NB: Fixme: This may need to return the wayland surface */
   return NULL;
}

static void 
eng_output_redraws_next_update_push(void *data, void *surface __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__) 
{
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(re = (Render_Engine *)data)) return;
   if (re->ob->priv.gl.context)
     evas_gl_common_context_flush(re->ob->priv.gl.context);

   eglWaitNative(EGL_CORE_NATIVE_ENGINE);
}

static void 
eng_output_flush(void *data) 
{
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(re = (Render_Engine *)data)) return;

   eglSwapInterval(re->ob->priv.egl.disp, 0);

   /* TODO: callback pre swap */

   eglSwapBuffers(re->ob->priv.egl.disp, re->ob->priv.egl.surface);
   eglWaitGL();

   /* TODO: Callback post swap */
}

static void 
eng_output_idle_flush(void *data) 
{
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(re = (Render_Engine *)data)) return;
}

static void 
eng_output_dump(void *data) 
{
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(re = (Render_Engine *)data)) return;
   evas_common_image_image_all_unload();
   evas_common_font_font_all_unload();
   evas_gl_common_image_all_unload(re->ob->priv.gl.context);
}

static void *
eng_gl_api_get(void *data) 
{
   Render_Engine *re = NULL;

   re = (Render_Engine *)data;

   gl_funcs.version = EVAS_GL_API_VERSION;

#define ORD(f) EVAS_API_OVERRIDE(f, &gl_funcs, evgl_)
   ORD(glGetString);
#undef ORD

   return &gl_funcs;
}

static const GLubyte *
evgl_glGetString(GLenum name) 
{
   if (name == GL_EXTENSIONS) 
     return (GLubyte *)glGetString(GL_EXTENSIONS);
   else
     return glGetString(name);
}

/* module functions */
static int 
module_open(Evas_Module *em) 
{
   if (!em) return 0;
   if (!evas_gl_common_module_open()) return 0;

   if (!_evas_module_engine_inherit(&pfunc, "software_generic"))
     return 0;

   _evas_engine_way_egl_log_dom = 
     eina_log_domain_register("evas-wayland_egl", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_way_egl_log_dom < 0) 
     {
        EINA_LOG_ERR("Could not create module log domain.");
        return 0;
     }

   func = pfunc;

#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(info);
   ORD(info_free);
   ORD(setup);
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
   ORD(gl_api_get);
#undef ORD

   em->functions = (void *)(&func);
   return 1;
}

static void 
module_close(Evas_Module *em __UNUSED__) 
{
   eina_log_domain_unregister(_evas_engine_way_egl_log_dom);
}

static Evas_Module_Api evas_modapi = 
{
   EVAS_MODULE_API_VERSION, "wayland_egl", "none", {module_open, module_close}
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, wayland_egl);

#ifndef EVAS_STATIC_BUILD_WAYLAND_EGL
EVAS_EINA_MODULE_DEFINE(engine, wayland_egl);
#endif
