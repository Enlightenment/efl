#include "evas_gl_private.h"

static void _evas_gl_common_gradient_texture_build(Evas_GL_Context *gc, Evas_GL_Gradient *gr);

Evas_GL_Gradient *
evas_gl_common_gradient_color_add(Evas_GL_Gradient *gr, int r, int g, int b, int a, int distance)
{
   if (!gr) gr = calloc(1, sizeof(Evas_GL_Gradient));
   if (!gr) return NULL;
   if (!gr->grad) gr->grad = evas_common_gradient_new();
   evas_common_gradient_color_add(gr->grad, r, g, b, a, distance);
   if (gr->tex) evas_gl_common_texture_free(gr->tex);
   gr->tex = NULL;
   return gr;
}

Evas_GL_Gradient *
evas_gl_common_gradient_colors_clear(Evas_GL_Gradient *gr)
{
   if (!gr) return NULL;
   evas_common_gradient_colors_clear(gr->grad);
   return gr;
}

Evas_GL_Gradient *
evas_gl_common_gradient_data_set(Evas_GL_Gradient *gr, void * map, int len, int has_alpha)
{
   if (!gr) gr = calloc(1, sizeof(Evas_GL_Gradient));
   if (!gr) return NULL;
   if (!gr->grad) gr->grad = evas_common_gradient_new();
   evas_common_gradient_data_set(gr->grad, map, len, has_alpha);
   return gr;
}

Evas_GL_Gradient *
evas_gl_common_gradient_data_unset(Evas_GL_Gradient *gr, void * data, int len, int has_alpha)
{
   if (!gr) return NULL;
   evas_common_gradient_data_unset(gr->grad);
   return gr;
}

void
evas_gl_common_gradient_free(Evas_GL_Gradient *gr)
{
   if (!gr) return;
   if (gr->grad) evas_common_gradient_free(gr->grad);
   if (gr->tex) evas_gl_common_texture_free(gr->tex);
   gr->tex = NULL;
   free(gr);
}

void
evas_gl_common_gradient_fill_set(Evas_GL_Gradient *gr, int x, int y, int w, int h)
{
   if (!gr) return;
   evas_common_gradient_fill_set(gr->grad, x, y, w, h);
}

void
evas_gl_common_gradient_range_offset_set(Evas_GL_Gradient *gr, float offset)
{
   if (!gr) return;
   evas_common_gradient_range_offset_set(gr->grad, offset);
}

void
evas_gl_common_gradient_type_set(Evas_GL_Gradient *gr, char *name)
{
   if (!gr) return;
   evas_common_gradient_type_set(gr->grad, name);
}

void
evas_gl_common_gradient_type_params_set(Evas_GL_Gradient *gr, char *params)
{
   if (!gr) return;
   evas_common_gradient_type_params_set(gr->grad, params);
}

void *
evas_gl_common_gradient_geometry_init(Evas_GL_Gradient *gr, int spread)
{
   if (!gr) return NULL;
   gr->grad = evas_common_gradient_geometry_init(gr->grad, spread);
   return gr;
}

int
evas_gl_common_gradient_alpha_get(Evas_GL_Gradient *gr, int spread, int op)
{
   if (!gr) return 0;
   return evas_common_gradient_has_alpha(gr->grad, spread, op);
}

void
evas_gl_common_gradient_map(RGBA_Draw_Context *dc, Evas_GL_Gradient *gr, int spread)
{
   int  mul_use;
   
   if (!gr || !dc) return;
   mul_use = dc->mul.use;
   dc->mul.use = 0;
   evas_common_gradient_map(dc, gr->grad, spread);
   dc->mul.use = mul_use;
   evas_common_cpu_end_opt();
}
void
evas_gl_common_gradient_draw(Evas_GL_Context *gc, RGBA_Draw_Context *dc, Evas_GL_Gradient *gr, int x, int y, int w, int h, double angle, int spread)
{
   int r, g, b, a;

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
   evas_gl_common_context_blend_set(gc, 1);
   if (dc->clip.use)
     evas_gl_common_context_clip_set(gc, 1,
				     dc->clip.x, dc->clip.y,
				     dc->clip.w, dc->clip.h);
   else
     evas_gl_common_context_clip_set(gc, 0,
				     0, 0, 0, 0);
   if (!gr->tex)
     _evas_gl_common_gradient_texture_build(gc, gr);
   evas_gl_common_context_texture_set(gc, gr->tex, 0, 255, 3);

   evas_gl_common_context_read_buf_set(gc, GL_BACK);
   evas_gl_common_context_write_buf_set(gc, GL_BACK);
     {
	double max, t[8];
	int tw, th, i;

	tw = 256;
	th = 4;

	t[0] = cos(((-angle + 45 + 90) * 2 * 3.141592654) / 360);
	t[1] = sin(((-angle + 45 + 90) * 2 * 3.141592654) / 360);

	t[2] = cos(((-angle + 45 + 180) * 2 * 3.141592654) / 360);
	t[3] = sin(((-angle + 45 + 180) * 2 * 3.141592654) / 360);

	t[4] = cos(((-angle + 45 + 270) * 2 * 3.141592654) / 360);
	t[5] = sin(((-angle + 45 + 270) * 2 * 3.141592654) / 360);

	t[6] = cos(((-angle + 45 + 0) * 2 * 3.141592654) / 360);
	t[7] = sin(((-angle + 45 + 0) * 2 * 3.141592654) / 360);
	max = 0;

	for (i = 0; i < 8; i++)
	  {
	     if ((t[i] < 0) && (-t[i] > max)) max = -t[i];
	     else if ((t[i] > max)) max = t[i];
	  }
	if (max > 0)
	  {
	     for (i = 0; i < 8; i++) t[i] *= 1 / max;
	  }
	for (i = 0; i < 8; i+=2)
	  {
	     t[i] = (1.0 + ((((0.5) + (t[i] / 2.0)) * (256.0 - 2.0)))) / 256.0;
	     t[i + 1] = (1.0 + ((((0.5) - (t[i + 1] / 2.0))) * 2.0)) / 4.0;
	     if (gc->ext.nv_texture_rectangle)
	       {
		  t[i] *= 256.0;
		  t[i + 1] *= 4.0;
	       }
	  }
	glBegin(GL_QUADS);
	glTexCoord2d(t[0],  t[1]); glVertex2i(x, y);
	glTexCoord2d(t[2],  t[3]); glVertex2i(x + w, y);
	glTexCoord2d(t[4],  t[5]); glVertex2i(x + w, y + h);
	glTexCoord2d(t[6],  t[7]); glVertex2i(x, y + h);
	glEnd();
     }
}

static void
_evas_gl_common_gradient_texture_build(Evas_GL_Context *gc, Evas_GL_Gradient *gr)
{
   DATA32 *map;
   RGBA_Draw_Context *dc;
   RGBA_Image *im;

   return;
/*
FIXME: this has been broken by the new gradient code!!
*/
   dc = evas_common_draw_context_new();
   if (!dc) return;
   evas_common_gradient_map(dc, gr->grad, 0);
   if (map = gr->grad->map.data)
     {
	im = evas_common_image_create(256, 4);
	if (im)
	  {
	     int i;

	     for (i = 0; i < 4; i++)
		 evas_common_scale_rgba_span(map, gr->grad.map.len, 0xffffffff,
		                             im->image->data + (i * 256), 256);
	     im->flags |= RGBA_IMAGE_HAS_ALPHA;
	     gr->tex = evas_gl_common_texture_new(gc, im, 0);
	     evas_common_image_free(im);
	  }
     }
   evas_common_draw_context_free(dc);
}
