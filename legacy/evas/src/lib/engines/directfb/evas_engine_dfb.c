#include "evas_engine_dfb.h"
#include <math.h>
#include <string.h>

Evas_Func           evas_engine_directfb_func = {
   evas_engine_directfb_info,
   evas_engine_directfb_info_free,
   evas_engine_directfb_setup,
   evas_engine_directfb_output_free,
   evas_engine_directfb_output_resize,
   evas_engine_directfb_output_tile_size_set,
   evas_engine_directfb_output_redraws_rect_add,
   evas_engine_directfb_output_redraws_rect_del,
   evas_engine_directfb_output_redraws_clear,
   evas_engine_directfb_output_redraws_next_update_get,
   evas_engine_directfb_output_redraws_next_update_push,
   evas_engine_directfb_output_flush,
   /* draw context virtual methods */
   evas_engine_directfb_context_new,
   evas_engine_directfb_context_free,
   evas_engine_directfb_context_clip_set,
   evas_engine_directfb_context_clip_clip,
   evas_engine_directfb_context_clip_unset,
   evas_engine_directfb_context_clip_get,
   evas_engine_directfb_context_color_set,
   evas_engine_directfb_context_color_get,
   evas_engine_directfb_context_multiplier_set,
   evas_engine_directfb_context_multiplier_unset,
   evas_engine_directfb_context_multiplier_get,
   evas_engine_directfb_context_cutout_add,
   evas_engine_directfb_context_cutout_clear,
   /* rectangle draw funcs */
   evas_engine_directfb_draw_rectangle,
   /* line draw funcs */
   evas_engine_directfb_line_draw,
   /* poly draw funcs */
   evas_engine_directfb_polygon_point_add,
   evas_engine_directfb_polygon_points_clear,
   evas_engine_directfb_polygon_draw,
   /* gardient draw funcs */
   evas_engine_directfb_gradient_color_add,
   evas_engine_directfb_gradient_colors_clear,
   evas_engine_directfb_gradient_draw,
   /* image draw funcs */
   evas_engine_directfb_image_load,
   evas_engine_directfb_image_new_from_data,
   evas_engine_directfb_image_new_from_copied_data,
   evas_engine_directfb_image_free,
   evas_engine_directfb_image_size_get,
   evas_engine_directfb_image_size_set,
   evas_engine_directfb_image_dirty_region,
   evas_engine_directfb_image_data_get,
   evas_engine_directfb_image_data_put,
   evas_engine_directfb_image_alpha_set,
   evas_engine_directfb_image_alpha_get,
   evas_engine_directfb_image_draw,
   evas_engine_directfb_image_comment_get,
   evas_engine_directfb_image_format_get,

   evas_engine_directfb_image_cache_flush,
   evas_engine_directfb_image_cache_set,
   evas_engine_directfb_image_cache_get,
   /* more to come */
   evas_engine_directfb_font_load,
   evas_engine_directfb_font_free,
   evas_engine_directfb_font_ascent_get,
   evas_engine_directfb_font_descent_get,
   evas_engine_directfb_font_max_ascent_get,
   evas_engine_directfb_font_max_descent_get,
   evas_engine_directfb_font_string_size_get,
   evas_engine_directfb_font_inset_get,
   evas_engine_directfb_font_h_advance_get,
   evas_engine_directfb_font_v_advance_get,
   evas_engine_directfb_font_char_coords_get,
   evas_engine_directfb_font_char_at_coords_get,
   evas_engine_directfb_font_draw,
   /* font cache functions */
   evas_engine_directfb_font_cache_flush,
   evas_engine_directfb_font_cache_set,
   evas_engine_directfb_font_cache_get
};


   
void        *
evas_engine_directfb_info(Evas * e)
{
   Evas_Engine_Info_DirectFB *info;

   info = calloc(1, sizeof(Evas_Engine_Info_DirectFB));
   if (!info)
      return NULL;

   info->magic.magic = rand();

   return info;
   e = NULL;
}

void
evas_engine_directfb_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_DirectFB *in;
   
   in = (Evas_Engine_Info_DirectFB *)info;
   free(in);
}

void
evas_engine_directfb_setup(Evas * e, void *in)
{
   Evas_Engine_Info_DirectFB *info;
   IDirectFBImageProvider *provider;

   info = (Evas_Engine_Info_DirectFB *) in;
   /* if we arent set to directfb, why the hell do we get called?! */
   if (evas_output_method_get(e) != RENDER_METHOD_DIRECTFB)
      return;
   /* lets just set up */
   e->engine.data.output = evas_engine_directfb_output_setup(e->output.w,
							     e->output.h,
							     info->info.dfb,
							     info->info.surface,
							     info->info.flags);
   if (!e->engine.data.output)
      return;
   e->engine.func = &evas_engine_directfb_func;
   e->engine.data.context = e->engine.func->context_new(e->engine.data.output);
}

void        *
evas_engine_directfb_output_setup(int w, int h, IDirectFB * dfb,
				  IDirectFBSurface * surf,
				  DFBSurfaceDrawingFlags flags)
{
   Render_Engine      *re;
   IDirectFBSurface   *buf;
   DFBSurfaceDescription dsc;
   RGBA_Image *im;

   re = calloc(1, sizeof(Render_Engine));
   /* if we haven't initialized - init (automatic abort if already done) */
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

   re->tb = evas_common_tilebuf_new(w, h);
   /* in preliminary tests 16x16 gave highest framerates */
   evas_common_tilebuf_set_tile_size(re->tb, 16, 16);
   re->dfb = dfb;
   re->surface = surf;

   memset(&dsc, 0, sizeof(DFBSurfaceDescription));
   dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT;
   dsc.width = w;
   dsc.height = h;
   dsc.pixelformat = DSPF_ARGB;

   if (dfb->CreateSurface(dfb, &dsc, &buf) == DFB_OK)
      re->backbuf = buf;
   /* FIXME else exit with error. */

   re->backbuf->SetDrawingFlags(re->backbuf, flags);

   /* We create a "fake" RGBA_Image which points the to DFB surface. Each access 
    * to that surface is wrapped in Lock / Unlock calls whenever the data is
    * manipulated directly. */
   im = evas_common_image_new();
   if (!im) return;
   im->image = evas_common_image_surface_new();
   if (!im->image) 
   {
      evas_common_image_free(im);
      return;
   }
   im->image->w = w;
   im->image->h = h;
   im->image->data =(void*) re->backbuf;
   im->image->no_free = 1;
   re->rgba_image = im;

   return re;
}

void
evas_engine_directfb_output_free(void *data)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_tilebuf_free(re->tb);
   if (re->rects)
      evas_common_tilebuf_free_render_rects(re->rects);
   re->backbuf->Release(re->backbuf);
   evas_common_image_free(re->rgba_image);
   free(re);
}

void
evas_engine_directfb_output_resize(void *data, int w, int h)
{
   Render_Engine      *re;
   IDirectFBSurface *new_surf;
   DFBSurfaceDescription dsc;

   re = (Render_Engine *) data;
   if (w == re->tb->outbuf_w && h == re->tb->outbuf_h) return;
   evas_common_tilebuf_free(re->tb);
   re->tb = evas_common_tilebuf_new(w, h);
   if (re->tb)
      evas_common_tilebuf_set_tile_size(re->tb, 16, 16);

   memset(&dsc, 0, sizeof(DFBSurfaceDescription));
   dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT;
   dsc.width = w;
   dsc.height = h;
   dsc.pixelformat = DSPF_ARGB;

   if (re->dfb->CreateSurface(re->dfb, &dsc, &new_surf) == DFB_OK)
   {
      new_surf->StretchBlit(new_surf, re->backbuf, NULL, NULL);
      re->backbuf->Release(re->backbuf);
      re->backbuf = new_surf;
      re->rgba_image->image->w = w;
      re->rgba_image->image->h = h;
      re->rgba_image->image->data = new_surf;
   }
}

 void
evas_engine_directfb_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

void
evas_engine_directfb_output_redraws_rect_add(void *data, int x, int y, int w,
					     int h)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);
}

void
evas_engine_directfb_output_redraws_rect_del(void *data, int x, int y, int w,
					     int h)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

void
evas_engine_directfb_output_redraws_clear(void *data)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_tilebuf_clear(re->tb);
}

void        *
evas_engine_directfb_output_redraws_next_update_get(void *data, int *x, int *y,
						    int *w, int *h, int *cx,
						    int *cy, int *cw, int *ch)
{
   Render_Engine      *re;
   RGBA_Image         *surface;
   Tilebuf_Rect       *rect;
   Evas_Object_List   *l;
   int                 ux, uy, uw, uh;
   static             first = 1;

   re = (Render_Engine *) data;
   if (re->end)
     {
	re->end = 0;
	return NULL;
     }
   if (!re->rects)
     {
	re->rects = evas_common_tilebuf_get_render_rects(re->tb);
	re->cur_rect = (Evas_Object_List *) re->rects;
     }
   if (!re->cur_rect)
      return NULL;
   rect = (Tilebuf_Rect *) re->cur_rect;
   ux = rect->x;
   uy = rect->y;
   uw = rect->w;
   uh = rect->h;
   re->cur_rect = re->cur_rect->next;
   if (!re->cur_rect)
     {
	evas_common_tilebuf_free_render_rects(re->rects);
	re->rects = NULL;
	re->end = 1;
     }
   *cx = *x = ux;
   *cy = *y = uy;
   *cw = *w = uw;
   *ch = *h = uh;

   /* Return the "fake" surface so it is passed to the drawing routines. */
   return re->rgba_image;
}

void
evas_engine_directfb_output_redraws_next_update_push(void *data, void *surface,
						     int x, int y, int w, int h)
{
   Render_Engine      *re;
   DFBRectangle        rect;

   rect.x = x;
   rect.y = y;
   rect.w = w;
   rect.h = h;
   re = (Render_Engine *) data;
   re->surface->Blit(re->surface, re->backbuf, &rect, x, y);
   evas_common_cpu_end_opt();
}

void
evas_engine_directfb_output_flush(void *data)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
}

void        *
evas_engine_directfb_context_new(void *data)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   return evas_common_draw_context_new();
}

void
evas_engine_directfb_context_free(void *data, void *context)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_draw_context_free(context);
}

void
evas_engine_directfb_context_clip_set(void *data, void *context, int x, int y,
				      int w, int h)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_draw_context_set_clip(context, x, y, w, h);
}

void
evas_engine_directfb_context_clip_clip(void *data, void *context, int x, int y,
				       int w, int h)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_draw_context_clip_clip(context, x, y, w, h);
}

void
evas_engine_directfb_context_clip_unset(void *data, void *context)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_draw_context_unset_clip(context);
}

int
evas_engine_directfb_context_clip_get(void *data, void *context, int *x, int *y,
				      int *w, int *h)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   *x = ((RGBA_Draw_Context *) data)->clip.x;
   *y = ((RGBA_Draw_Context *) data)->clip.y;
   *w = ((RGBA_Draw_Context *) data)->clip.w;
   *h = ((RGBA_Draw_Context *) data)->clip.h;
   return ((RGBA_Draw_Context *) data)->clip.use;
}

void
evas_engine_directfb_context_color_set(void *data, void *context, int r, int g,
				       int b, int a)
{
   Render_Engine      *re;
   RGBA_Draw_Context  *dc = (RGBA_Draw_Context *) context;

   re = (Render_Engine *) data;
   evas_common_draw_context_set_color(dc, r, g, b, a);
}

int
evas_engine_directfb_context_color_get(void *data, void *context, int *r,
				       int *g, int *b, int *a)
{

   DATA32              p;
   DATA8              *pp;
   RGBA_Draw_Context  *dc = (RGBA_Draw_Context *) context;

   p = dc->col.col;
   pp = (DATA8 *) & p;
   *r = R_VAL(pp);
   *g = G_VAL(pp);
   *b = B_VAL(pp);
   *a = A_VAL(pp);
}

void
evas_engine_directfb_context_multiplier_set(void *data, void *context, int r,
					    int g, int b, int a)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_draw_context_set_multiplier(context, r, g, b, a);
}

void
evas_engine_directfb_context_multiplier_unset(void *data, void *context)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_draw_context_unset_multiplier(context);
}

int
evas_engine_directfb_context_multiplier_get(void *data, void *context, int *r,
					    int *g, int *b, int *a)
{
   DATA32              p;
   DATA8              *pp;
   RGBA_Draw_Context  *dc = (RGBA_Draw_Context *) context;

   p = dc->mul.col;
   pp = (DATA8 *) & p;
   *r = R_VAL(pp);
   *g = G_VAL(pp);
   *b = B_VAL(pp);
   *a = A_VAL(pp);

   return dc->mul.use;
}

void
evas_engine_directfb_context_cutout_add(void *data, void *context, int x, int y,
					int w, int h)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_draw_context_add_cutout(context, x, y, w, h);
}

void
evas_engine_directfb_context_cutout_clear(void *data, void *context)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_draw_context_clear_cutouts(context);
}

/* 
 * Rectangles
 *
 *
 *
 * */

void
evas_engine_directfb_draw_rectangle(void *data, void *context, void *surface,
				    int x, int y, int w, int h)
{
   int                 c, cx, cy, cw, ch;
   Cutout_Rect        *rects, *r;
   RGBA_Draw_Context  *dc = (RGBA_Draw_Context *) context;
   Evas_Object_List   *l;
   Render_Engine      *re = (Render_Engine *) data;

   /* handle cutouts here! */

   /* no cutouts - cut right to the chase */
   if (!dc->cutout.rects)
     {
	rectangle_draw_internal(data, dc, x, y, w, h);
	return;
     }
   /* save out clip info */
   c = dc->clip.use;
   cx = dc->clip.x;
   cy = dc->clip.y;
   cw = dc->clip.w;
   ch = dc->clip.h;
   evas_common_draw_context_clip_clip(dc, 0, 0, re->tb->outbuf_w, re->tb->outbuf_h);
   evas_common_draw_context_clip_clip(dc, x, y, w, h);
   /* our clip is 0 size.. abort */
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0))
     {
	dc->clip.use = c;
	dc->clip.x = cx;
	dc->clip.y = cy;
	dc->clip.w = cw;
	dc->clip.h = ch;
	return;
     }
   rects = evas_common_draw_context_apply_cutouts(dc);
   for (l = (Evas_Object_List *) rects; l; l = l->next)
     {
	r = (Cutout_Rect *) l;
	evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
	rectangle_draw_internal(data, dc, x, y, w, h);
     }
   evas_common_draw_context_apply_free_cutouts(rects);
   /* restore clip info */
   dc->clip.use = c;
   dc->clip.x = cx;
   dc->clip.y = cy;
   dc->clip.w = cw;
   dc->clip.h = ch;

}

void
rectangle_draw_internal(void *data, void *context, int x, int y, int w, int h)
{
   int                 yy;
   DATA32             *ptr;
   RGBA_Draw_Context  *dc = (RGBA_Draw_Context *) context;
   Render_Engine      *re = (Render_Engine *) data;
   int                 r, g, b, a;

   if ((w <= 0) || (h <= 0))
      return;

   if (!(RECTS_INTERSECT(x, y, w, h, 0, 0, re->tb->outbuf_w, re->tb->outbuf_h)))
      return;

   if (x < 0)
     {
	w += x;
	x = 0;
     }
   if ((x + w) > re->tb->outbuf_w)
      w = re->tb->outbuf_w - x;
   if (y < 0)
     {
	h += y;
	y = 0;
     }
   if ((w <= 0) || (h <= 0))
      return;

   if (dc->clip.use)
     {
	if (dc->clip.x > x)
	  {
	     w += x - dc->clip.x;
	     x = dc->clip.x;
	  }
	if ((dc->clip.x + dc->clip.w) < (x + w))
	   w = dc->clip.x + dc->clip.w - x;
	if (dc->clip.y > y)
	  {
	     h += y - dc->clip.y;
	     y = dc->clip.y;
	  }
	if ((dc->clip.y + dc->clip.h) < (y + h))
	   h = dc->clip.y + dc->clip.h - y;
     }
   if ((w <= 0) || (h <= 0))
      return;

   evas_engine_directfb_context_color_get(data, context, &r, &g, &b, &a);
   re->backbuf->SetColor(re->backbuf, r, g, b, a);
   re->backbuf->FillRectangle(re->backbuf, x, y, w, h);
   evas_common_cpu_end_opt();
}

/* 
 * Lines
 *
 *
 *
 * */

void
evas_engine_directfb_line_draw(void *data, void *context, void *surface, int x1,
			       int y1, int x2, int y2)
{
   int r, g, b, a;
   Render_Engine      *re = (Render_Engine *) data;

   evas_engine_directfb_context_color_get(data, context, &r, &g, &b, &a);

   re->backbuf->SetColor(re->backbuf, r, g, b, a);
   re->backbuf->DrawLine(re->backbuf, x1, y1, x2, y2);
}

/* 
 * Polygons
 *
 *
 *
 * */

void *
evas_engine_directfb_polygon_point_add(void *data, void *context, void *polygon, int x, int y)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   return evas_common_polygon_point_add(polygon, x, y);
   context = NULL;
}

void *
evas_engine_directfb_polygon_points_clear(void *data, void *context, void *polygon)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   return evas_common_polygon_points_clear(polygon);
   context = NULL;
}

void
evas_engine_directfb_polygon_draw(void *data, void *context, void *surface, void *polygon)
{

   Render_Engine *re;
   IDirectFBSurface *surf;
   RGBA_Image  *im;
   void *p;
   int pitch;
   
   re = (Render_Engine *)data;
   im = surface;
   surf = (IDirectFBSurface *)im->image->data;
   surf->Lock(surf, DSLF_WRITE, &p, & pitch);
   im->image->data = p;
   evas_common_polygon_draw(im, context, polygon);
   surf->Unlock(surf);
   im->image->data = (void *)surf;
   evas_common_cpu_end_opt();
}


/* 
 * Gradients
 *
 *
 *
 * */
void *
evas_engine_directfb_gradient_color_add(void *data, void *context, void *gradient, int r, int g, int b, int a, int distance)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   if (!gradient)
     gradient = evas_common_gradient_new();
   evas_common_gradient_color_add(gradient, r, g, b, a, distance);
   return gradient;
   context = NULL;
}

void *
evas_engine_directfb_gradient_colors_clear(void *data, void *context, void *gradient)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   if (gradient) evas_common_gradient_free(gradient);
   return NULL;
   context = NULL;
}

void
evas_engine_directfb_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h, double angle)
{
   Render_Engine *re;
   IDirectFBSurface *surf;
   RGBA_Image  *im;
   void *p;
   int pitch;
   
   re = (Render_Engine *)data;
   im = surface;
   surf = (IDirectFBSurface *)im->image->data;
   surf->Lock(surf, DSLF_WRITE, &p, & pitch);
   im->image->data = p;
   evas_common_gradient_draw(im, context, x, y, w, h, gradient, angle);
   surf->Unlock(surf);
   im->image->data = (void *)surf;
   evas_common_cpu_end_opt();
}



/* 
 * Font objects
 *
 *
 *
 * */

void        *
evas_engine_directfb_font_load(void *data, char *name, int size)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   return evas_common_font_load(name, size);
}

void
evas_engine_directfb_font_free(void *data, void *font)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_font_free(font);
}

int
evas_engine_directfb_font_ascent_get(void *data, void *font)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   return evas_common_font_ascent_get(font);
}

int
evas_engine_directfb_font_descent_get(void *data, void *font)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   return evas_common_font_descent_get(font);
}

int
evas_engine_directfb_font_max_ascent_get(void *data, void *font)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   return evas_common_font_max_ascent_get(font);
}

int
evas_engine_directfb_font_max_descent_get(void *data, void *font)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   return evas_common_font_max_descent_get(font);
}

void
evas_engine_directfb_font_string_size_get(void *data, void *font, char *text,
					  int *w, int *h)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_font_query_size(font, text, w, h);
}

int
evas_engine_directfb_font_inset_get(void *data, void *font, char *text)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   return evas_common_font_query_inset(font, text);
}

int
evas_engine_directfb_font_h_advance_get(void *data, void *font, char *text)
{
   Render_Engine      *re;
   int                 h, v;

   re = (Render_Engine *) data;
   evas_common_font_query_advance(font, text, &h, &v);
   return h;
}

int
evas_engine_directfb_font_v_advance_get(void *data, void *font, char *text)
{
   Render_Engine      *re;
   int                 h, v;

   re = (Render_Engine *) data;
   evas_common_font_query_advance(font, text, &h, &v);
   return v;
}

int
evas_engine_directfb_font_char_coords_get(void *data, void *font, char *text,
					  int pos, int *cx, int *cy, int *cw,
					  int *ch)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   return evas_common_font_query_char_coords(font, text, pos, cx, cy, cw, ch);
}

int
evas_engine_directfb_font_char_at_coords_get(void *data, void *font, char *text,
					     int x, int y, int *cx, int *cy,
					     int *cw, int *ch)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   return evas_common_font_query_text_at_pos(font, text, x, y, cx, cy, cw, ch);
}

void
evas_engine_directfb_font_draw(void *data, void *context, void *surface,
			       void *font, int x, int y, int w, int h, int ow,
			       int oh, char *text)
{
   Render_Engine *re;
   void *p;
   RGBA_Image *im;
   IDirectFBSurface * surf;
   int pitch;

   im = surface;
   re = (Render_Engine *)data;
   surf = (IDirectFBSurface *)im->image->data; 
   surf->Lock(surf, DSLF_WRITE, &p, &pitch);
   im->image->data = p;
   if ((w == ow) && (h == oh))
      evas_common_font_draw(im, context, font, x, y, text);
   else
     {
	/* create output surface size ow x oh and scale to w x h */
	RGBA_Draw_Context *dc, *dc_in;

	dc_in = context;
	dc = evas_common_draw_context_new();
	if (dc)
	  {
	     RGBA_Image *im;
	     
	     dc->col.col = dc_in->col.col;
	     im = evas_common_image_create(ow, oh);
	     if (im)
	       {
		  int max_ascent;
		  int i, j;
		  
		  im->flags |= RGBA_IMAGE_HAS_ALPHA;
		  j = ow * oh;
		  for (i = 0; i < j; i++) im->image->data[i] = (dc->col.col & 0xffffff);
		  
		  max_ascent = evas_common_font_max_ascent_get(font);
		  
		  evas_common_font_draw(im, dc, font, 0, max_ascent, text);
		  evas_common_cpu_end_opt();
		  evas_common_scale_rgba_in_to_out_clip_smooth(im, surface, context, 
						   0, 0, ow, oh, 
						   x, y - ((max_ascent * h) / oh), 
						   w, h);
		  evas_common_image_free(im);
	       }
	     evas_common_draw_context_free(dc);
	  }
     }
      
   surf->Unlock(surf);
   im->image->data = (void *)surf;
   evas_common_cpu_end_opt();
}

void
evas_engine_directfb_font_cache_flush(void *data)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_font_flush();
}

void
evas_engine_directfb_font_cache_set(void *data, int bytes)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   evas_common_font_cache_set(bytes);
}

int
evas_engine_directfb_font_cache_get(void *data)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   return evas_common_font_cache_get();
}

