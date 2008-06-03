#include "evas_gl_private.h"

Evas_GL_Image *
evas_gl_common_image_load(Evas_GL_Context *gc, const char *file, const char *key, Evas_Image_Load_Opts *lo)
{
   Evas_GL_Image        *im;
   RGBA_Image           *im_im;
   Evas_List            *l;

   im_im = evas_common_load_image_from_file(file, key, lo);
   if (!im_im) return NULL;

   for (l = gc->images; l; l = l->next)
     {
	im = l->data;
	if (im->im == im_im)
	  {
             evas_cache_image_drop(&im_im->cache_entry);
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
   im->cs.space = EVAS_COLORSPACE_ARGB8888;
   if (lo) im->load_opts = *lo;
   gc->images = evas_list_prepend(gc->images, im);
   return im;
}

Evas_GL_Image *
evas_gl_common_image_new_from_data(Evas_GL_Context *gc, int w, int h, DATA32 *data, int alpha, int cspace)
{
   Evas_GL_Image *im;
   Evas_List *l;

   for (l = gc->images; l; l = l->next)
     {
	im = l->data;
	if (((void *)(im->im->image.data) == (void *)data) &&
	    (im->im->cache_entry.w == w) &&
	    (im->im->cache_entry.h == h))
	  {
	     gc->images = evas_list_remove_list(gc->images, l);
	     gc->images = evas_list_prepend(gc->images, im);
	     im->references++;
	     return im;
	  }
     }
   im = calloc(1, sizeof(Evas_GL_Image));
   if (!im) return NULL;
   im->references = 1;
   im->im = (RGBA_Image *) evas_cache_image_data(evas_common_image_cache_get(),
                                                 w, h, data, alpha, cspace);
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   im->gc = gc;
   im->cs.space = cspace;
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
        break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
        if (im->tex) evas_gl_common_texture_free(im->tex);
	im->tex = NULL;
	im->cs.data = data;
	im->cs.no_free = 1;
	break;
      default:
	abort();
	break;
     }
   /*
    im->cached = 1;
    gc->images = evas_list_prepend(gc->images, im);
    */
   printf("new im cs = %i\n", im->cs.space);
   return im;
}

Evas_GL_Image *
evas_gl_common_image_new_from_copied_data(Evas_GL_Context *gc, int w, int h, DATA32 *data, int alpha, int cspace)
{
   Evas_GL_Image *im;

   im = calloc(1, sizeof(Evas_GL_Image));
   if (!im) return NULL;
   im->references = 1;
   im->im = (RGBA_Image *) evas_cache_image_copied_data(evas_common_image_cache_get(),
                                                        w, h, data, alpha, cspace);
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   im->gc = gc;
   im->cs.space = cspace;
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
        if (im->tex) evas_gl_common_texture_free(im->tex);
	im->tex = NULL;
	im->cs.no_free = 0;
        im->cs.data = calloc(1, im->im->cache_entry.h * sizeof(unsigned char *) * 2);
        if ((data) && (im->cs.data))
	  memcpy(im->cs.data, data, im->im->cache_entry.h * sizeof(unsigned char *) * 2);
	break;
      default:
	abort();
	break;
     }
   return im;
}

Evas_GL_Image *
evas_gl_common_image_new(Evas_GL_Context *gc, int w, int h, int alpha, int cspace)
{
   Evas_GL_Image *im;

   im = calloc(1, sizeof(Evas_GL_Image));
   if (!im) return NULL;
   im->references = 1;
   im->im = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   im->gc = gc;
   im->cs.space = cspace;
   im->im->cache_entry.flags.alpha = alpha ? 1 : 0;
   evas_cache_image_colorspace(&im->im->cache_entry, cspace);
   im->im = (RGBA_Image *) evas_cache_image_size_set(&im->im->cache_entry, w, h);
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
//        if (im->tex) evas_gl_common_texture_free(im->tex);
	im->tex = NULL;
	im->cs.no_free = 0;
        im->cs.data = calloc(1, im->im->cache_entry.h * sizeof(unsigned char *) * 2);
	break;
      default:
	abort();
	break;
     }
   return im;
}

void
evas_gl_common_image_free(Evas_GL_Image *im)
{
   im->references--;
   if (im->references > 0) return;

   if (im->cs.data)
     {
	if (!im->cs.no_free) free(im->cs.data);
     }
   if (im->cached) im->gc->images = evas_list_remove(im->gc->images, im);
   if (im->im) evas_cache_image_drop(&im->im->cache_entry);
   if (im->tex) evas_gl_common_texture_free(im->tex);
   free(im);
}

void
evas_gl_common_image_dirty(Evas_GL_Image *im)
{
   im->im = (RGBA_Image *) evas_cache_image_dirty(&im->im->cache_entry, 0, 0, im->im->cache_entry.w, im->im->cache_entry.h);
   im->dirty = 1;
}

void
evas_gl_common_image_draw(Evas_GL_Context *gc, Evas_GL_Image *im, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int smooth)
{
   RGBA_Draw_Context *dc;
   int r, g, b, a;
   double tx1, ty1, tx2, ty2;
   int    ow, oh;
   int    space;

   if (sw < 1) sw = 1;
   if (sh < 1) sh = 1;
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
   
   if (!gc->ext.arb_program && (im->cs.space == EVAS_COLORSPACE_YCBCR422P601_PL
      || im->cs.space == EVAS_COLORSPACE_YCBCR422P709_PL))
     {
        if ((im->cs.data) && (*((unsigned char **)im->cs.data)))
          {
             if (im->dirty || !im->im->image.data)
               {
                  free(im->im->image.data);
                  im->im->image.data = malloc(im->im->cache_entry.w * im->im->cache_entry.h * sizeof(DATA32));
                  if (im->im->image.data)
                    evas_common_convert_yuv_420p_601_rgba(im->cs.data, 
                                                          (void *)im->im->image.data,
                                                          im->im->cache_entry.w, im->im->cache_entry.h);
               }
          }
        space = EVAS_COLORSPACE_ARGB8888;
     }
   else
     space = im->cs.space;
   
/* leak in this switch */
   switch (space)
     {
      case EVAS_COLORSPACE_ARGB8888:
        evas_cache_image_load_data(&im->im->cache_entry);
	if ((im->tex) && (im->dirty))
	  {
	     evas_gl_common_texture_update(im->tex, im->im, im->tex->smooth);
	     im->dirty = 0;
	  }
	if (!im->tex)
	  im->tex = evas_gl_common_texture_new(gc, im->im, smooth);
	ow = (dw * im->tex->tw) / sw;
	oh = (dh * im->tex->th) / sh;
	if (im->tex->rectangle)
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
	evas_gl_common_context_texture_set(gc, im->tex, smooth, ow, oh);
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	if ((im->tex) && (im->dirty))
	  {
	     evas_gl_common_ycbcr601pl_texture_update(im->tex, im->cs.data, im->im->cache_entry.w, im->im->cache_entry.h, smooth);
	     im->dirty = 0;
	  }
	if ((!im->tex) && (im->cs.data) && (*((unsigned char **)im->cs.data)))
	  {
	     im->tex = evas_gl_common_ycbcr601pl_texture_new(gc, im->cs.data, im->im->cache_entry.w, im->im->cache_entry.h, smooth);
	  }
	if (!im->tex) return;
	ow = (dw * im->tex->tw) / sw;
	oh = (dh * im->tex->th) / sh;
	if (im->tex->rectangle)
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
	evas_gl_common_context_texture_set(gc, im->tex, smooth, ow, oh);

	break;
      default:
	abort();
	break;
    }

//   if ((!im->tex->have_mipmaps) && (smooth) &&
//       ((im->tex->uw < im->tex->tw) || (im->tex->uh < im->tex->th)) &&
//       (!gc->ext.sgis_generate_mipmap))
//     evas_gl_common_texture_mipmaps_build(im->tex, im->im, smooth);

   evas_gl_common_context_color_set(gc, r, g, b, a);
   if ((a < 255) || im->im->cache_entry.flags.alpha)
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
