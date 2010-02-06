#include "evas_engine.h"

#include <dlfcn.h>      /* dlopen,dlclose,etc */

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
// EGL / GLES
# if defined(GLES_VARIETY_S3C6410)
# elif defined(GLES_VARIETY_SGX)
# endif
#else
// GLX
#endif

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)

#ifndef EGL_NATIVE_PIXMAP_KHR
# define EGL_NATIVE_PIXMAP_KHR 0x30b0
#endif
typedef void (*_eng_fn) (void);

_eng_fn  (*glsym_eglGetProcAddress)            (const char *a) = NULL;
void     (*glsym_eglBindTexImage)              (EGLDisplay a, EGLSurface b, int c) = NULL;
void     (*glsym_eglReleaseTexImage)           (EGLDisplay a, EGLSurface b, int c) = NULL;
void    *(*glsym_eglCreateImage)               (EGLDisplay a, EGLContext b, EGLenum c, EGLClientBuffer d, const int *e) = NULL;
void     (*glsym_eglDestroyImage)              (EGLDisplay a, void *b) = NULL;
void     (*glsym_glEGLImageTargetTexture2DOES) (int a, void *b)  = NULL;
#else
typedef void (*_eng_fn) (void);

_eng_fn  (*glsym_glXGetProcAddress)  (const char *a) = NULL;
void     (*glsym_glXBindTexImage)    (Display *a, GLXDrawable b, int c, int *d) = NULL;
void     (*glsym_glXReleaseTexImage) (Display *a, GLXDrawable b, int c) = NULL;
int      (*glsym_glXGetVideoSync)    (unsigned int *a) = NULL;
int      (*glsym_glXWaitVideoSync)   (int a, int b, unsigned int *c) = NULL;
#endif
                
static void
_sym_init(void)
{
   static int done = 0;
   
   
   if (done) return;
   
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
#define FINDSYM(dst, sym) \
   if ((!dst) && (glsym_eglGetProcAddress)) dst = glsym_eglGetProcAddress(sym); \
   if (!dst) dst = dlsym(RTLD_DEFAULT, sym)
   
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddress");
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressEXT");
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressARB");
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressKHR");
   
   FINDSYM(glsym_eglBindTexImage, "eglBindTexImage");
   FINDSYM(glsym_eglBindTexImage, "eglBindTexImageEXT");
   FINDSYM(glsym_eglBindTexImage, "eglBindTexImageARB");
   FINDSYM(glsym_eglBindTexImage, "eglBindTexImageKHR");
   
   FINDSYM(glsym_eglReleaseTexImage, "eglReleaseTexImage");
   FINDSYM(glsym_eglReleaseTexImage, "eglReleaseTexImageEXT");
   FINDSYM(glsym_eglReleaseTexImage, "eglReleaseTexImageARB");
   FINDSYM(glsym_eglReleaseTexImage, "eglReleaseTexImageKHR");
   
   FINDSYM(glsym_eglCreateImage, "eglCreateImage");
   FINDSYM(glsym_eglCreateImage, "eglCreateImageEXT");
   FINDSYM(glsym_eglCreateImage, "eglCreateImageARB");
   FINDSYM(glsym_eglCreateImage, "eglCreateImageKHR");

   FINDSYM(glsym_eglDestroyImage, "eglDestroyImage");
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageEXT");
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageARB");
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageKHR");

   FINDSYM(glsym_glEGLImageTargetTexture2DOES, "glEGLImageTargetTexture2DOES");
#else
#define FINDSYM(dst, sym) \
   if ((!dst) && (glsym_glXGetProcAddress)) dst = glsym_glXGetProcAddress(sym); \
   if (!dst) dst = dlsym(RTLD_DEFAULT, sym)

   FINDSYM(glsym_glXGetProcAddress, "glXGetProcAddress");
   FINDSYM(glsym_glXGetProcAddress, "glXGetProcAddressEXT");
   FINDSYM(glsym_glXGetProcAddress, "glXGetProcAddressARB");
   
   FINDSYM(glsym_glXBindTexImage, "glXBindTexImage");
   FINDSYM(glsym_glXBindTexImage, "glXBindTexImageEXT");
   FINDSYM(glsym_glXBindTexImage, "glXBindTexImageARB");

   FINDSYM(glsym_glXReleaseTexImage, "glXReleaseTexImage");
   FINDSYM(glsym_glXReleaseTexImage, "glXReleaseTexImageEXT");
   FINDSYM(glsym_glXReleaseTexImage, "glXReleaseTexImageARB");

   FINDSYM(glsym_glXGetVideoSync, "glXGetVideoSyncSGI");
   
   FINDSYM(glsym_glXWaitVideoSync, "glXWaitVideoSyncSGI");
#endif
}

int _evas_engine_GL_X11_log_dom = -1;
/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Evas_GL_X11_Window      *win;
   Evas_Engine_Info_GL_X11 *info;
   Evas                    *evas;
   int                      end;
   
   XrmDatabase   xrdb; // xres - dpi
   struct { // xres - dpi
      int        dpi; // xres - dpi
   } xr; // xres - dpi
};

static void *
eng_info(Evas *e)
{
   Evas_Engine_Info_GL_X11 *info;

   info = calloc(1, sizeof(Evas_Engine_Info_GL_X11));
   info->magic.magic = rand();
   info->func.best_visual_get = eng_best_visual_get;
   info->func.best_colormap_get = eng_best_colormap_get;
   info->func.best_depth_get = eng_best_depth_get;
   return info;
   e = NULL;
}

static void
eng_info_free(Evas *e __UNUSED__, void *info)
{
   Evas_Engine_Info_GL_X11 *in;
// dont free! why bother? its not worth it   
//   eina_log_domain_unregister(_evas_engine_GL_X11_log_dom);
   in = (Evas_Engine_Info_GL_X11 *)info;
   free(in);
}

static int
eng_setup(Evas *e, void *in)
{
   Render_Engine *re;
   Evas_Engine_Info_GL_X11 *info;

   info = (Evas_Engine_Info_GL_X11 *)in;
   if (!e->engine.data.output)
     {
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
#else        
        int eb, evb;
        
	if (!glXQueryExtension(info->info.display, &eb, &evb)) return 0;
#endif
	re = calloc(1, sizeof(Render_Engine));
	if (!re) return 0;
        re->info = info;
        re->evas = e;
	e->engine.data.output = re;
	re->win = eng_window_new(info->info.display,
				 info->info.drawable,
				 0 /* FIXME: screen 0 assumption */,
				 info->info.visual,
				 info->info.colormap,
				 info->info.depth,
				 e->output.w,
				 e->output.h);
	if (!re->win)
	  {
	     free(re);
	     e->engine.data.output = NULL;
	     return 0;
	  }
        
          {
             int status;
             char *type = NULL;
             XrmValue val;
             
             re->xr.dpi = 75000; // dpy * 1000
             re->xrdb = XrmGetDatabase(info->info.display);
             status = XrmGetResource(re->xrdb, "Xft.dpi", "Xft.Dpi", &type, &val);
             if ((status) && (type))
               {
                  if (!strcmp(type, "String"))
                    {
                       const char *str, *dp;
                       
                       str = val.addr;
                       dp = strchr(str, '.');
                       if (!dp) dp = strchr(str, ',');
                       
                       if (dp)
                         {
                            int subdpi, len, i;
                            char *buf;
                            
                            buf = alloca(dp - str + 1);
                            strncpy(buf, str, dp - str);
                            buf[dp - str] = 0;
                            len = strlen(dp + 1);
                            subdpi = atoi(dp + 1);
                            
                            if (len < 3)
                              {
                                 for (i = len; i < 3; i++) subdpi *= 10;
                              }
                            else if (len > 3)
                              {
                                 for (i = len; i > 3; i--) subdpi /= 10;
                              }
                            re->xr.dpi = atoi(buf) * 1000;
                         }
                       else
                         re->xr.dpi = atoi(str) * 1000;
                       evas_common_font_dpi_set(re->xr.dpi / 1000);
                    }
               }
          }
        
	evas_common_cpu_init();
        
	evas_common_blend_init();
	evas_common_image_init();
	evas_common_convert_init();
	evas_common_scale_init();
	evas_common_rectangle_init();
	evas_common_gradient_init();
	evas_common_polygon_init();
	evas_common_line_init();
	evas_common_font_init();
	evas_common_draw_init();
	evas_common_tilebuf_init();
     }
   else
     {
	re = e->engine.data.output;
        if ((info->info.display != re->win->disp) ||
            (info->info.drawable != re->win->win) ||
            (0 != re->win->screen) || /* FIXME: screen 0 assumption */
            (info->info.visual != re->win->visual) ||
            (info->info.colormap != re->win->colormap) ||
            (info->info.depth != re->win->depth))
          {
             eng_window_free(re->win);
             re->win = eng_window_new(info->info.display,
                                      info->info.drawable,
                                      0,/* FIXME: screen 0 assumption */
                                      info->info.visual,
                                      info->info.colormap,
                                      info->info.depth,
                                      e->output.w,
                                      e->output.h);
          }
        else if ((re->win->w != e->output.w) ||
                 (re->win->h != e->output.h))
          {
             re->win->w = e->output.w;
             re->win->h = e->output.h;
             eng_window_use(re->win);
             evas_gl_common_context_resize(re->win->gl_context, re->win->w, re->win->h);
          }
        
     }
   if (!e->engine.data.output) return 0;
   if (!e->engine.data.context)
     e->engine.data.context =
     e->engine.func->context_new(e->engine.data.output);
   eng_window_use(re->win);
   
   _sym_init();
   
   return 1;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   
//   if (re->xrdb) XrmDestroyDatabase(re->xrdb);
   
   eng_window_free(re->win);
   free(re);

   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->w = w;
   re->win->h = h;
   eng_window_use(re->win);
   evas_gl_common_context_resize(re->win->gl_context, w, h);
}

static void
eng_output_tile_size_set(void *data, int w __UNUSED__, int h __UNUSED__)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
}

static void
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_context_resize(re->win->gl_context, re->win->w, re->win->h);
   /* smple bounding box */
   if (!re->win->draw.redraw)
     {
#if 0
	re->win->draw.x1 = x;
	re->win->draw.y1 = y;
	re->win->draw.x2 = x + w - 1;
	re->win->draw.y2 = y + h - 1;
#else
	re->win->draw.x1 = 0;
	re->win->draw.y1 = 0;
	re->win->draw.x2 = re->win->w - 1;
	re->win->draw.y2 = re->win->h - 1;
#endif
     }
   else
     {
	if (x < re->win->draw.x1) re->win->draw.x1 = x;
	if (y < re->win->draw.y1) re->win->draw.y1 = y;
	if ((x + w - 1) > re->win->draw.x2) re->win->draw.x2 = x + w - 1;
	if ((y + h - 1) > re->win->draw.y2) re->win->draw.y2 = y + h - 1;
     }
   re->win->draw.redraw = 1;
}

static void
eng_output_redraws_rect_del(void *data, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
}

static void
eng_output_redraws_clear(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->draw.redraw = 0;
//   INF("GL: finish update cycle!");
}

/* at least the nvidia drivers are so abysmal that copying from the backbuffer
 * to the front using glCopyPixels() that you literally can WATCH it draw the
 * pixels slowly across the screen with a window update taking multiple
 * seconds - so workaround by doing a full buffer render as frankly GL isn't
 * up to doing anything that isn't done by quake (etc.)
 */
#define SLOW_GL_COPY_RECT 1
/* vsync games - not for now though */
#define VSYNC_TO_SCREEN 1

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_context_flush(re->win->gl_context);
   /* get the upate rect surface - return engine data as dummy */
   if (!re->win->draw.redraw)
     {
//	printf("GL: NO updates!\n");
	return NULL;
     }
//   printf("GL: update....!\n");
#ifdef SLOW_GL_COPY_RECT
   /* if any update - just return the whole canvas - works with swap
    * buffers then */
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = re->win->w;
   if (h) *h = re->win->h;
   if (cx) *cx = 0;
   if (cy) *cy = 0;
   if (cw) *cw = re->win->w;
   if (ch) *ch = re->win->h;
#else
   /* 1 update - INCREDIBLY SLOW if combined with swap_rect in flush. a gl
    * problem where there just is no hardware path for somethnig that
    * obviously SHOULD be there */
   /* only 1 update to minimise gl context games and rendering multiple update
    * regions as evas does with other engines
    */
   if (x) *x = re->win->draw.x1;
   if (y) *y = re->win->draw.y1;
   if (w) *w = re->win->draw.x2 - re->win->draw.x1 + 1;
   if (h) *h = re->win->draw.y2 - re->win->draw.y1 + 1;
   if (cx) *cx = re->win->draw.x1;
   if (cy) *cy = re->win->draw.y1;
   if (cw) *cw = re->win->draw.x2 - re->win->draw.x1 + 1;
   if (ch) *ch = re->win->draw.y2 - re->win->draw.y1 + 1;
#endif
// clear buffer. only needed for dest alpha
//   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
//   glClear(GL_COLOR_BUFFER_BIT);
//x//   printf("frame -> new\n");
   return re->win->gl_context->def_surface;
}

static void
eng_output_redraws_next_update_push(void *data, void *surface __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* put back update surface.. in this case just unflag redraw */
   re->win->draw.redraw = 0;
   re->win->draw.drew = 1;
   evas_gl_common_context_flush(re->win->gl_context);
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   // this is needed to make sure all previous rendering is flushed to
   // buffers/surfaces
   eglWaitNative(EGL_CORE_NATIVE_ENGINE); // previous rendering should be done and swapped
#else
   glXWaitGL();
#endif
//x//   printf("frame -> push\n");
}

static void
eng_output_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!re->win->draw.drew) return;
//x//   printf("frame -> flush\n");
   re->win->draw.drew = 0;
   eng_window_use(re->win);

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   eglSwapBuffers(re->win->egl_disp, re->win->egl_surface[0]);
#else
#ifdef VSYNC_TO_SCREEN   
   if (re->info->vsync) 
     {
        if ((glsym_glXGetVideoSync) && (glsym_glXWaitVideoSync))
          {
             unsigned int rc;
             
             glsym_glXGetVideoSync(&rc);
             glsym_glXWaitVideoSync(1, 0, &rc);
          }
     }
# endif
   if (re->info->callback.pre_swap)
     {
        re->info->callback.pre_swap(re->info->callback.data, re->evas);
     }
   glXSwapBuffers(re->win->disp, re->win->win);
   if (re->info->callback.post_swap)
     {
        re->info->callback.post_swap(re->info->callback.data, re->evas);
     }
#endif   
}

static void
eng_output_idle_flush(void *data)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
}

static void
eng_context_cutout_add(void *data, void *context, int x, int y, int w, int h)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
//   re->win->gl_context->dc = context;
   evas_common_draw_context_add_cutout(context, x, y, w, h);
}

static void
eng_context_cutout_clear(void *data, void *context)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
//   re->win->gl_context->dc = context;
   evas_common_draw_context_clear_cutouts(context);
}

static void
eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_rect_draw(re->win->gl_context, x, y, w, h);
}

static void
eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_line_draw(re->win->gl_context, x1, y1, x2, y2);
}

static void *
eng_polygon_point_add(void *data, void *context __UNUSED__, void *polygon, int x, int y)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_poly_point_add(polygon, x, y);
}

static void *
eng_polygon_points_clear(void *data, void *context __UNUSED__, void *polygon)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_poly_points_clear(polygon);
}

static void
eng_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_poly_draw(re->win->gl_context, polygon);
}

static void
eng_gradient2_color_np_stop_insert(void *data __UNUSED__, void *gradient __UNUSED__, int r __UNUSED__, int g __UNUSED__, int b __UNUSED__, int a __UNUSED__, float pos __UNUSED__)
{
   evas_common_gradient2_color_np_stop_insert(gradient, r, g, b, a, pos);
}

static void
eng_gradient2_clear(void *data __UNUSED__, void *gradient __UNUSED__)
{
   evas_common_gradient2_clear(gradient);
}

static void
eng_gradient2_fill_transform_set(void *data __UNUSED__, void *gradient __UNUSED__, void *transform __UNUSED__)
{
   evas_common_gradient2_fill_transform_set(gradient, transform);
}

static void
eng_gradient2_fill_spread_set(void *data __UNUSED__, void *gradient __UNUSED__, int spread __UNUSED__)
{
   evas_common_gradient2_fill_spread_set(gradient, spread);
}

static void *
eng_gradient2_linear_new(void *data __UNUSED__)
{
   return evas_common_gradient2_linear_new();
}

static void
eng_gradient2_linear_free(void *data __UNUSED__, void *linear_gradient __UNUSED__)
{
   evas_common_gradient2_free(linear_gradient);
}

static void
eng_gradient2_linear_fill_set(void *data __UNUSED__, void *linear_gradient __UNUSED__, float x0 __UNUSED__, float y0 __UNUSED__, float x1 __UNUSED__, float y1 __UNUSED__)
{
   evas_common_gradient2_linear_fill_set(linear_gradient, x0, y0, x1, y1);
}

static int
eng_gradient2_linear_is_opaque(void *data __UNUSED__, void *context __UNUSED__, void *linear_gradient __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   RGBA_Gradient2 *gr = (RGBA_Gradient2 *)linear_gradient;

   if (!dc || !gr || !gr->type.geometer)  return 0;
   return !(gr->type.geometer->has_alpha(gr, dc->render_op) |
            gr->type.geometer->has_mask(gr, dc->render_op));
}

static int
eng_gradient2_linear_is_visible(void *data __UNUSED__, void *context __UNUSED__, void *linear_gradient __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;

   if (!dc || !linear_gradient)  return 0;
   return 1;
}

static void
eng_gradient2_linear_render_pre(void *data __UNUSED__, void *context __UNUSED__, void *linear_gradient __UNUSED__)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   RGBA_Gradient2 *gr = (RGBA_Gradient2 *)linear_gradient;
   int  len;
   
   if (!dc || !gr || !gr->type.geometer)  return;
   gr->type.geometer->geom_update(gr);
   len = gr->type.geometer->get_map_len(gr);
   evas_common_gradient2_map(dc, gr, len);
}

static void
eng_gradient2_linear_render_post(void *data __UNUSED__, void *linear_gradient __UNUSED__)
{
}

static void
eng_gradient2_linear_draw(void *data __UNUSED__, void *context __UNUSED__, void *surface __UNUSED__, void *linear_gradient __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   re->win->gl_context->dc = context;
     {
        Evas_GL_Image *gim;
        RGBA_Image *im;
        RGBA_Draw_Context *dc = context;
        int op = dc->render_op, cuse = dc->clip.use;
        
        im = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
        im = (RGBA_Image *)evas_cache_image_size_set(&im->cache_entry, w, h);
        
        dc->render_op = _EVAS_RENDER_FILL;
        dc->clip.use = 0;
        
        // draw to buf, copy to tex, draw tex
        evas_common_gradient2_draw(im, dc, 0, 0, w, h, linear_gradient);

        gim = evas_gl_common_image_new_from_data(re->win->gl_context, w, h,
                                                 im->image.data, 1,
                                                 EVAS_COLORSPACE_ARGB8888);
        dc->render_op = op;
        dc->clip.use = cuse;
        evas_gl_common_image_draw(re->win->gl_context, gim, 0, 0, w, h, x, y, w, h, 0);
        evas_cache_image_drop(&im->cache_entry);
        evas_gl_common_image_free(gim);
     }
}

static void *
eng_gradient2_radial_new(void *data __UNUSED__)
{
   return evas_common_gradient2_radial_new();
}

static void
eng_gradient2_radial_free(void *data __UNUSED__, void *radial_gradient __UNUSED__)
{
   evas_common_gradient2_free(radial_gradient);
}

static void
eng_gradient2_radial_fill_set(void *data __UNUSED__, void *radial_gradient __UNUSED__, float cx __UNUSED__, float cy __UNUSED__, float rx __UNUSED__, float ry __UNUSED__)
{
   evas_common_gradient2_radial_fill_set(radial_gradient, cx, cy, rx, ry);
}

static int
eng_gradient2_radial_is_opaque(void *data __UNUSED__, void *context __UNUSED__, void *radial_gradient __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   RGBA_Gradient2 *gr = (RGBA_Gradient2 *)radial_gradient;
   
   if (!dc || !gr || !gr->type.geometer)  return 0;
   return !(gr->type.geometer->has_alpha(gr, dc->render_op) |
            gr->type.geometer->has_mask(gr, dc->render_op));
}

static int
eng_gradient2_radial_is_visible(void *data __UNUSED__, void *context __UNUSED__, void *radial_gradient __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   
   if (!dc || !radial_gradient)  return 0;
   return 1;
}

static void
eng_gradient2_radial_render_pre(void *data __UNUSED__, void *context __UNUSED__, void *radial_gradient __UNUSED__)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   RGBA_Gradient2 *gr = (RGBA_Gradient2 *)radial_gradient;
   int  len;
   
   if (!dc || !gr || !gr->type.geometer)  return;
   gr->type.geometer->geom_update(gr);
   len = gr->type.geometer->get_map_len(gr);
   evas_common_gradient2_map(dc, gr, len);
}

static void
eng_gradient2_radial_render_post(void *data __UNUSED__, void *radial_gradient __UNUSED__)
{
}

static void
eng_gradient2_radial_draw(void *data __UNUSED__, void *context __UNUSED__, void *surface __UNUSED__, void *radial_gradient __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   re->win->gl_context->dc = context;
     {
        Evas_GL_Image *gim;
        RGBA_Image *im;
        RGBA_Draw_Context *dc = context;
        int op = dc->render_op, cuse = dc->clip.use;
        
        im = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
        im = (RGBA_Image *)evas_cache_image_size_set(&im->cache_entry, w, h);
        
        dc->render_op = _EVAS_RENDER_FILL;
        dc->clip.use = 0;
        
        // draw to buf, copy to tex, draw tex
        evas_common_gradient2_draw(im, dc, 0, 0, w, h, radial_gradient);

        gim = evas_gl_common_image_new_from_data(re->win->gl_context, w, h,
                                                 im->image.data, 1,
                                                 EVAS_COLORSPACE_ARGB8888);
        dc->render_op = op;
        dc->clip.use = cuse;
        evas_gl_common_image_draw(re->win->gl_context, gim, 0, 0, w, h, x, y, w, h, 0);
        evas_cache_image_drop(&im->cache_entry);
        evas_gl_common_image_free(gim);
     }
}

static void *
eng_gradient_new(void *data __UNUSED__)
{
   return evas_common_gradient_new();
}

static void
eng_gradient_free(void *data __UNUSED__, void *gradient)
{
   evas_common_gradient_free(gradient);
}

static void
eng_gradient_color_stop_add(void *data __UNUSED__, void *gradient, int r, int g, int b, int a, int delta)
{
   evas_common_gradient_color_stop_add(gradient, r, g, b, a, delta);
}

static void
eng_gradient_alpha_stop_add(void *data __UNUSED__, void *gradient, int a, int delta)
{
   evas_common_gradient_alpha_stop_add(gradient, a, delta);
}

static void
eng_gradient_color_data_set(void *data __UNUSED__, void *gradient, void *map, int len, int has_alpha)
{
   evas_common_gradient_color_data_set(gradient, map, len, has_alpha);
}

static void
eng_gradient_alpha_data_set(void *data __UNUSED__, void *gradient, void *alpha_map, int len)
{
   evas_common_gradient_alpha_data_set(gradient, alpha_map, len);
}

static void
eng_gradient_clear(void *data __UNUSED__, void *gradient)
{
   evas_common_gradient_clear(gradient);
}

static void
eng_gradient_fill_set(void *data __UNUSED__, void *gradient, int x, int y, int w, int h)
{
   evas_common_gradient_fill_set(gradient, x, y, w, h);
}

static void
eng_gradient_fill_angle_set(void *data __UNUSED__, void *gradient, double angle)
{
   evas_common_gradient_fill_angle_set(gradient, angle);
}

static void
eng_gradient_fill_spread_set(void *data __UNUSED__, void *gradient, int spread)
{
   evas_common_gradient_fill_spread_set(gradient, spread);
}

static void
eng_gradient_angle_set(void *data __UNUSED__, void *gradient, double angle)
{
   evas_common_gradient_map_angle_set(gradient, angle);
}

static void
eng_gradient_offset_set(void *data __UNUSED__, void *gradient, float offset)
{
   evas_common_gradient_map_offset_set(gradient, offset);
}

static void
eng_gradient_direction_set(void *data __UNUSED__, void *gradient, int direction)
{
   evas_common_gradient_map_direction_set(gradient, direction);
}

static void
eng_gradient_type_set(void *data __UNUSED__, void *gradient, char *name, char *params)
{
   evas_common_gradient_type_set(gradient, name, params);
}

static int
eng_gradient_is_opaque(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   RGBA_Gradient *gr = (RGBA_Gradient *)gradient;
   
   if (!dc || !gr || !gr->type.geometer)  return 0;
   return !(gr->type.geometer->has_alpha(gr, dc->render_op) |
            gr->type.geometer->has_mask(gr, dc->render_op));
}

static int
eng_gradient_is_visible(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   
   if (!dc || !gradient)  return 0;
   return 1;
}

static void
eng_gradient_render_pre(void *data, void *context, void *gradient)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   RGBA_Gradient *gr = (RGBA_Gradient *)gradient;
   int  len;
   
   if (!dc || !gr || !gr->type.geometer)  return;
   gr->type.geometer->geom_set(gr);
   len = gr->type.geometer->get_map_len(gr);
   evas_common_gradient_map(dc, gr, len);
}

static void
eng_gradient_render_post(void *data __UNUSED__, void *gradient)
{
}

static void
eng_gradient_draw(void *data, void *context, void *surface __UNUSED__, void *gradient, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   re->win->gl_context->dc = context;
     {
        Evas_GL_Image *gim;
        RGBA_Image *im;
        RGBA_Draw_Context *dc = context;
        int op = dc->render_op, cuse = dc->clip.use;
        
        im = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
        im = (RGBA_Image *)evas_cache_image_size_set(&im->cache_entry, w, h);
        
        dc->render_op = _EVAS_RENDER_FILL;
        dc->clip.use = 0;
        
        // draw to buf, copy to tex, draw tex
        evas_common_gradient_draw(im, dc, 0, 0, w, h, gradient);

        gim = evas_gl_common_image_new_from_data(re->win->gl_context, w, h,
                                                 im->image.data, 1,
                                                 EVAS_COLORSPACE_ARGB8888);
        dc->render_op = op;
        dc->clip.use = cuse;
        evas_gl_common_image_draw(re->win->gl_context, gim, 0, 0, w, h, x, y, w, h, 0);
        evas_cache_image_drop(&im->cache_entry);
        evas_gl_common_image_free(gim);
     }
}

static int
eng_image_alpha_get(void *data, void *image)
{
//   Render_Engine *re;
   Evas_GL_Image *im;

//   re = (Render_Engine *)data;
   if (!image) return 1;
   im = image;
   return im->alpha;
}

static int
eng_image_colorspace_get(void *data, void *image)
{
//   Render_Engine *re;
   Evas_GL_Image *im;

//   re = (Render_Engine *)data;
   if (!image) return EVAS_COLORSPACE_ARGB8888;
   im = image;
   return im->cs.space;
}

static void *
eng_image_alpha_set(void *data, void *image, int has_alpha)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   if (!image) return NULL;
   im = image;
   if (im->native.data)
     {
        im->alpha = has_alpha;
        return image;
     }
   eng_window_use(re->win);
   /* FIXME: can move to gl_common */
   if (im->cs.space != EVAS_COLORSPACE_ARGB8888) return im;
   if ((has_alpha) && (im->im->cache_entry.flags.alpha)) return image;
   else if ((!has_alpha) && (!im->im->cache_entry.flags.alpha)) return image;
   if (im->references > 1)
     {
        Evas_GL_Image *im_new;
        
        im_new = evas_gl_common_image_new_from_copied_data(im->gc, im->im->cache_entry.w, im->im->cache_entry.h, im->im->image.data,
                                                           eng_image_alpha_get(data, image),
                                                           eng_image_colorspace_get(data, image));
        if (!im_new) return im;
        evas_gl_common_image_free(im);
        im = im_new;
     }
   else
     evas_gl_common_image_dirty(im, 0, 0, 0, 0);
   im->im->cache_entry.flags.alpha = has_alpha ? 1 : 0;
   return image;
}

static void *
eng_image_border_set(void *data, void *image, int l __UNUSED__, int r __UNUSED__, int t __UNUSED__, int b __UNUSED__)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
   return image;
}

static void
eng_image_border_get(void *data, void *image __UNUSED__, int *l __UNUSED__, int *r __UNUSED__, int *t __UNUSED__, int *b __UNUSED__)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
}

static char *
eng_image_comment_get(void *data, void *image, char *key __UNUSED__)
{
//   Render_Engine *re;
   Evas_GL_Image *im;

//   re = (Render_Engine *)data;
   if (!image) return NULL;
   im = image;
   if (!im->im) return NULL;
   return im->im->info.comment;
}

static char *
eng_image_format_get(void *data, void *image)
{
//   Render_Engine *re;
   Evas_GL_Image *im;

//   re = (Render_Engine *)data;
   im = image;
   return NULL;
}

static void
eng_image_colorspace_set(void *data, void *image, int cspace)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   if (!image) return;
   im = image;
   if (im->native.data) return;
   /* FIXME: can move to gl_common */
   if (im->cs.space == cspace) return;
   eng_window_use(re->win);
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
        if (im->tex) evas_gl_common_texture_free(im->tex);
        im->tex = NULL;
	if (im->cs.data)
	  {
	     if (!im->cs.no_free) free(im->cs.data);
	  }
	im->cs.data = calloc(1, im->im->cache_entry.h * sizeof(unsigned char *) * 2);
	im->cs.no_free = 0;
	break;
      default:
	abort();
	break;
     }
   im->cs.space = cspace;
}

/////////////////////////////////////////////////////////////////////////
//
//
typedef struct _Native Native;

struct _Native
{
   Evas_Native_Surface ns;
   Pixmap     pixmap;
   Visual    *visual;
   
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   void      *egl_surface;
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT
   GLXFBConfig fbc;
   GLXPixmap   glx_pixmap;
# endif
#endif
};

// FIXME: this is enabled so updates happen - but its SLOOOOOOOOOOOOOOOW
// (i am sure this is the reason)  not to mention seemingly superfluous. but
// i need to enable it for it to work on fglrx at least. havent tried nvidia.
// 
// why is this the case? does anyone know? has anyone tried it on other gfx
// drivers?
// 
//#define GLX_TEX_PIXMAP_RECREATE 1

static void
_native_bind_cb(void *data, void *image)
{
   Render_Engine *re = data;
   Evas_GL_Image *im = image;
   Native *n = im->native.data;
   
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   if (n->egl_surface)
     {
        if (glsym_glEGLImageTargetTexture2DOES)
          glsym_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, n->egl_surface);
     }
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT
   if (glsym_glXBindTexImage)
     glsym_glXBindTexImage(re->win->disp, n->glx_pixmap, 
                           GLX_FRONT_LEFT_EXT, NULL);
# endif
#endif
}

static void
_native_unbind_cb(void *data, void *image)
{
   Render_Engine *re = data;
   Evas_GL_Image *im = image;
   Native *n = im->native.data;

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   // nothing
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT
   if (glsym_glXReleaseTexImage)
     glsym_glXReleaseTexImage(re->win->disp, n->glx_pixmap, 
                              GLX_FRONT_LEFT_EXT);
# endif
#endif
}

static void
_native_free_cb(void *data, void *image)
{
   Render_Engine *re = data;
   Evas_GL_Image *im = image;
   Native *n = im->native.data;

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   if (n->egl_surface)
     {
        if (glsym_eglDestroyImage)
          glsym_eglDestroyImage(re->win->egl_disp,
                                n->egl_surface);
     }
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT
   if (n->glx_pixmap)
     {
        if (im->native.loose)
          {
             if (glsym_glXReleaseTexImage)
               glsym_glXReleaseTexImage(re->win->disp, n->glx_pixmap,
                                        GLX_FRONT_LEFT_EXT);
          }
        glXDestroyPixmap(re->win->disp, n->glx_pixmap);
        n->glx_pixmap = 0;
     }
# endif
#endif
   im->native.data        = NULL;
   im->native.func.data   = NULL;
   im->native.func.bind   = NULL;
   im->native.func.unbind = NULL;
   im->native.func.free   = NULL;
   free(n);
}

static void
eng_image_native_set(void *data, void *image, void *native)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Native_Surface *ns = native;
   Evas_GL_Image *im = image;
   Visual *vis = NULL;
   Pixmap pm = 0;

   if (ns)
     {
        vis = ns->data.x11.visual;
        pm = ns->data.x11.pixmap;
        if (im->native.data)
          {
             Evas_Native_Surface *n = im->native.data;
             if ((n->data.x11.visual == vis) && (n->data.x11.pixmap == pm))
               {
                  return;
               }
          }
     }
   if ((!ns) && (!im->native.data)) return;
   if (!im) return;
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   if (im->native.data)
     {
        if (im->native.func.free)
          im->native.func.free(im->native.func.data, im);
        evas_gl_common_image_native_disable(im);
        im->native.data = NULL;
     }
   if (native)
     {
        Native *n;
        
        n = calloc(1, sizeof(Native));
        if (n)
          {
             EGLConfig egl_config;
             int config_attrs[20];
             int num_config, i;
             
             i = 0;
             config_attrs[i++] = EGL_RED_SIZE;
             config_attrs[i++] = 8;
             config_attrs[i++] = EGL_GREEN_SIZE;
             config_attrs[i++] = 8;
             config_attrs[i++] = EGL_BLUE_SIZE;
             config_attrs[i++] = 8;
             config_attrs[i++] = EGL_ALPHA_SIZE;
             config_attrs[i++] = 8;
             config_attrs[i++] = EGL_DEPTH_SIZE;
             config_attrs[i++] = 0;
             config_attrs[i++] = EGL_STENCIL_SIZE;
             config_attrs[i++] = 0;
             config_attrs[i++] = EGL_RENDERABLE_TYPE;
             config_attrs[i++] = EGL_OPENGL_ES2_BIT;
             config_attrs[i++] = EGL_SURFACE_TYPE;
             config_attrs[i++] = EGL_PIXMAP_BIT;
             config_attrs[i++] = EGL_NONE;
             
             if (!eglChooseConfig(re->win->egl_disp, config_attrs, 
                                  &egl_config, 1, &num_config))
               {
                  printf("ERROR: eglChooseConfig() failed for pixmap 0x%x, num_config = %i\n", (unsigned int)pm, num_config);
               }
             n->pixmap = pm;
             n->visual = vis;
             im->native.yinvert     = 1;
             im->native.loose       = 0;
             im->native.data        = n;
             im->native.func.data   = re;
             im->native.func.bind   = _native_bind_cb;
             im->native.func.unbind = _native_unbind_cb;
             im->native.func.free   = _native_free_cb;
             im->native.target      = GL_TEXTURE_2D;
             im->native.mipmap      = 0;
             if (glsym_eglCreateImage)
               n->egl_surface = glsym_eglCreateImage(re->win->egl_disp,
                                                     EGL_NO_CONTEXT,
                                                     EGL_NATIVE_PIXMAP_KHR,
                                                     (void *)pm,
                                                     NULL);
             if (!n->egl_surface)
               {
                  printf("ERROR: eglCreatePixmapSurface() for 0x%x failed\n", (unsigned int)pm);
               }
             evas_gl_common_image_native_enable(im);
          }
     }
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT
   if (im->native.data)
     {
        if (im->native.func.free)
          im->native.func.free(im->native.func.data, im);
        evas_gl_common_image_native_disable(im);
     }
   if (native)
     {
        int dummy;
        unsigned int w, h, depth = 32, border;
        Window wdummy;
        Native *n;
        
        XGetGeometry(re->win->disp, pm, &wdummy, &dummy, &dummy, 
                     &w, &h, &border, &depth);
        n = calloc(1, sizeof(Native));
        if (n)
          {
             int pixmap_att[20];
             int target = 0;
             int i = 0;
             
             if ((re->win->depth_cfg[depth].tex_target &
                  GLX_TEXTURE_2D_BIT_EXT) &&
                 (1) // we assume npo2 for now
                 // size is pow2 || mnpo2 supported
                 )
               {
                  printf("2d\n");
                  target = GLX_TEXTURE_2D_EXT;
               }
             else if ((re->win->depth_cfg[depth].tex_target &
                      GLX_TEXTURE_RECTANGLE_BIT_EXT))
               {
                  printf("rect\n");
                  target = GLX_TEXTURE_RECTANGLE_EXT;
               }
             if (!target)
               {
                  printf("broken text-from-pixmap\n");
                  if (!(re->win->depth_cfg[depth].tex_target &
                        GLX_TEXTURE_2D_BIT_EXT))
                    {
                       target = GLX_TEXTURE_RECTANGLE_EXT;
                    }
                  else if (!(re->win->depth_cfg[depth].tex_target &
                             GLX_TEXTURE_RECTANGLE_BIT_EXT))
                    {
                       target = GLX_TEXTURE_2D_EXT;
                    }
               }
             
             
             pixmap_att[i++] = GLX_TEXTURE_FORMAT_EXT;
             pixmap_att[i++] = re->win->depth_cfg[depth].tex_format;
             pixmap_att[i++] = GLX_MIPMAP_TEXTURE_EXT;
             pixmap_att[i++] = re->win->depth_cfg[depth].mipmap;
             
             if (target)
               {
                  pixmap_att[i++] = GLX_TEXTURE_TARGET_EXT;
                  pixmap_att[i++] = target;
               }
             
             pixmap_att[i++] = 0;

             memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
             n->pixmap = pm;
             n->visual = vis;
             n->fbc = re->win->depth_cfg[depth].fbc;
             im->native.yinvert     = re->win->depth_cfg[depth].yinvert;
             im->native.loose       = 0;
             im->native.data        = n;
             im->native.func.data   = re;
             im->native.func.bind   = _native_bind_cb;
             im->native.func.unbind = _native_unbind_cb;
             im->native.func.free   = _native_free_cb;

             n->glx_pixmap = glXCreatePixmap(re->win->disp, n->fbc, 
                                             n->pixmap, pixmap_att);
             if (!target)
               {
                  printf("notgt\n");
                  glXQueryDrawable(re->win->disp, n->pixmap, GLX_TEXTURE_TARGET_EXT, &target);
               }
             if (target == GLX_TEXTURE_2D_EXT)
               {
                  im->native.target = GL_TEXTURE_2D;
                  im->native.mipmap = re->win->depth_cfg[depth].mipmap;
               }
             else if (target == GLX_TEXTURE_RECTANGLE_EXT)
               {
                  im->native.target = GL_TEXTURE_RECTANGLE_ARB;
                  im->native.mipmap = 0;
               }
             else
               {
                  im->native.target = GL_TEXTURE_2D;
                  im->native.mipmap = 0;
                  printf("still unknown target\n");
               }

             evas_gl_common_image_native_enable(im);
          }
     }
# endif   
#endif
}

static void *
eng_image_native_get(void *data, void *image)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_GL_Image *im = image;
   Native *n;
   if (!im) return NULL;
   n = im->native.data;
   if (!n) return NULL;
   return &(n->ns);
}

//
//
/////////////////////////////////////////////////////////////////////////

static void *
eng_image_load(void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   *error = EVAS_LOAD_ERROR_NONE;
   eng_window_use(re->win);
   return evas_gl_common_image_load(re->win->gl_context, file, key, lo, error);
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   return evas_gl_common_image_new_from_data(re->win->gl_context, w, h, image_data, alpha, cspace);
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   return evas_gl_common_image_new_from_copied_data(re->win->gl_context, w, h, image_data, alpha, cspace);
}

static void
eng_image_free(void *data, void *image)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!image) return;
   eng_window_use(re->win);
   evas_gl_common_image_free(image);
}

static void
eng_image_size_get(void *data, void *image, int *w, int *h)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
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
   Evas_GL_Image *im = image;
   Evas_GL_Image *im_old;
   
   re = (Render_Engine *)data;
   if (!im) return NULL;
   if (im->native.data)
     {
        im->w = w;
        im->h = h;
        return image;
     }
   eng_window_use(re->win);
   im_old = image;
   if ((eng_image_colorspace_get(data, image) == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (eng_image_colorspace_get(data, image) == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;
   if ((im_old) && (im_old->im->cache_entry.w == w) && (im_old->im->cache_entry.h == h))
     return image;
   if (im_old)
     {
   	im = evas_gl_common_image_new(re->win->gl_context, w, h,
   				      eng_image_alpha_get(data, image),
   				      eng_image_colorspace_get(data, image));
/*
	evas_common_load_image_data_from_file(im_old->im);
	if (im_old->im->image->data)
	  {
	     evas_common_blit_rectangle(im_old->im, im->im, 0, 0, w, h, 0, 0);
	     evas_common_cpu_end_opt();
	  }
 */
        evas_gl_common_image_free(im_old);
     }
   else
     im = evas_gl_common_image_new(re->win->gl_context, w, h, 1, EVAS_COLORSPACE_ARGB8888);
   return im;
}

static void *
eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   Render_Engine *re;
   Evas_GL_Image *im = image;

   re = (Render_Engine *)data;
   if (!image) return NULL;
   if (im->native.data) return image;
   eng_window_use(re->win);
   evas_gl_common_image_dirty(image, x, y, w, h);
   return image;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   if (!image)
     {
	*image_data = NULL;
	return NULL;
     }
   im = image;
   if (im->native.data)
     {
        *image_data = NULL;
        return im;
     }
   eng_window_use(re->win);
   evas_cache_image_load_data(&im->im->cache_entry);
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (to_write)
	  {
	     if (im->references > 1)
	       {
		  Evas_GL_Image *im_new;

   		  im_new = evas_gl_common_image_new_from_copied_data(im->gc, im->im->cache_entry.w, im->im->cache_entry.h, im->im->image.data,
   								     eng_image_alpha_get(data, image),
   								     eng_image_colorspace_get(data, image));
   		  if (!im_new)
   		    {
   		       *image_data = NULL;
   		       return im;
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
	*image_data = im->cs.data;
	break;
      default:
	abort();
	break;
     }
   return im;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   Render_Engine *re;
   Evas_GL_Image *im, *im2;

   re = (Render_Engine *)data;
   if (!image) return NULL;
   im = image;
   if (im->native.data) return image;
   eng_window_use(re->win);
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (image_data != im->im->image.data)
	  {
	     int w, h;

	     w = im->im->cache_entry.w;
	     h = im->im->cache_entry.h;
	     im2 = eng_image_new_from_data(data, w, h, image_data,
					   eng_image_alpha_get(data, image),
					   eng_image_colorspace_get(data, image));
   	     if (!im2) return im;
   	     evas_gl_common_image_free(im);
   	     im = im2;
	  }
        break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
        if (image_data != im->cs.data)
	  {
	     if (im->cs.data)
	       {
		  if (!im->cs.no_free) free(im->cs.data);
	       }
	     im->cs.data = image_data;
	  }
	break;
      default:
	abort();
	break;
     }
   /* hmmm - but if we wrote... why bother? */
   evas_gl_common_image_dirty(im, 0, 0, 0, 0);
   return im;
}

static void
eng_image_data_preload_request(void *data __UNUSED__, void *image, const void *target)
{
   Evas_GL_Image *gim = image;
   RGBA_Image *im;

   if (!gim) return;
   if (gim->native.data) return;
   im = (RGBA_Image *)gim->im;
   if (!im) return;
   evas_cache_image_preload_data(&im->cache_entry, target);
}

static void
eng_image_data_preload_cancel(void *data __UNUSED__, void *image, const void *target)
{
   Evas_GL_Image *gim = image;
   RGBA_Image *im;

   if (!gim) return;
   if (gim->native.data) return;
   im = (RGBA_Image *)gim->im;
   if (!im) return;
   evas_cache_image_preload_cancel(&im->cache_entry, target);
}

static void
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!image) return;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_image_draw(re->win->gl_context, image,
                             src_x, src_y, src_w, src_h,
                             dst_x, dst_y, dst_w, dst_h,
                             smooth);
}

static void
eng_image_scale_hint_set(void *data __UNUSED__, void *image, int hint)
{
}

static void
eng_image_map4_draw(void *data __UNUSED__, void *context, void *surface, void *image, RGBA_Map_Point *p, int smooth, int level)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_image_map4_draw(re->win->gl_context, image, p, smooth, level);
}

static void *
eng_image_map_surface_new(void *data __UNUSED__, int w, int h, int alpha)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   return evas_gl_common_image_surface_new(re->win->gl_context, w, h, alpha);
}

static void
eng_image_map_surface_free(void *data __UNUSED__, void *surface)
{
   evas_gl_common_image_free(surface);
}

static int
eng_image_scale_hint_get(void *data __UNUSED__, void *image)
{
   return EVAS_IMAGE_SCALE_HINT_NONE;
}

static void
eng_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w __UNUSED__, int h __UNUSED__, int ow __UNUSED__, int oh __UNUSED__, const char *text)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
     {
        // FIXME: put im into context so we can free it
	static RGBA_Image *im = NULL;
        
        if (!im)
          im = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
        im->cache_entry.w = re->win->w;
        im->cache_entry.h = re->win->h;
        evas_common_draw_context_font_ext_set(context,
   					      re->win->gl_context,
   					      evas_gl_font_texture_new,
   					      evas_gl_font_texture_free,
   					      evas_gl_font_texture_draw);
	evas_common_font_draw(im, context, font, x, y, text);
	evas_common_draw_context_font_ext_set(context,
					      NULL,
					      NULL,
					      NULL,
					      NULL);
     }
}

static Eina_Bool
eng_canvas_alpha_get(void *data __UNUSED__, void *info __UNUSED__)
{
   // FIXME: support ARGB gl targets!!!
   return EINA_FALSE;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;
   if (_evas_engine_GL_X11_log_dom < 0)
     _evas_engine_GL_X11_log_dom = eina_log_domain_register("EvasEngineGLX11", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_GL_X11_log_dom < 0)
     {
        EINA_LOG_ERR("Impossible to create a log domain for GL X11 engine.\n");
        return 0;
     }
   /* store it for later use */
   func = pfunc;
   /* now to override methods */
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
   ORD(context_cutout_add);
   ORD(context_cutout_clear);
   ORD(output_flush);
   ORD(output_idle_flush);
   ORD(rectangle_draw);
   ORD(line_draw);
   ORD(polygon_point_add);
   ORD(polygon_points_clear);
   ORD(polygon_draw);

   ORD(gradient2_color_np_stop_insert);
   ORD(gradient2_clear);
   ORD(gradient2_fill_transform_set);
   ORD(gradient2_fill_spread_set);
   ORD(gradient2_linear_new);
   ORD(gradient2_linear_free);
   ORD(gradient2_linear_fill_set);
   ORD(gradient2_linear_is_opaque);
   ORD(gradient2_linear_is_visible);
   ORD(gradient2_linear_render_pre);
   ORD(gradient2_linear_render_post);
   ORD(gradient2_linear_draw);
   ORD(gradient2_radial_new);
   ORD(gradient2_radial_free);
   ORD(gradient2_radial_fill_set);
   ORD(gradient2_radial_is_opaque);
   ORD(gradient2_radial_is_visible);
   ORD(gradient2_radial_render_pre);
   ORD(gradient2_radial_render_post);
   ORD(gradient2_radial_draw);

   ORD(gradient_new);
   ORD(gradient_free);
   ORD(gradient_color_stop_add);
   ORD(gradient_alpha_stop_add);
   ORD(gradient_color_data_set);
   ORD(gradient_alpha_data_set);
   ORD(gradient_clear);
   ORD(gradient_fill_set);
   ORD(gradient_fill_angle_set);
   ORD(gradient_fill_spread_set);
   ORD(gradient_angle_set);
   ORD(gradient_offset_set);
   ORD(gradient_direction_set);
   ORD(gradient_type_set);
   ORD(gradient_is_opaque);
   ORD(gradient_is_visible);
   ORD(gradient_render_pre);
   ORD(gradient_render_post);
   ORD(gradient_draw);
   ORD(image_load);
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
   ORD(image_native_set);
   ORD(image_native_get);
   ORD(font_draw);
   
   ORD(image_scale_hint_set);
   ORD(image_scale_hint_get);
   
   ORD(image_map4_draw);
   ORD(image_map_surface_new);
   ORD(image_map_surface_free);
   
   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

static void
module_close(Evas_Module *em)
{
    eina_log_domain_unregister(_evas_engine_GL_X11_log_dom);
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "gl_x11",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, gl_x11);

#ifndef EVAS_STATIC_BUILD_GL_X11
EVAS_EINA_MODULE_DEFINE(engine, gl_x11);
#endif
