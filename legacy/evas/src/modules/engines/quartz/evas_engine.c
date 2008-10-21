/*
 * vim:ts=3:sw=3:sts=3:expandtab
 */
#include <ApplicationServices/ApplicationServices.h>

#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_Quartz.h"
#include "evas_quartz_private.h"

static Evas_Func func;

typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   CGContextRef ctx;
   int w, h;
   
   struct
   {
      int redraw : 1;
      int x1, y1, x2, y2;
   } draw;
};

static inline void
flip_pixels(int *y, int *h, void *re)
{
   // We need to flip the Y axis, because Quartz uses a coordinate system
   // with the origin in the bottom left, while Evas uses a top left origin.
   
   (*y) = ((Evas_Quartz_Context *)re)->h - (*y);
   
   if (h && y) (*y) -= *h;
}

static void *
eng_info(Evas *e)
{
   Evas_Engine_Info_Quartz *info;

   info = calloc(1, sizeof(Evas_Engine_Info_Quartz));
   if (!info) return NULL;
      
   info->magic.magic = rand();

   return info;
}

static void
eng_info_free(Evas *e, void *info)
{
   free((Evas_Engine_Info_Quartz *)info);
}

static void
eng_setup(Evas *e, void *in)
{
   Render_Engine *re;
   Evas_Engine_Info_Quartz *info = (Evas_Engine_Info_Quartz *)in;

   if (!e->engine.data.output)
     e->engine.data.output = eng_output_setup(info->info.context, e->output.w, e->output.h);
   if (!e->engine.data.output) return;

   if (!e->engine.data.context)
      e->engine.data.context = e->engine.func->context_new(e->engine.data.output);
   
   ((Evas_Quartz_Context *)e->engine.data.context)->w = e->output.w;
   ((Evas_Quartz_Context *)e->engine.data.context)->h = e->output.h;
}

#pragma mark Output Setup

static void *
eng_output_setup(CGContextRef context, int w, int h)
{
   Render_Engine *re = calloc(1, sizeof(Render_Engine));
   if (!re) return NULL;
   
   re->ctx = context;
   re->w = w;
   re->h = h;
   
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

   return re;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   free(re);

   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re = (Render_Engine *)data;

   re->w = w;
   re->h = h;
}

static void
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re = (Render_Engine *)data;
   
   if (!re->draw.redraw)
   {
      re->draw.x1 = x;
      re->draw.y1 = y;
      re->draw.x2 = x + w;
      re->draw.y2 = y + h;
   }
   else
   {
      if (x < re->draw.x1) re->draw.x1 = x;
      if (y < re->draw.y1) re->draw.y1 = y;
      if ((x + w - 1) > re->draw.x2) re->draw.x2 = x + w - 1;
      if ((y + h - 1) > re->draw.y2) re->draw.y2 = y + h - 1;
   }
   
   re->draw.redraw = 1;
}

static void
eng_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   // FIXME: Implement this?
}

static void
eng_output_redraws_clear(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   re->draw.redraw = 0;
}

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re = (Render_Engine *)data;
   
   if (!re->draw.redraw) return NULL;

   if (x) *x = re->draw.x1;
   if (y) *y = re->draw.y1;
   if (w) *w = re->draw.x2 - re->draw.x1 + 1;
   if (h) *h = re->draw.y2 - re->draw.y1 + 1;
   if (cx) *cx = re->draw.x1;
   if (cy) *cy = re->draw.y1;
   if (cw) *cw = re->draw.x2 - re->draw.x1 + 1;
   if (ch) *ch = re->draw.y2 - re->draw.y1 + 1;
   
   return re;
}

static void
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Quartz_Context *ctx = (Evas_Quartz_Context *)(re->ctx);
   re->draw.redraw = 0;
   
   flip_pixels(&y, &h, ctx);
   
   CGContextClearRect(re->ctx, CGRectMake(x, y, w, h));
}

static void
eng_output_flush(void *data)
{
   // By default, Apple coalesces calls to CGContextFlush, but this actually
   // blocks if called more than 60 times per second, which is a waste of time.
   // 
   // http://developer.apple.com/technotes/tn2005/tn2133.html
   
   CGContextFlush(((Render_Engine *)data)->ctx);
}

#pragma mark Context Manipulation

static void *
eng_context_new(void *data)
{
   Evas_Quartz_Context *ctxt = calloc(1, sizeof(Evas_Quartz_Context));
   if (!ctxt) return NULL;

   return ctxt;
}

static void
eng_context_free(void *data, void *context)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;
   
   if (re->ctx) CGContextRelease(re->ctx);
   free(ctxt);
}

static void
eng_context_clip_set(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;
   
   flip_pixels(&y, &h, ctxt);
   
   CGContextResetClip(re->ctx);
   CGContextClipToRect(re->ctx, CGRectMake(0, 0, re->w, re->h)); // don't draw over the title bar
   CGContextClipToRect(re->ctx, CGRectMake(x, y, w, h));
   
   ctxt->clipped = 1;
}

static void
eng_context_clip_unset(void *data, void *context)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;
   
   CGContextResetClip(re->ctx);
   
   ctxt->clipped = 0;
}

static int
eng_context_clip_get(void *data, void *context, int *x, int *y, int *w, int *h)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;
   
   CGRect clip = CGContextGetClipBoundingBox(re->ctx);
   if (x) *x = clip.origin.x;
   if (y) *y = clip.origin.y;
   if (w) *w = clip.size.width;
   if (h) *h = clip.size.height;
   
   return ctxt->clipped;
}

static void
eng_context_color_set(void *data, void *context, int r, int g, int b, int a)
{
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;
   
   ctxt->col.r = (double)r / 255.0;
   ctxt->col.g = (double)g / 255.0;
   ctxt->col.b = (double)b / 255.0;
   ctxt->col.a = (double)a / 255.0;
}

static int
eng_context_color_get(void *data, void *context, int *r, int *g, int *b, int *a)
{
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;

   if (r) *r = ctxt->col.r * 255;
   if (g) *g = ctxt->col.g * 255;
   if (b) *b = ctxt->col.b * 255;
   if (a) *a = ctxt->col.a * 255;
   return 1;
}

static void
eng_context_multiplier_set(void *data, void *context, int r, int g, int b, int a)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;

   ctxt->mul.r = (double)r / 255.0;
   ctxt->mul.g = (double)g / 255.0;
   ctxt->mul.b = (double)b / 255.0;
   ctxt->mul.a = (double)a / 255.0;
   ctxt->mul.set = 1;
   
   CGContextSetAlpha(re->ctx, ctxt->mul.a);
}

static void
eng_context_multiplier_unset(void *data, void *context)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;
   
   ctxt->mul.set = 0;
   CGContextSetAlpha(re->ctx, 1.0);
}

static int
eng_context_multiplier_get(void *data, void *context, int *r, int *g, int *b, int *a)
{
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;

   if (r) *r = ctxt->mul.r * 255;
   if (g) *g = ctxt->mul.g * 255;
   if (b) *b = ctxt->mul.b * 255;
   if (a) *a = ctxt->mul.a * 255;
   return ctxt->mul.set;
}

static void
eng_context_cutout_add(void *data, void *context, int x, int y, int w, int h)
{
   // FIXME: This doesn't seem to be implemented anywhere. What does it do?
}

static void
eng_context_cutout_clear(void *data, void *context)
{
   // FIXME: This doesn't seem to be implemented anywhere. What does it do?
}

static void
eng_context_anti_alias_set(void *data, void *context, unsigned char aa)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;
   
   ctxt->aa = aa;
   
   CGContextSetAllowsAntialiasing(re->ctx, (bool)aa);
   CGContextSetShouldAntialias(re->ctx, (bool)aa);
   CGContextSetInterpolationQuality(re->ctx, kCGInterpolationLow); // is it OK to assume low quality?
}

static unsigned char
eng_context_anti_alias_get(void *data, void *context)
{
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;
   
   return ctxt->aa;
}

#pragma mark Rectangle Drawing

static void
eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;
   double r, g, b, a;
   
   flip_pixels(&y, &h, ctxt);

   r = ctxt->col.r;
   g = ctxt->col.g;
   b = ctxt->col.b;
   a = ctxt->col.a;
   
   if (ctxt->mul.set)
   {
      r *= ctxt->mul.r;
      g *= ctxt->mul.g;
      b *= ctxt->mul.b;
      a *= ctxt->mul.a;
   }
   
   CGContextSetRGBFillColor(re->ctx, r, g, b, a);
   CGContextFillRect(re->ctx, CGRectMake(x, y, w, h));
}

#pragma mark Line Drawing

static void
eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;
   double r, g, b, a;
   
   flip_pixels(&y1, NULL, ctxt);
   flip_pixels(&y2, NULL, ctxt);

   r = ctxt->col.r;
   g = ctxt->col.g;
   b = ctxt->col.b;
   a = ctxt->col.a;
   
   if (ctxt->mul.set)
   {
      r *= ctxt->mul.r;
      g *= ctxt->mul.g;
      b *= ctxt->mul.b;
      a *= ctxt->mul.a;
   }
   
   CGContextSetRGBStrokeColor(re->ctx, r, g, b, a);
   CGContextBeginPath(re->ctx);
   CGContextMoveToPoint(re->ctx, x1, y1);
   CGContextAddLineToPoint(re->ctx, x2, y2);
   CGContextStrokePath(re->ctx);
}

#pragma mark Polygon Manipulation & Drawing

static void *
eng_polygon_point_add(void *data, void *context, void *polygon, int x, int y)
{
   Evas_Quartz_Polygon *poly;
   Evas_Quartz_Polygon_Point *pt;
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;
   
   flip_pixels(&y, NULL, ctxt);

   poly = (Evas_Quartz_Polygon *)polygon;
   if (!poly) poly = calloc(1, sizeof(Evas_Quartz_Polygon));
   if (!poly) return NULL;

   pt = calloc(1, sizeof(Evas_Quartz_Polygon_Point));
   if (pt)
   {
      pt->x = x;
      pt->y = y;
      poly->points = eina_list_append(poly->points, pt);
   }
   return poly;
}

static void *
eng_polygon_points_clear(void *data, void *context, void *polygon)
{
   Evas_Quartz_Polygon *poly;

   poly = (Evas_Quartz_Polygon *)polygon;
   if (!poly) return NULL;

   while (poly->points)
   {
      free(poly->points->data);
      poly->points = eina_list_remove_list(poly->points, poly->points);
   }
   free(poly);

   return NULL;
}

static void
eng_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;
   Evas_Quartz_Polygon *poly = (Evas_Quartz_Polygon *)polygon;
   Evas_Quartz_Polygon_Point *pt;
   
   double r, g, b, a;
   
   CGContextBeginPath(re->ctx);

   pt = poly->points->data;
   if (pt)
   {
      Eina_List *l;
      CGContextMoveToPoint(re->ctx, pt->x, pt->y);
      EINA_LIST_FOREACH(poly->points->next, l, pt)
	CGContextAddLineToPoint(re->ctx, pt->x, pt->y);
   }

   r = ctxt->col.r;
   g = ctxt->col.g;
   b = ctxt->col.b;
   a = ctxt->col.a;

   if (ctxt->mul.set)
   {
      r *= ctxt->mul.r;
      g *= ctxt->mul.g;
      b *= ctxt->mul.b;
      a *= ctxt->mul.a;
   }
   
   CGContextSetRGBFillColor(re->ctx, r, g, b, a);
   CGContextFillPath(re->ctx);
}

#pragma mark Gradient Manipulation & Drawing

static void *
eng_gradient_new(void *data)
{
   Evas_Quartz_Gradient *gradient = calloc(1, sizeof(Evas_Quartz_Gradient));
   if (!gradient) return NULL;
   
   gradient->grad = evas_common_gradient_new();
   if (!(gradient->grad))
   {
      free(gradient);
      return NULL;
   }
   
   gradient->changed = 1;
   gradient->buf = NULL;

   return gradient;
}

static void
eng_gradient_free(void *data, void *gradient)
{
   Evas_Quartz_Gradient *gr = (Evas_Quartz_Gradient *)gradient;
   if (!gr) return;
   if (gr->grad) evas_common_gradient_free(gr->grad);
   if (gr->im) eng_image_free(data, gr->im);
   if (gr->buf) free(gr->buf);
   free(gr);
}

static void
eng_gradient_color_stop_add(void *data, void *gradient, int r, int g, int b, int a, int delta)
{
   Evas_Quartz_Gradient *gr = (Evas_Quartz_Gradient *)gradient;
   if (!gr) return;
   evas_common_gradient_color_stop_add(gr->grad, r, g, b, a, delta);
   gr->changed = 1;
}

static void
eng_gradient_alpha_stop_add(void *data, void *gradient, int a, int delta)
{
   Evas_Quartz_Gradient *gr = (Evas_Quartz_Gradient *)gradient;
   if (!gr) return;
   evas_common_gradient_alpha_stop_add(gr->grad, a, delta);
   gr->changed = 1;
}

static void
eng_gradient_color_data_set(void *data, void *gradient, void *map, int len, int has_alpha)
{
   Evas_Quartz_Gradient *gr = (Evas_Quartz_Gradient *)gradient;
   if (!gr) return;
   evas_common_gradient_color_data_set(gr->grad, map, len, has_alpha);
   gr->changed = 1;
}

static void
eng_gradient_alpha_data_set(void *data, void *gradient, void *alpha_map, int len)
{
   Evas_Quartz_Gradient *gr = (Evas_Quartz_Gradient *)gradient;
   if (!gr) return;
   evas_common_gradient_alpha_data_set(gr->grad, alpha_map, len);
   gr->changed = 1;
}

static void
eng_gradient_clear(void *data, void *gradient)
{
   Evas_Quartz_Gradient *gr = (Evas_Quartz_Gradient *)gradient;
   if (!gr) return;
   evas_common_gradient_clear(gr->grad);
   gr->changed = 1;
}

static void
eng_gradient_fill_set(void *data, void *gradient, int x, int y, int w, int h)
{
   Evas_Quartz_Gradient *gr = (Evas_Quartz_Gradient *)gradient;
   if (!gr) return;
   evas_common_gradient_fill_set(gr->grad, x, y, w, h);
   gr->changed = 1;
}

static void
eng_gradient_fill_angle_set(void *data, void *gradient, double angle)
{
   Evas_Quartz_Gradient *gr = (Evas_Quartz_Gradient *)gradient;
   if (!gr) return;
   evas_common_gradient_fill_angle_set(gr->grad, angle);
   gr->changed = 1;
}

static void
eng_gradient_fill_spread_set(void *data, void *gradient, int spread)
{
   Evas_Quartz_Gradient *gr = (Evas_Quartz_Gradient *)gradient;
   if (!gr) return;
   evas_common_gradient_fill_spread_set(gr->grad, spread);
   gr->changed = 1;
}

static void
eng_gradient_angle_set(void *data, void *gradient, double angle)
{
   Evas_Quartz_Gradient *gr = (Evas_Quartz_Gradient *)gradient;
   if (!gr) return;
   evas_common_gradient_map_angle_set(gr->grad, angle);
   gr->changed = 1;
}

static void
eng_gradient_offset_set(void *data, void *gradient, float offset)
{
   Evas_Quartz_Gradient *gr = (Evas_Quartz_Gradient *)gradient;
   if (!gr) return;
   evas_common_gradient_map_offset_set(gr->grad, offset);
   gr->changed = 1;
}

static void
eng_gradient_direction_set(void *data, void *gradient, int direction)
{
   Evas_Quartz_Gradient *gr = (Evas_Quartz_Gradient *)gradient;
   if (!gr) return;
   evas_common_gradient_map_direction_set(gr->grad, direction);
   gr->changed = 1;
}

static void
eng_gradient_type_set(void *data, void *gradient, char *name, char *params)
{
   Evas_Quartz_Gradient *gr = (Evas_Quartz_Gradient *)gradient;
   if (!gr) return;
   evas_common_gradient_type_set(gr->grad, name, params);
   gr->changed = 1;
}

static int
eng_gradient_is_opaque(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   RGBA_Gradient *grad;
   if (!gradient) return 0;
   
   grad = ((Evas_Quartz_Gradient *)gradient)->grad;

   if (!grad || !grad->type.geometer) return 0;
   
   return !(grad->type.geometer->has_alpha(grad, dc->render_op) |
              grad->type.geometer->has_mask(grad, dc->render_op));
}

static int
eng_gradient_is_visible(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   if (!gradient) return 0;
   return 1;
}

static void
eng_gradient_render_pre(void *data, void *context, void *gradient)
{
   int  len;
   RGBA_Gradient *grad;

   if (!context || !gradient) return;
   grad = ((Evas_Quartz_Gradient *)gradient)->grad;
   if (!grad || !grad->type.geometer) return;
   grad->type.geometer->geom_set(grad);
   len = grad->type.geometer->get_map_len(grad);
   evas_common_gradient_map(context, grad, len);
   ((Evas_Quartz_Gradient *)gradient)->changed = 1;
}

static void
eng_gradient_render_post(void *data, void *gradient)
{
}

static void
eng_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   Evas_Quartz_Gradient *gr = (Evas_Quartz_Gradient *)gradient;
   
   printf("#Gradient#\n");
   printf("Fill: %i %i %i %i\n", gr->grad->fill.x, gr->grad->fill.y, gr->grad->fill.w, gr->grad->fill.h);
   printf("Type: %s %s\n", gr->grad->type.name, gr->grad->type.params);
   printf("XYWH: %i %i %i %i\n", x, y, w, h);
   printf("Geom: %p %p\n", gr->grad->type.geometer, gr->grad->type.geometer->get_fill_func);
   printf("Map: len: %d angle: %f direction: %d offset: %f\n", gr->grad->map.len, gr->grad->map.angle, gr->grad->map.direction, gr->grad->map.offset);
   printf("Color: nstops: %d len: %d\n", gr->grad->color.nstops, gr->grad->color.len);
   printf("Alpha: nstops: %d len: %d\n", gr->grad->alpha.nstops, gr->grad->alpha.len);
   printf("\n");
   
   if ((gr->sw != w) || (gr->sh != h))
      gr->changed = 1;
   
   if ((gr->changed) || (!gr->im))
   {
      if (gr->buf && ((gr->sw != w) || (gr->sh != h)))
      {
         free(gr->buf);
         gr->buf = NULL;
      }
      
      if (!gr->buf)
         gr->buf = calloc(w * h, 4);
      
      eng_image_free(data, gr->im);
      gr->im = eng_image_new_from_data(data, w, h, gr->buf, 1, EVAS_COLORSPACE_ARGB8888);
      dc->render_op = _EVAS_RENDER_FILL;
      evas_common_gradient_draw((RGBA_Image *) &gr->im->im->cache_entry, dc, 0, 0, w, h, gr->grad);
      evas_common_cpu_end_opt();
      gr->sw = w;
      gr->sh = h;
      gr->changed = 0;
   }
   
   eng_image_draw(data, context, NULL, gr->im, 0, 0, 0, 0, x, y, w, h, 0);
}

#pragma mark Image Manipulation & Drawing

static void *
eng_image_load(void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Evas_Quartz_Image *im = calloc(1, sizeof(Evas_Quartz_Image));
   
   // FIXME: set error before returning without a new image...
   // I can't figure out what to set it to, even trying to follow through the core code.
   // Also, none of the other engines set it.
   
   if (error) *error = 0;
   if (!im) return NULL;

   im->im = (RGBA_Image *)evas_common_load_image_from_file(file, key, lo);
   if (!im->im)
   {
      free(im);
      return NULL;
   }
   im->references = 1;
   return im;
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Evas_Quartz_Image *im = calloc(1, sizeof(Evas_Quartz_Image));
   
   if (!im) return NULL;
   
   im->im = (RGBA_Image *)evas_cache_image_data(evas_common_image_cache_get(), w, h, image_data, alpha, cspace);
   im->references = 1;
   
   if (!im->im)
   {
      free(im);
      return NULL;
   }
   
   return im;
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Evas_Quartz_Image *im = calloc(1, sizeof(Evas_Quartz_Image));
   
   if (!im) return NULL;
   
   im->im = (RGBA_Image *)evas_cache_image_copied_data(evas_common_image_cache_get(), w, h, image_data, alpha, cspace);
   im->references = 1;
   
   if (!im->im)
   {
      free(im);
      return NULL;
   }
   
   return im;
}

static void
eng_image_free(void *data, void *image)
{
   Evas_Quartz_Image *im = (Evas_Quartz_Image *)image;
   
   if (!im) return;
   
   if (--im->references > 0) return;
   
   if (im->cgim) CGImageRelease(im->cgim);
   if (im->im) evas_cache_image_drop(&im->im->cache_entry);
   free(im);
}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   Evas_Quartz_Image *im = (Evas_Quartz_Image *)image;
   Evas_Quartz_Image *im_old;
   
   if (!im) return NULL;
   im_old = image;
   if ((eng_image_colorspace_get(data, image) == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (eng_image_colorspace_get(data, image) == EVAS_COLORSPACE_YCBCR422P709_PL))
      w &= ~0x1;

   if ((im_old) && (im_old->im->cache_entry.w == w) && (im_old->im->cache_entry.h == h))
      return image;
      
   if ((w <= 0) || (h <= 0))
   {
      eng_image_free(data, im_old);
      return NULL;
   }

   if (im_old)
   {
      im = eng_image_new_from_data(data, w, h, im_old->im->image.data, eng_image_alpha_get(data, im_old), eng_image_colorspace_get(data, im_old));
      eng_image_free(data, im_old);
   }
   else
      im = eng_image_new_from_data(data, w, h, NULL, true, EVAS_COLORSPACE_ARGB8888);
   return im;
}

static void 
eng_image_size_get(void *data, void *image, int *w, int *h)
{
   Evas_Quartz_Image *im = (Evas_Quartz_Image *)image;

   if (!image)
   {
      if (w) *w = 0;
      if (h) *h = 0;
   }
   else
   {
      if (w) *w = im->im->cache_entry.w;
      if (h) *h = im->im->cache_entry.h;
   }
}

static void *
eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   Evas_Quartz_Image *im = (Evas_Quartz_Image *)image;
   
   if ((im) && (im->im))
      return evas_cache_image_dirty(&im->im->cache_entry, x, y, w, h);
}

static void *
eng_image_alpha_set(void *data, void *image, int has_alpha)
{
   Evas_Quartz_Image *im = (Evas_Quartz_Image *)image;
   
   if ((!im) || (!im->im)) return NULL;
      
   if (im->im->cache_entry.space != EVAS_COLORSPACE_ARGB8888)
   {
      im->im->cache_entry.flags.alpha = 0;
   }
   else
   {
      im->im = (RGBA_Image *)evas_cache_image_alone(&im->im->cache_entry);
      evas_common_image_colorspace_dirty(im->im);
      im->im->cache_entry.flags.alpha = has_alpha ? 1 : 0;
   }
   
   return im;
}

static int
eng_image_alpha_get(void *data, void *image)
{
   Evas_Quartz_Image *im = (Evas_Quartz_Image *) image;
   
   if (!im->im) return 0;
   
   return (int)(im->im->cache_entry.flags.alpha);
}

static char *
eng_image_comment_get(void *data, void *image, char *key)
{
   Evas_Quartz_Image *im = (Evas_Quartz_Image *)image;
   
   if ((!im) || (!im->im))
      return NULL;
   else
      return im->im->info.comment;
}

static char *
eng_image_format_get(void *data, void *image)
{
   // these are unimplemented for now, until core features are finished

   return NULL;
}

static void
eng_image_colorspace_set(void *data, void *image, int cspace)
{
   Evas_Quartz_Image *im = (Evas_Quartz_Image *)image;
   
   if (!im)
      return;
   else
      evas_cache_image_colorspace(&im->im->cache_entry, cspace);
}

static int
eng_image_colorspace_get(void *data, void *image)
{
   Evas_Quartz_Image *im = (Evas_Quartz_Image *)image;
   
   if (!im)
      return EVAS_COLORSPACE_ARGB8888;
   else
      return im->im->cache_entry.space;
}

static void
eng_image_native_set(void *data, void *image, void *native)
{
   // these are unimplemented for now, until core features are finished
}

static void *
eng_image_native_get(void *data, void *image)
{
   // these are unimplemented for now, until core features are finished
   return NULL;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   // FIXME: one last leak somewhere in this function
   
   Evas_Quartz_Image *im_old = (Evas_Quartz_Image *)image;

   if (!im_old) return NULL;
   
   if (im_old->cgim)
   {
      CGImageRelease(im_old->cgim);
      im_old->cgim = NULL;
   }
   
   switch (im_old->im->cache_entry.space)
   {
      case EVAS_COLORSPACE_ARGB8888:
         image = eng_image_new_from_data(data, im_old->im->cache_entry.w, im_old->im->cache_entry.h, image_data, 1, EVAS_COLORSPACE_ARGB8888);
         eng_image_free(data, im_old);
         break;
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422P601_PL:
         if (image_data != im_old->im->cs.data)
         {
            if (im_old->im->cs.data)
               if (!im_old->im->cs.no_free)
                  free(im_old->im->cs.data);
            
            im_old->im->cs.data = image_data;
            evas_common_image_colorspace_dirty(im_old->im);
         }
         break;
   }
   
   if (!image) return NULL;

   return image;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   Evas_Quartz_Image *im = (Evas_Quartz_Image *)image;

   if ((!im) || (!im->im))
   {
      *image_data = NULL;
      return NULL;
   }
   
   switch (im->im->cache_entry.space)
   {
      case EVAS_COLORSPACE_ARGB8888:
         if (to_write)
         {
            CGImageRelease(im->cgim);
            im->cgim = NULL;
            
            if (im->references > 1)
            {
               Evas_Quartz_Image *im_new;
               im_new = eng_image_new_from_copied_data(data,
                                                       im->im->cache_entry.w,
                                                       im->im->cache_entry.h,
                                                       im->im->image.data,
                                                       eng_image_alpha_get(data, image),
                                                       eng_image_colorspace_get(data, image));
               
               if (!im_new)
               {
                  if (image_data) *image_data = NULL;
                  return im;
               }
               
               eng_image_free(data, im);
               im = im_new;
            }
            else
            {
               im->im = (RGBA_Image *)evas_cache_image_dirty(&im->im->cache_entry,
                                                            0, 0,
                                                            im->im->cache_entry.w, im->im->cache_entry.h);
               im->references++;
            }
         }
         
         evas_cache_image_load_data(&im->im->cache_entry);
         
         if (image_data) *image_data = im->im->image.data;
         break;
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422P601_PL:
         if (image_data) *image_data = im->im->cs.data;
         im->references++;
         break;
      default:
         abort(); // this seems ... incredibly unreasonable, but GL does it...
         break;
   }
   
   return im;
}

static void
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;
   Evas_Quartz_Image *im = (Evas_Quartz_Image *)image;
   flip_pixels(&dst_y, &dst_h, ctxt);

   if ((!im) || (!im->im)) return;
   
   if (!im->cgim)
   {
      CGColorSpaceRef colorspace;
      CGDataProviderRef provider;
      evas_cache_image_load_data(&im->im->cache_entry);
      evas_common_image_colorspace_normalize((RGBA_Image *)(&im->im->cache_entry));
      
      if (!im->im->image.data) return;
      
      colorspace = CGColorSpaceCreateDeviceRGB();
      provider = CGDataProviderCreateWithData(NULL,
                                              im->im->image.data,
                                              im->im->cache_entry.w * im->im->cache_entry.h * sizeof(DATA32),
                                              NULL);
      im->cgim = CGImageCreate(im->im->cache_entry.w,
                               im->im->cache_entry.h,
                               8,
                               8 * 4,
                               4 * im->im->cache_entry.w,
                               colorspace,
                               kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host,
                               provider,
                               NULL,
                               smooth,
                               kCGRenderingIntentDefault);
                               
      CGDataProviderRelease(provider);
      CGColorSpaceRelease(colorspace);
      
      if (!im->cgim) return;
   }
   
   CGImageRef subImage = NULL;
   
   if (src_x != 0 || src_y != 0 || src_w != 0 || src_h != 0)
   {
      subImage = CGImageCreateWithImageInRect(im->cgim, CGRectMake(src_x,src_y,src_w,src_h));
      if (!subImage) return;

      CGContextDrawImage(re->ctx, CGRectMake(dst_x, dst_y, dst_w, dst_h), subImage);
      CGImageRelease(subImage);
   }
   else
      CGContextDrawImage(re->ctx, CGRectMake(dst_x, dst_y, dst_w, dst_h), im->cgim);
}

#pragma mark Text Manipulation & Drawing

static Evas_Quartz_Font *
quartz_font_from_ats(ATSFontContainerRef container, int size)
{
   ItemCount count;
   ATSFontRef *fonts;
   CTFontRef font;
   CFStringRef keys[1];
   CFTypeRef values[1];
   CFDictionaryRef attr;
   Evas_Quartz_Font *loaded_font;
   
   ATSFontFindFromContainer(container, kATSOptionFlagsDefault, 0, NULL, &count);
   fonts = calloc(count, sizeof(ATSFontRef));
   if (!fonts) return NULL;
   ATSFontFindFromContainer(container, kATSOptionFlagsDefault, count, fonts, NULL);
   
   font = CTFontCreateWithPlatformFont(fonts[0], size, NULL, NULL);
   
   loaded_font = calloc(1, sizeof(Evas_Quartz_Font));
   if (!font || !loaded_font) return NULL;
   
   keys[0] = kCTFontAttributeName;
   values[0] = font;
   attr = CFDictionaryCreate(NULL,
                            (const void **)&keys,
                            (const void **)&values,
                            sizeof(keys) / sizeof(keys[0]),
                            &kCFTypeDictionaryKeyCallBacks,
                            &kCFTypeDictionaryValueCallBacks);
   
   loaded_font->font = font;
   loaded_font->attr = attr;
   loaded_font->size = size;
   
   free(fonts);
   
   return loaded_font;
}

static void *
eng_font_add(void *data, void *font, const char *name, int size)
{
   // FIXME: what is this function supposed to do?
   // if I delete it, we eventually crash when it gets run
   
   return (char *)name;
}

static void *
eng_font_load(void *data, const char *name, int size)
{
   FSRef fontFile;
   ATSFontContainerRef container;
   
   FSPathMakeRef((unsigned char *)name, &fontFile, NULL);
   ATSFontActivateFromFileReference(&fontFile, kATSFontContextLocal, kATSFontFormatUnspecified, NULL, kATSOptionFlagsDefault, &container);
   
   return quartz_font_from_ats(container, size);;
}

static void *
eng_font_memory_load(void *data, char *name, int size, const void *fdata, int fdata_size)
{
   ATSFontContainerRef container;
   
   ATSFontActivateFromMemory((void *)fdata, fdata_size, kATSFontContextLocal, kATSFontFormatUnspecified, NULL, kATSOptionFlagsDefault, &container);

   return quartz_font_from_ats(container, size);
}

static void
eng_font_free(void *data, void *font)
{
   Evas_Quartz_Font *loaded_font = (Evas_Quartz_Font *)font;

   CFRelease(loaded_font->font);
   CFRelease(loaded_font->attr);
   free(loaded_font);
}

static int
eng_font_ascent_get(void *data, void *font)
{
   Evas_Quartz_Font *loaded_font = (Evas_Quartz_Font *)font;
   
   return CTFontGetAscent(loaded_font->font);
}

static int
eng_font_descent_get(void *data, void *font)
{
   Evas_Quartz_Font *loaded_font = (Evas_Quartz_Font *)font;
   
   return CTFontGetDescent(loaded_font->font);
}

static int
eng_font_max_ascent_get(void *data, void *font)
{
   Evas_Quartz_Font *loaded_font = (Evas_Quartz_Font *)font;
   
   return CTFontGetAscent(loaded_font->font);
}

static int
eng_font_max_descent_get(void *data, void *font)
{
   Evas_Quartz_Font *loaded_font = (Evas_Quartz_Font *)font;
   
   return CTFontGetDescent(loaded_font->font);
}

static void
eng_font_string_size_get(void *data, void *font, const char *text, int *w, int *h)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Quartz_Font *loaded_font = (Evas_Quartz_Font *)font;
   
   CFStringRef string = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
   CFAttributedStringRef attrString = CFAttributedStringCreate(NULL, string, loaded_font->attr);
   CTLineRef line = CTLineCreateWithAttributedString(attrString);
   CGContextSetTextMatrix(re->ctx, CGAffineTransformIdentity);
   CGContextSetTextPosition(re->ctx, 0, CTFontGetDescent(loaded_font->font));
   CGRect bounds = CTLineGetImageBounds(line, re->ctx);
   
   // Descenders on characters seem to leave origin at typographic origin, instead of image origin.
   // Evas expects text to be treated like an image, so we have to offset by the typographic origin.
   
   if (w) (*w) = ceil(bounds.size.width + bounds.origin.x);
   if (h) (*h) = ceil(bounds.size.height + bounds.origin.y);
   
   CFRelease(attrString);
   CFRelease(string);
   CFRelease(line);
}

static int
eng_font_inset_get(void *data, void *font, const char *text)
{
   return 0;
}

static int
eng_font_h_advance_get(void *data, void *font, const char *text)
{
   int w;
   
   eng_font_string_size_get(data, font, text, &w, NULL);
   
   return w + 2; // FIXME: shouldn't need a constant here. from where do we get word spacing?
   // it seems we lose the space between differently-styled text in a text block. Why?
}

static int
eng_font_v_advance_get(void *data, void *font, const char *text)
{
   int h;
   
   eng_font_string_size_get(data, font, text, NULL, &h);
   
   return h;
}

static int
eng_font_char_coords_get(void *data, void *font, const char *text, int pos, int *cx, int *cy, int *cw, int *ch)
{
   Evas_Quartz_Font *loaded_font = (Evas_Quartz_Font *)font;
   
   CFStringRef string = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
   CFAttributedStringRef attrString = CFAttributedStringCreate(NULL, string, loaded_font->attr);
   CTLineRef line = CTLineCreateWithAttributedString(attrString);
   
   float offset = CTLineGetOffsetForStringIndex(line, pos, NULL);
   if (cx) *cx = offset;
   if (cy) *cy = loaded_font->size;
   
   CFRelease(attrString);
   CFRelease(string);
   CFRelease(line);
   
   return 1;
}

static int
eng_font_char_at_coords_get(void *data, void *font, const char *text, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   // Return the index of the character at the given point, also lookup it's origin x, y, w, and h.
   Evas_Quartz_Font *loaded_font = (Evas_Quartz_Font *)font;
   
   CFStringRef string = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
   CFAttributedStringRef attrString = CFAttributedStringCreate(NULL, string, loaded_font->attr);
   CTLineRef line = CTLineCreateWithAttributedString(attrString);
   
   int stringIndex = (int) CTLineGetStringIndexForPosition(line, CGPointMake(x, y));
   
   // In order to get the character's size and position, look up the position of this character and the next one
   eng_font_char_coords_get(data, font, text, stringIndex, cx, cy, NULL, NULL);
   eng_font_char_coords_get(data, font, text, stringIndex + 1, cw, NULL, NULL, NULL);
   
   if (cw && cx) *cw -= *cx;
   if (ch) *ch = loaded_font->size;

   CFRelease(attrString);
   CFRelease(string);
   CFRelease(line);
   
   return stringIndex;
}

static void
eng_font_hinting_set(void *data, void *font, int hinting)
{
   Evas_Quartz_Font *loaded_font = (Evas_Quartz_Font *)font;
   loaded_font->hint = hinting;
}

static int
eng_font_hinting_can_hint(void *data, int hinting)
{
   return 1;
}

static void
eng_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, const char *text)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Quartz_Context *ctxt = (Evas_Quartz_Context *)context;
   Evas_Quartz_Font *loaded_font = (Evas_Quartz_Font *)font;
   double r, g, b, a;
   CGFloat colors[4];
   CGColorSpaceRef colorspace;
   CGColorRef color;
   CFStringRef keys[2];
   CFTypeRef values[2];
   CFDictionaryRef attr;
   CFStringRef string;
   CFAttributedStringRef attrString;
   CTLineRef line;
   
   flip_pixels(&y, &h, ctxt);
   
   // FIXME: I know this should never happen, but the next line is magic.
   // It's also... broken. It /works/, but ... for example, subtracting 1 shouldn't need to happen.
   // The text drawing is a mess, but this is the closest I've gotten yet...
   
   y += floor(CTFontGetAscent(loaded_font->font) - (CTFontGetXHeight(loaded_font->font) + CTFontGetDescent(loaded_font->font))) - 1;
   
   CGContextSetShouldSmoothFonts(re->ctx, (bool)(loaded_font->hint));
   
   r = ctxt->col.r;
   g = ctxt->col.g;
   b = ctxt->col.b;
   a = ctxt->col.a;

   if (ctxt->mul.set)
   {
      r *= ctxt->mul.r;
      g *= ctxt->mul.g;
      b *= ctxt->mul.b;
      a *= ctxt->mul.a;
   }
   
   colors[0] = r;
   colors[1] = g;
   colors[2] = b;
   colors[3] = a;
   
   // Create an attributed string
   colorspace = CGColorSpaceCreateDeviceRGB();
   color = CGColorCreate(colorspace, colors);
   
   keys[0] =   kCTFontAttributeName;
   values[0] = loaded_font->font;
   keys[1] =   kCTForegroundColorAttributeName;
   values[1] = color;
   
   attr = CFDictionaryCreate(NULL, (const void **)&keys, (const void **)&values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
   string = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
   attrString = CFAttributedStringCreate(NULL, string, attr);

   // Draw the string
   line = CTLineCreateWithAttributedString(attrString);
   CGContextSetTextMatrix(re->ctx, CGAffineTransformIdentity);
   CGContextSetTextPosition(re->ctx, x, y + h);
   CTLineDraw(line, re->ctx);

   // Clean up
   CGColorSpaceRelease(colorspace);
   CFRelease(attr);
   CFRelease(string);
   CFRelease(attrString);
   CFRelease(line);
   CGColorRelease(color);
}

#pragma mark Module Function Export

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   if (!_evas_module_engine_inherit(&func, "software_generic")) return 0;
   #define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(context_anti_alias_get);
   ORD(context_anti_alias_set);
   ORD(context_clip_get);
   ORD(context_clip_set);
   ORD(context_clip_unset);
   ORD(context_color_get);
   ORD(context_color_set);
   ORD(context_cutout_add);
   ORD(context_cutout_clear);
   ORD(context_free);
   ORD(context_multiplier_get);
   ORD(context_multiplier_set);
   ORD(context_multiplier_unset);
   ORD(context_new);
   ORD(font_add);
   ORD(font_ascent_get);
   ORD(font_char_at_coords_get);
   ORD(font_char_coords_get);
   ORD(font_descent_get);
   ORD(font_draw);
   ORD(font_free);
   ORD(font_hinting_can_hint);
   ORD(font_hinting_set);
   ORD(font_h_advance_get);
   ORD(font_inset_get);
   ORD(font_load);
   ORD(font_max_ascent_get);
   ORD(font_max_descent_get);
   ORD(font_memory_load);
   ORD(font_string_size_get);
   ORD(font_v_advance_get);
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
   ORD(image_alpha_get);
   ORD(image_alpha_set);
   ORD(image_colorspace_get);
   ORD(image_colorspace_set);
   ORD(image_comment_get);
   ORD(image_data_get);
   ORD(image_data_put);
   ORD(image_dirty_region);
   ORD(image_draw);
   ORD(image_format_get);
   ORD(image_free);
   ORD(image_load);
   ORD(image_native_get);
   ORD(image_native_set);
   ORD(image_new_from_copied_data);
   ORD(image_new_from_data);
   ORD(image_size_get);
   ORD(image_size_set);
   ORD(info);
   ORD(info_free);
   ORD(line_draw);
   ORD(output_flush);
   ORD(output_free);
   ORD(output_redraws_clear);
   ORD(output_redraws_next_update_get);
   ORD(output_redraws_next_update_push);
   ORD(output_redraws_rect_add);
   ORD(output_redraws_rect_del);
   ORD(output_resize);
   ORD(polygon_draw);
   ORD(polygon_points_clear);
   ORD(polygon_point_add);
   ORD(rectangle_draw);
   ORD(setup);

   /* now advertise out our api */
   em->functions = (void *)(&func);
   return 1;
}

EAPI void
module_close(void)
{
   
}

EAPI Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
     EVAS_MODULE_TYPE_ENGINE,
     "quartz",
     "none"
};
