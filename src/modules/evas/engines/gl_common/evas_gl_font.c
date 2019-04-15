#include "evas_gl_private.h"

void *
evas_gl_font_texture_new(void *context, RGBA_Font_Glyph *fg)
{
   Evas_Engine_GL_Context *gc = context;
   Evas_GL_Texture *tex;
   int w, h, nw, fh, y;
   DATA8 *ndata, *data, *p1, *p2;

   if (fg->ext_dat) return fg->ext_dat; // FIXME: one engine at a time can do this :(

   w = fg->glyph_out->bitmap.width;
   h = fg->glyph_out->bitmap.rows;
   if ((w == 0) || (h == 0)) return NULL;

   if (!fg->glyph_out->rle) return NULL;
   data = evas_common_font_glyph_uncompress(fg, &w, &h);
   if (!data) return NULL;

   // expand to 32bit (4 byte) aligned rows for texture upload
   nw = ((w + 3) / 4) * 4;
   // if image already (4 byte) aligned rows then assign old image data to new data(ndata) to 
   if(nw == w){
      ndata = data;
   } else {
      ndata = alloca(nw *h);
      if (!ndata) return NULL;
      // else copy row by row
      for (y = 0; y < h; y++)
      {
        p1 = data + (w * y);
        p2 = ndata + (nw * y);
        memcpy(p2,p1,w);
      }
   }

   fh = fg->fi->max_h;
   tex = evas_gl_common_texture_alpha_new(gc, ndata, w, h, fh);
   if (!tex) goto done;
   tex->sx1 = ((double)(tex->x)) / (double)tex->pt->w;
   tex->sy1 = ((double)(tex->y)) / (double)tex->pt->h;
   tex->sx2 = ((double)(tex->x + tex->w)) / (double)tex->pt->w;
   tex->sy2 = ((double)(tex->y + tex->h)) / (double)tex->pt->h;
   tex->fglyph = fg;
   gc->font_glyph_textures = eina_list_append(gc->font_glyph_textures, tex);
done:
   free(data);
   return tex;
}

void
evas_gl_font_texture_free(void *tex)
{
   if (!tex) return;
   evas_gl_common_texture_free(tex, EINA_TRUE);
}

void
evas_gl_font_texture_draw(void *context, void *surface EINA_UNUSED, void *draw_context, RGBA_Font_Glyph *fg, int x, int y, int w, int h)
{
   Evas_Engine_GL_Context *gc = context;
   RGBA_Draw_Context *dc = draw_context;
   Evas_GL_Image *mask = gc->dc->clip.mask;
   Evas_GL_Texture *tex, *mtex = NULL;
   Cutout_Rect  *rct;
   int r, g, b, a;
   double ssx, ssy, ssw, ssh;
   int c, cx, cy, cw, ch;
   int i;
   int sx, sy, sw, sh;
   double mx = 0.0, my = 0.0, mw = 0.0, mh = 0.0;
   Eina_Bool mask_smooth = EINA_FALSE;
   Eina_Bool mask_color = EINA_FALSE;

   if (dc != gc->dc) return;
   tex = fg->ext_dat;
   if (!tex) return;
   a = (dc->col.col >> 24) & 0xff;
   if (a == 0) return;
   r = (dc->col.col >> 16) & 0xff;
   g = (dc->col.col >> 8 ) & 0xff;
   b = (dc->col.col      ) & 0xff;
   sx = 0; sy = 0; sw = tex->w, sh = tex->h;

   if (mask)
     {
        evas_gl_common_image_update(gc, mask);
        mtex = mask->tex;
        if (mtex && mtex->pt && mtex->pt->w && mtex->pt->h)
          {
             // canvas coords
             mx = gc->dc->clip.mask_x;
             my = gc->dc->clip.mask_y;
             mw = mask->w;
             mh = mask->h;
             mask_smooth = mask->scaled.smooth;
             mask_color = gc->dc->clip.mask_color;
          }
        else mtex = NULL;
     }

   if ((!gc->dc->cutout.rects) ||
       ((gc->shared->info.tune.cutout.max > 0) &&
           (gc->dc->cutout.active > gc->shared->info.tune.cutout.max)))
     {
        if (gc->dc->clip.use)
          {
             int nx, ny, nw, nh;

             nx = x; ny = y; nw = w; nh = h;
             RECTS_CLIP_TO_RECT(nx, ny, nw, nh,
                                gc->dc->clip.x, gc->dc->clip.y,
                                gc->dc->clip.w, gc->dc->clip.h);
             if ((nw < 1) || (nh < 1)) return;
             if ((nx == x) && (ny == y) && (nw == w) && (nh == h))
               {
                  evas_gl_common_context_font_push(gc, tex,
                                                   0.0, 0.0, 0.0, 0.0,
//                                                   sx, sy, sw, sh,
                                                   x, y, w, h,
                                                   mtex, mx, my, mw, mh, mask_smooth, mask_color,
                                                   r, g, b, a);
                  return;
               }
             ssx = (double)sx + ((double)(sw * (nx - x)) / (double)(w));
             ssy = (double)sy + ((double)(sh * (ny - y)) / (double)(h));
             ssw = ((double)sw * (double)(nw)) / (double)(w);
             ssh = ((double)sh * (double)(nh)) / (double)(h);
             evas_gl_common_context_font_push(gc, tex,
                                              ssx, ssy, ssw, ssh,
                                              nx, ny, nw, nh,
                                              mtex, mx, my, mw, mh, mask_smooth, mask_color,
                                              r, g, b, a);
          }
        else
          {
             evas_gl_common_context_font_push(gc, tex,
                                              0.0, 0.0, 0.0, 0.0,
//                                              sx, sy, sw, sh,
                                              x, y, w, h,
                                              mtex, mx, my, mw, mh, mask_smooth, mask_color,
                                              r, g, b, a);
          }
        return;
     }
   /* save out clip info */
   c = gc->dc->clip.use; cx = gc->dc->clip.x; cy = gc->dc->clip.y; cw = gc->dc->clip.w; ch = gc->dc->clip.h;
   evas_common_draw_context_clip_clip(gc->dc, 0, 0, gc->shared->w, gc->shared->h);
   evas_common_draw_context_clip_clip(gc->dc, x, y, w, h);
   /* our clip is 0 size.. abort */
   if ((gc->dc->clip.w <= 0) || (gc->dc->clip.h <= 0))
     {
        gc->dc->clip.use = c; gc->dc->clip.x = cx; gc->dc->clip.y = cy; gc->dc->clip.w = cw; gc->dc->clip.h = ch;
        return;
     }
   _evas_gl_common_cutout_rects = evas_common_draw_context_apply_cutouts(dc, _evas_gl_common_cutout_rects);
   for (i = 0; i < _evas_gl_common_cutout_rects->active; ++i)
     {
        int nx, ny, nw, nh;

        rct = _evas_gl_common_cutout_rects->rects + i;
        nx = x; ny = y; nw = w; nh = h;
        RECTS_CLIP_TO_RECT(nx, ny, nw, nh, rct->x, rct->y, rct->w, rct->h);
        if ((nw < 1) || (nh < 1)) continue;
        if ((nx == x) && (ny == y) && (nw == w) && (nh == h))
          {
             evas_gl_common_context_font_push(gc, tex,
                                              0.0, 0.0, 0.0, 0.0,
//                                              sx, sy, sw, sh,
                                              x, y, w, h,
                                              mtex, mx, my, mw, mh, mask_smooth, mask_color,
                                              r, g, b, a);
             continue;
          }
        ssx = (double)sx + ((double)(sw * (nx - x)) / (double)(w));
        ssy = (double)sy + ((double)(sh * (ny - y)) / (double)(h));
        ssw = ((double)sw * (double)(nw)) / (double)(w);
        ssh = ((double)sh * (double)(nh)) / (double)(h);
        evas_gl_common_context_font_push(gc, tex,
                                         ssx, ssy, ssw, ssh,
                                         nx, ny, nw, nh,
                                         mtex, mx, my, mw, mh, mask_smooth, mask_color,
                                         r, g, b, a);
     }
   evas_common_draw_context_cutouts_free(_evas_gl_common_cutout_rects);
   /* restore clip info */
   gc->dc->clip.use = c; gc->dc->clip.x = cx; gc->dc->clip.y = cy; gc->dc->clip.w = cw; gc->dc->clip.h = ch;
}

void *
evas_gl_font_image_new(void *gc, RGBA_Font_Glyph *fg, int alpha, Evas_Colorspace cspace)
{
   Evas_Engine_GL_Context *context = (Evas_Engine_GL_Context *)gc;
   Evas_GL_Image *im = evas_gl_common_image_new_from_data(context,
                                                          (unsigned int)fg->glyph_out->bitmap.width,
                                                          (unsigned int)fg->glyph_out->bitmap.rows,
                                                          (DATA32 *)fg->glyph_out->bitmap.buffer,
                                                          alpha,
                                                          cspace);

   if (im)
     {
        im->fglyph = fg;
        context->font_glyph_images = eina_list_append(context->font_glyph_images, im);
     }

   return (void *)im;
}

void
evas_gl_font_image_free(void *im)
{
   evas_gl_common_image_free((Evas_GL_Image *)im);
}

void
evas_gl_font_image_draw(void *gc, void *gl_image, int dx, int dy, int dw, int dh, int smooth)
{
   Evas_GL_Image *im = (Evas_GL_Image *)gl_image;

   if (!im || !im->fglyph) return;

   evas_gl_common_image_draw((Evas_Engine_GL_Context *)gc,
                             im, 0, 0,
                             (unsigned int)im->fglyph->glyph_out->bitmap.width,
                             (unsigned int)im->fglyph->glyph_out->bitmap.rows,
                             dx, dy, dw, dh,
                             smooth);
}
