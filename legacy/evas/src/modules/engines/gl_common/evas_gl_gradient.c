#include "evas_gl_private.h"

Evas_GL_Gradient *
evas_gl_common_gradient_new(void)
{
   Evas_GL_Gradient *gr;

   gr = calloc(1, sizeof(Evas_GL_Gradient));
   if (!gr) return NULL;
   gr->grad = evas_common_gradient_new();
   if (!gr->grad) { free(gr);  return NULL; }
   gr->changed = 1;
   return gr;
}

void
evas_gl_common_gradient_free(Evas_GL_Gradient *gr)
{
   if (!gr) return;
   if (gr->grad) evas_common_gradient_free(gr->grad);
   if (gr->tex) evas_gl_common_texture_free(gr->tex);
   free(gr);
}

void
evas_gl_common_gradient_color_stop_add(Evas_GL_Gradient *gr, int r, int g, int b, int a, int delta)
{
   if (!gr) return;
   evas_common_gradient_color_stop_add(gr->grad, r, g, b, a, delta);
   gr->changed = 1;
}

void
evas_gl_common_gradient_alpha_stop_add(Evas_GL_Gradient *gr, int a, int delta)
{
   if (!gr) return;
   evas_common_gradient_alpha_stop_add(gr->grad, a, delta);
   gr->changed = 1;
}

void
evas_gl_common_gradient_color_data_set(Evas_GL_Gradient *gr, void * map, int len, int has_alpha)
{
   if (!gr) return;
   evas_common_gradient_color_data_set(gr->grad, map, len, has_alpha);
   gr->changed = 1;
}

void
evas_gl_common_gradient_alpha_data_set(Evas_GL_Gradient *gr, void * alpha_map, int len)
{
   if (!gr) return;
   evas_common_gradient_alpha_data_set(gr->grad, alpha_map, len);
   gr->changed = 1;
}

void
evas_gl_common_gradient_clear(Evas_GL_Gradient *gr)
{
   if (!gr) return;
   evas_common_gradient_clear(gr->grad);
   gr->changed = 1;
}

void
evas_gl_common_gradient_fill_set(Evas_GL_Gradient *gr, int x, int y, int w, int h)
{
   if (!gr) return;
   evas_common_gradient_fill_set(gr->grad, x, y, w, h);
   gr->changed = 1;
}

void
evas_gl_common_gradient_fill_angle_set(Evas_GL_Gradient *gr, double angle)
{
   if (!gr) return;
   evas_common_gradient_fill_angle_set(gr->grad, angle);
   gr->changed = 1;
}

void
evas_gl_common_gradient_fill_spread_set(Evas_GL_Gradient *gr, int spread)
{
   if (!gr) return;
   evas_common_gradient_fill_spread_set(gr->grad, spread);
   gr->changed = 1;
}

void
evas_gl_common_gradient_map_angle_set(Evas_GL_Gradient *gr, double angle)
{
   if (!gr) return;
   evas_common_gradient_map_angle_set(gr->grad, angle);
   gr->changed = 1;
}

void
evas_gl_common_gradient_map_offset_set(Evas_GL_Gradient *gr, float offset)
{
   if (!gr) return;
   evas_common_gradient_map_offset_set(gr->grad, offset);
   gr->changed = 1;
}

void
evas_gl_common_gradient_map_direction_set(Evas_GL_Gradient *gr, int direction)
{
   if (!gr) return;
   evas_common_gradient_map_direction_set(gr->grad, direction);
   gr->changed = 1;
}

void
evas_gl_common_gradient_type_set(Evas_GL_Gradient *gr, char *name, char *params)
{
   if (!gr) return;
   evas_common_gradient_type_set(gr->grad, name, params);
   gr->changed = 1;
}

int
evas_gl_common_gradient_is_opaque(Evas_GL_Context *gc, Evas_GL_Gradient *gr,
                                  int x, int y, int w, int h)
{
   RGBA_Draw_Context *dc;

   if (!gc || !gc->dc || !gr || !gr->grad || !gr->grad->type.geometer)  return 0;
   dc = gc->dc;
   return !(gr->grad->type.geometer->has_alpha(gr->grad, dc->render_op) |
              gr->grad->type.geometer->has_mask(gr->grad, dc->render_op));
}

int
evas_gl_common_gradient_is_visible(Evas_GL_Context *gc, Evas_GL_Gradient *gr,
                                   int x, int y, int w, int h)
{
   if (!gc || !gr)  return 0;
   return 1;
}

void
evas_gl_common_gradient_render_pre(Evas_GL_Context *gc, Evas_GL_Gradient *gr)
{
   int  len;
   RGBA_Draw_Context *dc;
   RGBA_Gradient     *grad;

   if (!gr || !gc || !gr->grad || !gc->dc) return;
   grad = gr->grad;
   dc = gc->dc;
   if (!grad->type.geometer) return;
   grad->type.geometer->geom_set(grad);
   len = grad->type.geometer->get_map_len(grad);
   evas_common_gradient_map(dc, grad, len);
   gr->changed = 1;
}

void
evas_gl_common_gradient_render_post(Evas_GL_Gradient *gr)
{
}

void
evas_gl_common_gradient_draw(Evas_GL_Context *gc,
			     Evas_GL_Gradient *gr,
			     int x, int y, int w, int h)
{
   int r, g, b, a;
   RGBA_Draw_Context *dc;
   double  tx2, ty2;
   
   if ((w < 1) || (h < 1)) return;
   if (!gr || !gc || !gc->dc) return;
   if (!gr->grad || !gr->grad->type.geometer) return;
   
   dc = gc->dc;
   if (dc->mul.use)
     {
	a = (dc->mul.col >> 24) & 0xff;
	r = (dc->mul.col >> 16) & 0xff;
	g = (dc->mul.col >> 8 ) & 0xff;
	b = (dc->mul.col      ) & 0xff;
     }
   else
     {
	r = g = b = a = 255;
     }
   evas_gl_common_context_color_set(gc, r, g, b, a);
   
   a = !evas_gl_common_gradient_is_opaque(gc, gr, x, y, w, h);
   evas_gl_common_context_blend_set(gc, a);
   
   if (dc->clip.use)
     evas_gl_common_context_clip_set(gc, 1,
				     dc->clip.x, dc->clip.y,
				     dc->clip.w, dc->clip.h);
   else
     evas_gl_common_context_clip_set(gc, 0,
				     0, 0, 0, 0);
   
   if (!gr->tex) gr->changed = 1;
   if (((w != gr->tw) || (h != gr->th)) && gr->tex)
     {
	evas_gl_common_texture_free(gr->tex);
	gr->tex = NULL;
	gr->changed = 1;
     }
   if (gr->changed)
     {
	RGBA_Image *im;
	int op = dc->render_op, cuse = dc->clip.use;

	im = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
        if (!im) return;
        im = (RGBA_Image *) evas_cache_image_size_set(&im->cache_entry, w, h);
	dc->render_op = _EVAS_RENDER_FILL;
	dc->clip.use = 0;
	evas_common_gradient_draw(im, dc, 0, 0, w, h, gr->grad);
	if (!gr->tex)
	  gr->tex = evas_gl_common_texture_new(gc, im, 0);
	else
	  evas_gl_common_texture_update(gr->tex, im, 0);

	evas_cache_image_drop(&im->cache_entry);

	dc->render_op = op;
	dc->clip.use = cuse;
	if (!gr->tex) return;
     }
   
   if (gr->tex->rectangle)
     {
	tx2 = w;
	ty2 = h;
     }
   else
     {
	tx2 = (double)w / (double)(gr->tex->w);
	ty2 = (double)h / (double)(gr->tex->h);
     }
   
   evas_gl_common_context_texture_set(gc, gr->tex, 0,
				      gr->tex->tw, gr->tex->th);
   
   evas_gl_common_context_read_buf_set(gc, GL_BACK);
   evas_gl_common_context_write_buf_set(gc, GL_BACK);
   
   glBegin(GL_QUADS);
   glTexCoord2d(0.0,  0.0); glVertex2i(x, y);
   glTexCoord2d(tx2,  0.0); glVertex2i(x + w, y);
   glTexCoord2d(tx2,  ty2); glVertex2i(x + w, y + h);
   glTexCoord2d(0.0,  ty2); glVertex2i(x, y + h);
   glEnd();
   gr->changed = 0;
   gr->tw = w;  gr->th = h;
}
