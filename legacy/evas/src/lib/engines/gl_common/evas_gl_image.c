#include "evas_gl_private.h"

Evas_GL_Image *
evas_gl_common_image_load(Evas_GL_Context *gc, char *file, char *key)
{
   Evas_GL_Image *im;
   RGBA_Image *im_im;
   Evas_List *l;

   im_im = evas_common_load_image_from_file(file, key);
   if (!im_im) return NULL;

   for (l = gc->images; l; l = l->next)
     {
	im = l->data;
	if (im->im == im_im)
	  {
	     evas_common_image_unref(im_im);
	     gc->images = evas_list_remove_list(gc->images, l);
	     gc->images = evas_list_prepend(gc->images, im);
	     im->references++;
	     return im;
	  }
     }

   im = calloc(1, sizeof(Evas_GL_Image));
   if (!im) return NULL;
   im->references = 1;
   im->im = im_im;
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   im->gc = gc;
   im->references = 1;
   im->cached = 1;
   gc->images = evas_list_prepend(gc->images, im);
   return im;
}

Evas_GL_Image *
evas_gl_common_image_new_from_data(Evas_GL_Context *gc, int w, int h, int *data)
{
   Evas_GL_Image *im;
   Evas_List *l;

   for (l = gc->images; l; l = l->next)
     {
	im = l->data;
	if (((void *)(im->im->image->data) == (void *)data) &&
	    (im->im->image->w == w) &&
	    (im->im->image->h == h))
	  {
	     gc->images = evas_list_remove_list(gc->images, l);
	     gc->images = evas_list_prepend(gc->images, im);
	     im->references++;
	     return im;
	  }
     }
   im = calloc(1, sizeof(Evas_GL_Image));
   im->references = 1;
   im->im = evas_common_image_new();
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   im->im->image = evas_common_image_surface_new(im->im);
   if (!im->im->image)
     {
	evas_common_image_free(im->im);
	free(im);
	return NULL;
     }
   im->im->image->w = w;
   im->im->image->h = h;
   im->im->image->data = data;
   im->im->image->no_free = 1;
   im->gc = gc;
/*
   im->cached = 1;
   gc->images = evas_list_prepend(gc->images, im);
 */
   return im;
}

Evas_GL_Image *
evas_gl_common_image_new_from_copied_data(Evas_GL_Context *gc, int w, int h, int *data)
{
   Evas_GL_Image *im;

   im = calloc(1, sizeof(Evas_GL_Image));
   im->references = 1;
   im->im = evas_common_image_create(w, h);
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   if (data)
     memcpy(im->im->image->data, data, w * h * sizeof(DATA32));
   im->gc = gc;
   return im;
}

Evas_GL_Image *
evas_gl_common_image_new(Evas_GL_Context *gc, int w, int h)
{
   Evas_GL_Image *im;

   im = calloc(1, sizeof(Evas_GL_Image));
   im->references = 1;
   im->im = evas_common_image_create(w, h);
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   return im;
}

void
evas_gl_common_image_free(Evas_GL_Image *im)
{
   im->references--;
   if (im->references > 0) return;

   if (im->cached)
     im->gc->images = evas_list_remove(im->gc->images, im);
   if (im->im) evas_common_image_unref(im->im);
   if (im->tex) evas_gl_common_texture_free(im->tex);
   free(im);
}

void
evas_gl_common_image_dirty(Evas_GL_Image *im)
{
   evas_common_image_dirty(im->im);
   im->dirty = 1;
}

void
evas_gl_common_image_draw(Evas_GL_Context *gc, RGBA_Draw_Context *dc, Evas_GL_Image *im, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int smooth)
{
   int r, g, b, a;
   double tx1, ty1, tx2, ty2;
   int    ow, oh;

   if (sw < 1) sw = 1;
   if (sh < 1) sh = 1;
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
   evas_common_load_image_data_from_file(im->im);
   if ((im->tex) && (im->dirty))
     {
	evas_gl_common_texture_update(im->tex, im->im, im->tex->smooth);
	im->dirty = 0;
     }
   if (!im->tex)
     im->tex = evas_gl_common_texture_new(gc, im->im, smooth);
   ow = (dw * im->tex->tw) / sw;
   oh = (dh * im->tex->th) / sh;
   evas_gl_common_context_texture_set(gc, im->tex, smooth, ow, oh);
   if ((!im->tex->have_mipmaps) && (smooth) &&
       ((im->tex->uw < im->tex->tw) || (im->tex->uh < im->tex->th)))
     evas_gl_common_texture_mipmaps_build(im->tex, im->im, smooth);

   if (im->tex->not_power_of_two)
     {
	tx1 = sx;
	ty1 = sy;
	tx2 = sx + sw;
	ty2 = sy + sh;
     }
   else
     {
	tx1 = (double)(sx     ) / (double)(im->tex->w);
	ty1 = (double)(sy     ) / (double)(im->tex->h);
	tx2 = (double)(sx + sw) / (double)(im->tex->w);
	ty2 = (double)(sy + sh) / (double)(im->tex->h);
     }
   evas_gl_common_context_color_set(gc, r, g, b, a);
   if ((a < 255) || (im->im->flags & RGBA_IMAGE_HAS_ALPHA))
     evas_gl_common_context_blend_set(gc, 1);
   else evas_gl_common_context_blend_set(gc, 0);
   if (dc->clip.use)
     evas_gl_common_context_clip_set(gc, 1,
				     dc->clip.x, dc->clip.y,
				     dc->clip.w, dc->clip.h);
   else
     evas_gl_common_context_clip_set(gc, 0,
				     0, 0, 0, 0);
   evas_gl_common_context_read_buf_set(gc, GL_BACK);
   evas_gl_common_context_write_buf_set(gc, GL_BACK);

   glBegin(GL_QUADS);
   glTexCoord2d(tx1, ty1); glVertex2i(dx     , dy     );
   glTexCoord2d(tx2, ty1); glVertex2i(dx + dw, dy     );
   glTexCoord2d(tx2, ty2); glVertex2i(dx + dw, dy + dh);
   glTexCoord2d(tx1, ty2); glVertex2i(dx     , dy + dh);
   glEnd();
}
