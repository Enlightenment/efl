#include "evas_gl_private.h"

Evas_GL_Image *
evas_gl_common_image_load(Evas_GL_Context *gc, const char *file, const char *key, Evas_Image_Load_Opts *lo, int *error)
{
   Evas_GL_Image        *im;
   RGBA_Image           *im_im;
   Eina_List            *l;

   im_im = evas_common_load_image_from_file(file, key, lo, error);
   if (!im_im) return NULL;

   EINA_LIST_FOREACH(gc->shared->images, l, im)
     {
	if (im->im == im_im)
	  {
             evas_cache_image_drop(&im_im->cache_entry);
	     gc->shared->images = eina_list_remove_list(gc->shared->images, l);
	     gc->shared->images = eina_list_prepend(gc->shared->images, im);
	     im->references++;
	     *error = EVAS_LOAD_ERROR_NONE;
	     return im;
	  }
     }

   im = calloc(1, sizeof(Evas_GL_Image));
   if (!im)
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	return NULL;
     }
   im->references = 1;
   im->im = im_im;
   im->gc = gc;
   im->references = 1;
   im->cached = 1;
   im->cs.space = EVAS_COLORSPACE_ARGB8888;
   im->alpha = im->im->cache_entry.flags.alpha;
   im->w = im->im->cache_entry.w;
   im->h = im->im->cache_entry.h;
   if (lo) im->load_opts = *lo;
   gc->shared->images = eina_list_prepend(gc->shared->images, im);
   return im;
}

Evas_GL_Image *
evas_gl_common_image_new_from_data(Evas_GL_Context *gc, int w, int h, DATA32 *data, int alpha, int cspace)
{
   Evas_GL_Image *im;
   Eina_List *l;

   EINA_LIST_FOREACH(gc->shared->images, l, im)
     {
	if (((void *)(im->im->image.data) == (void *)data) &&
	    (im->im->cache_entry.w == w) &&
	    (im->im->cache_entry.h == h))
	  {
	     gc->shared->images = eina_list_remove_list(gc->shared->images, l);
	     gc->shared->images = eina_list_prepend(gc->shared->images, im);
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
   im->alpha = im->im->cache_entry.flags.alpha;
   im->w = im->im->cache_entry.w;
   im->h = im->im->cache_entry.h;
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
   im->alpha = im->im->cache_entry.flags.alpha;
   im->w = im->im->cache_entry.w;
   im->h = im->im->cache_entry.h;
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
   im->im->cache_entry.flags.alpha = alpha ? 1 : 0;
   im->cs.space = cspace;
   im->alpha = im->im->cache_entry.flags.alpha;
   im->w = im->im->cache_entry.w;
   im->h = im->im->cache_entry.h;
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
evas_gl_common_image_native_enable(Evas_GL_Image *im)
{
   if (im->cs.data)
     {
	if (!im->cs.no_free) free(im->cs.data);
        im->cs.data = NULL;
     }
   im->cs.no_free = 0;
   if (im->cached)
     {
        im->gc->shared->images = eina_list_remove(im->gc->shared->images, im);
        im->cached = 0;
     }
   if (im->im)
     {
        evas_cache_image_drop(&im->im->cache_entry);
        im->im = NULL;
     }
   if (im->tex)
     {
        evas_gl_common_texture_free(im->tex);
        im->tex = NULL;
     }
   
   im->cs.space = EVAS_COLORSPACE_ARGB8888;
   im->tex = evas_gl_common_texture_native_new(im->gc, im->w, im->h, im->alpha, im);
   im->tex_only = 1;
}

void
evas_gl_common_image_native_disable(Evas_GL_Image *im)
{
   if (im->im)
     {
        evas_cache_image_drop(&im->im->cache_entry);
        im->im = NULL;
     }
   if (im->tex)
     {
        evas_gl_common_texture_free(im->tex);
        im->tex = NULL;
     }
   im->tex_only = 0;
   
   im->im = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
   im->im->cache_entry.flags.alpha = im->alpha;
   im->cs.space = EVAS_COLORSPACE_ARGB8888;
   evas_cache_image_colorspace(&im->im->cache_entry, im->cs.space);
   im->im = (RGBA_Image *)evas_cache_image_size_set(&im->im->cache_entry, im->w, im->h);
}

void
evas_gl_common_image_free(Evas_GL_Image *im)
{
   im->references--;
   if (im->references > 0) return;
   
   if (im->native.func.free)
     im->native.func.free(im->native.func.data, im);
   
   if (im->cs.data)
     {
	if (!im->cs.no_free) free(im->cs.data);
     }
   if (im->cached) im->gc->shared->images = eina_list_remove(im->gc->shared->images, im);
   if (im->im) evas_cache_image_drop(&im->im->cache_entry);
   if (im->tex) evas_gl_common_texture_free(im->tex);
   free(im);
}

Evas_GL_Image *
evas_gl_common_image_surface_new(Evas_GL_Context *gc, int w, int h, int alpha)
{
   Evas_GL_Image *im;

   im = calloc(1, sizeof(Evas_GL_Image));
   if (!im) return NULL;
   im->references = 1;
   im->gc = gc;
   im->cs.space = EVAS_COLORSPACE_ARGB8888;
   im->alpha = alpha;
   im->w = w;
   im->h = h;
   im->tex = evas_gl_common_texture_render_new(gc, w, h, alpha);
   im->tex_only = 1;
   return im;
}

void
evas_gl_common_image_dirty(Evas_GL_Image *im, int x, int y, int w, int h)
{
   if ((w == 0) && (h == 0) && (x == 0) && (y == 0))
     {
        w = im->w;
        h = im->h;
     }
   if (im->im)
     {
        im->im = (RGBA_Image *)evas_cache_image_dirty(&im->im->cache_entry, x, y, w, h);
     }
   im->dirty = 1;
}

static void
_evas_gl_common_image_update(Evas_GL_Context *gc, Evas_GL_Image *im)
{
   if (!im->im) return;
/*   
   if ((im->cs.space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->cs.space == EVAS_COLORSPACE_YCBCR422P709_PL))
     {
        // SOFTWARE convert. do multi texture later
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
 */
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
        evas_cache_image_load_data(&im->im->cache_entry);
	if ((im->tex) && (im->dirty))
	  {
	     evas_gl_common_texture_update(im->tex, im->im);
	  }
	if (!im->tex)
	  im->tex = evas_gl_common_texture_new(gc, im->im);
        im->dirty = 0;
        if (!im->tex) return;
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
        if ((im->tex) && (im->dirty))
          {
             evas_gl_common_texture_yuv_update(im->tex, im->cs.data,
                                               im->im->cache_entry.w, 
                                               im->im->cache_entry.h);
             im->dirty = 0;
          }
        if ((!im->tex) && (im->cs.data) && (*((unsigned char **)im->cs.data)))
          {
             im->tex = evas_gl_common_texture_yuv_new(gc, im->cs.data,
                                                      im->im->cache_entry.w, 
                                                      im->im->cache_entry.h);
             im->dirty = 0;
          }
        if (!im->tex) return;
        break;
      default:
        printf("unhandled img format\n");
	break;
    }
}

void
evas_gl_common_image_map4_draw(Evas_GL_Context *gc, Evas_GL_Image *im, 
                               RGBA_Map_Point *p, int smooth, int level)
{
   RGBA_Draw_Context *dc;
   int r, g, b, a;
   int c, cx, cy, cw, ch;
   
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
   
   _evas_gl_common_image_update(gc, im);

   c = gc->dc->clip.use; 
   cx = gc->dc->clip.x; cy = gc->dc->clip.y; 
   cw = gc->dc->clip.w; ch = gc->dc->clip.h;
   im->tex->im = im;
   evas_gl_common_context_image_map4_push(gc, im->tex, p, 
                                          c, cx, cy, cw, ch, 
                                          r, g, b, a, smooth, im->tex_only);
}

void
evas_gl_common_image_draw(Evas_GL_Context *gc, Evas_GL_Image *im, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int smooth)
{
   RGBA_Draw_Context *dc;
   int r, g, b, a;
   double ssx, ssy, ssw, ssh;
   Cutout_Rects *rects;
   Cutout_Rect  *rct;
   int c, cx, cy, cw, ch;
   int i;
   int yuv = 0;
   
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
   
   _evas_gl_common_image_update(gc, im);

   if ((im->cs.space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->cs.space == EVAS_COLORSPACE_YCBCR422P709_PL))
     yuv = 1;
   
   im->tex->im = im;
   if ((!gc->dc->cutout.rects) || (gc->dc->cutout.active > 16))
     {
        if (gc->dc->clip.use)
          {
             int nx, ny, nw, nh;
             
             nx = dx; ny = dy; nw = dw; nh = dh;
             RECTS_CLIP_TO_RECT(nx, ny, nw, nh, 
                                gc->dc->clip.x, gc->dc->clip.y,
                                gc->dc->clip.w, gc->dc->clip.h);
             if ((nw < 1) || (nh < 1)) return;
             if ((nx == dx) && (ny == dy) && (nw == dw) && (nh == dh))
               {
                  if (yuv)
                    evas_gl_common_context_yuv_push(gc,
                                                    im->tex,
                                                    sx, sy, sw, sh,
                                                    dx, dy, dw, dh,
                                                    r, g, b, a,
                                                    smooth);
                  else
                    evas_gl_common_context_image_push(gc,
                                                      im->tex,
                                                      sx, sy, sw, sh,
                                                      dx, dy, dw, dh,
                                                      r, g, b, a,
                                                      smooth, im->tex_only);
                  return;
               }
             
             ssx = (double)sx + ((double)(sw * (nx - dx)) / (double)(dw));
             ssy = (double)sy + ((double)(sh * (ny - dy)) / (double)(dh));
             ssw = ((double)sw * (double)(nw)) / (double)(dw);
             ssh = ((double)sh * (double)(nh)) / (double)(dh);
             if (yuv)
               evas_gl_common_context_yuv_push(gc,
                                               im->tex,
                                               ssx, ssy, ssw, ssh,
                                               nx, ny, nw, nh,
                                               r, g, b, a,
                                               smooth);
             else
               evas_gl_common_context_image_push(gc,
                                                 im->tex,
                                                 ssx, ssy, ssw, ssh,
                                                 nx, ny, nw, nh,
                                                 r, g, b, a,
                                                 smooth, im->tex_only);
          }
        else
          {
             if (yuv)
               evas_gl_common_context_yuv_push(gc,
                                               im->tex,
                                               sx, sy, sw, sh,
                                               dx, dy, dw, dh,
                                               r, g, b, a,
                                               smooth);
             else
               evas_gl_common_context_image_push(gc,
                                                 im->tex,
                                                 sx, sy, sw, sh,
                                                 dx, dy, dw, dh,
                                                 r, g, b, a,
                                                 smooth, im->tex_only);
          }
        return;
     }
   
   /* save out clip info */
   c = gc->dc->clip.use; cx = gc->dc->clip.x; cy = gc->dc->clip.y; cw = gc->dc->clip.w; ch = gc->dc->clip.h;
   evas_common_draw_context_clip_clip(gc->dc, 0, 0, gc->w, gc->h);
   evas_common_draw_context_clip_clip(gc->dc, dx, dy, dw, dh);
   /* our clip is 0 size.. abort */
   if ((gc->dc->clip.w <= 0) || (gc->dc->clip.h <= 0))
     {
        gc->dc->clip.use = c; gc->dc->clip.x = cx; gc->dc->clip.y = cy; gc->dc->clip.w = cw; gc->dc->clip.h = ch;
        return;
     }
   rects = evas_common_draw_context_apply_cutouts(dc);
   for (i = 0; i < rects->active; ++i)
     {
        int nx, ny, nw, nh;

        rct = rects->rects + i;
        nx = dx; ny = dy; nw = dw; nh = dh;
        RECTS_CLIP_TO_RECT(nx, ny, nw, nh, rct->x, rct->y, rct->w, rct->h);
        if ((nw < 1) || (nh < 1)) continue;
        if ((nx == dx) && (ny == dy) && (nw == dw) && (nh == dh))
          {
             if (yuv)
               evas_gl_common_context_yuv_push(gc,
                                               im->tex,
                                               sx, sy, sw, sh,
                                               dx, dy, dw, dh,
                                               r, g, b, a,
                                               smooth);
             else
               evas_gl_common_context_image_push(gc,
                                                 im->tex,
                                                 sx, sy, sw, sh,
                                                 dx, dy, dw, dh,
                                                 r, g, b, a,
                                                 smooth, im->tex_only);
             continue;
          }
        ssx = (double)sx + ((double)(sw * (nx - dx)) / (double)(dw));
        ssy = (double)sy + ((double)(sh * (ny - dy)) / (double)(dh));
        ssw = ((double)sw * (double)(nw)) / (double)(dw);
        ssh = ((double)sh * (double)(nh)) / (double)(dh);
        if (yuv)
          evas_gl_common_context_yuv_push(gc,
                                          im->tex,
                                          ssx, ssy, ssw, ssh,
                                          nx, ny, nw, nh,
                                          r, g, b, a,
                                          smooth);
        else
          evas_gl_common_context_image_push(gc,
                                            im->tex,
                                            ssx, ssy, ssw, ssh,
                                            nx, ny, nw, nh,
                                            r, g, b, a,
                                            smooth, im->tex_only);
     }
   evas_common_draw_context_apply_clear_cutouts(rects);
   /* restore clip info */
   gc->dc->clip.use = c; gc->dc->clip.x = cx; gc->dc->clip.y = cy; gc->dc->clip.w = cw; gc->dc->clip.h = ch;
}
