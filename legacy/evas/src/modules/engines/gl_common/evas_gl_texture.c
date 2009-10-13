#include "evas_gl_private.h"

static int
_nearest_pow2(int num)
{
   unsigned int n = num - 1;
   n |= n >> 1;
   n |= n >> 2;
   n |= n >> 4;
   n |= n >> 8;
   n |= n >> 16;
   return n + 1;
}

static void
_tex_adjust(Evas_GL_Context *gc, int *w, int *h)
{
   unsigned int n;
   
// disable - has a bug somewhere   
//   if (gc->info.tex_npo2) return;
   /*if (gc->info.tex_rect) return;*/
   *w = _nearest_pow2(*w);
   *h = _nearest_pow2(*h);
}

static int
_tex_round_slot(Evas_GL_Context *gc, int h)
{
// disable. has a bug somewhere   
//   if (!gc->info.tex_npo2)
     h = _nearest_pow2(h);
   return (h + 15) >> 4;
}

static int
_tex_format_index(GLuint format)
{
   switch (format)
     {
     case GL_RGBA:
        return 0;
     case GL_RGB:
        return 1;
     case GL_ALPHA:
        return 2;
     case GL_LUMINANCE:
        return 3;
     default:
        break;
     }
   return 0;
}

static Evas_GL_Texture_Pool *
_pool_tex_new(Evas_GL_Context *gc, int w, int h, GLuint format)
{
   Evas_GL_Texture_Pool *pt;
   
   pt = calloc(1, sizeof(Evas_GL_Texture_Pool));
   if (!pt) return NULL;
   _tex_adjust(gc, &w, &h);
   pt->gc = gc;
   pt->w = w;
   pt->h = h;
   pt->format = format;
   pt->references = 0;
   glGenTextures(1, &(pt->texture));
   glBindTexture(GL_TEXTURE_2D, pt->texture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0,
                format, GL_UNSIGNED_BYTE/* fixme - pass this in */, NULL);
   glBindTexture(GL_TEXTURE_2D, gc->shader.cur_tex);
   return pt;
}

static int
_pool_tex_alloc(Evas_GL_Texture_Pool *pt, GLuint format, int w, int h, int *u, int *v, Eina_List **l_after)
{
   Eina_List *l;
   Evas_GL_Texture *tex, *tex2;
   int nx, d, b;

   if (pt->allocations)
     {
        tex = pt->allocations->data;
        // if firest tex is not at left edge...
        if (tex->x > (0 + 1))
          {
             if ((tex->x - 1) >= w)
               {
                  *u = 0;
                  *v = 0;
                  *l_after = NULL;
                  return 1;
               }
          }
     }
   EINA_LIST_FOREACH(pt->allocations, l, tex)
     {
        b = tex->x + tex->w + 2;
        if (l->next)
          {
             tex2 = l->next->data;
             nx = tex2->x - 1;
          }
        else
          nx = pt->w - 1;
        d = nx - b;
        if (d >= w)
          {
             *u = b;
             *v = 0;
             *l_after = l;
             return 1;
          }
     }
   *l_after = NULL;
   return 0;
}

static Evas_GL_Texture_Pool *
_pool_tex_find(Evas_GL_Context *gc, int w, int h, GLuint format, int *u, int *v, Eina_List **l_after, int atlas_w)
{
   Evas_GL_Texture_Pool *pt = NULL;
   Eina_List *l;
   int th, th2;
   
   if ((w > 512) || (h > 512))
     {
        pt = _pool_tex_new(gc, w + 2, h + 1, format);
        gc->tex.whole = eina_list_prepend(gc->tex.whole, pt);
        pt->slot = -1;
        pt->fslot = -1;
        pt->whole = 1;
        *u = 0;
        *v = 0;
        *l_after = NULL;
        return pt;
     }
   
   th = _tex_round_slot(gc, h);
   th2 = _tex_format_index(format);
   EINA_LIST_FOREACH(gc->tex.atlas[th][th2], l, pt)
     {
        if (_pool_tex_alloc(pt, format, w, h, u, v, l_after))
          {
             gc->tex.atlas[th][th2] = 
               eina_list_remove_list(gc->tex.atlas[th][th2], l);
             gc->tex.atlas[th][th2] = 
               eina_list_prepend(gc->tex.atlas[th][th2], pt);
             return pt;
          }
     }
   pt = _pool_tex_new(gc, atlas_w, h, format);
   gc->tex.atlas[th][th2] = eina_list_prepend(gc->tex.atlas[th][th2], pt);
   pt->slot = th;
   pt->fslot = th2;
   *u = 0;
   *v = 0;
   *l_after = NULL;
   return pt;
}

Evas_GL_Texture *
evas_gl_common_texture_new(Evas_GL_Context *gc, RGBA_Image *im)
{
   Evas_GL_Texture *tex;
   Eina_List *l_after = NULL;
   int u = 0, v = 0;

   tex = calloc(1, sizeof(Evas_GL_Texture));
   if (!tex) return NULL;
   
   tex->gc = gc;
   tex->references = 1;
//   if (im->cache_entry.flags.alpha)
   tex->pt = _pool_tex_find(gc, im->cache_entry.w + 3, 
                            im->cache_entry.h + 1, GL_RGBA, 
                            &u, &v, &l_after, 1024);
//   else
//     tex->pt = _pool_tex_find(gc, im->cache_entry.w + 3, 
//                              im->cache_entry.h + 1, GL_RGB, 
//                              &u, &v, &l_after, 1024);
   if (!tex->pt)
     {
        free(tex);
        return NULL;
     }
   tex->x = u + 1;
   tex->y = v;
   tex->w = im->cache_entry.w;
   tex->h = im->cache_entry.h;
   if (l_after)
     tex->pt->allocations = 
     eina_list_append_relative_list(tex->pt->allocations, tex, l_after);
   else
     tex->pt->allocations = 
     eina_list_prepend(tex->pt->allocations, tex);
   tex->pt->references++;
   evas_gl_common_texture_update(tex, im);
   return tex;
}

void
evas_gl_common_texture_update(Evas_GL_Texture *tex, RGBA_Image *im)
{
#if defined(GL_UNSIGNED_INT_8_8_8_8_REV) && defined(GL_BGRA)
#define COLOR_FORMAT GL_RGBA   
#define PIXEL_FORMAT GL_UNSIGNED_BYTE
// FIXME: need to change shader for this to work
//#define COLOR_FORMAT GL_BGRA
//#define PIXEL_FORMAT GL_UNSIGNED_INT_8_8_8_8_REV
#else
#define COLOR_FORMAT GL_RGBA   
#define PIXEL_FORMAT GL_UNSIGNED_BYTE
#endif   
   glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
#ifdef GL_UNPACK_ROW_LENGTH   
   glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif   
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   //  +-+
   //  +-+
   // 
   glTexSubImage2D(GL_TEXTURE_2D, 0,
		   tex->x, tex->y, 
                   im->cache_entry.w, im->cache_entry.h,
		   COLOR_FORMAT, PIXEL_FORMAT,
		   im->image.data);
   // |xxx
   // |xxx
   // 
   glTexSubImage2D(GL_TEXTURE_2D, 0,
		   tex->x - 1, tex->y, 
                  1, im->cache_entry.h,
		   COLOR_FORMAT, PIXEL_FORMAT,
		   im->image.data);
   //  xxx|
   //  xxx|
   // 
   glTexSubImage2D(GL_TEXTURE_2D, 0,
		   tex->x + im->cache_entry.w, tex->y, 
                  1, im->cache_entry.h,
		   COLOR_FORMAT, PIXEL_FORMAT,
		   im->image.data + (im->cache_entry.w - 1));
   //  xxx
   //  xxx
   //  ---
   glTexSubImage2D(GL_TEXTURE_2D, 0,
		   tex->x, tex->y + im->cache_entry.h,
                  im->cache_entry.w, 1,
		   COLOR_FORMAT, PIXEL_FORMAT,
		   im->image.data + ((im->cache_entry.h - 1) * im->cache_entry.w));
   //  xxx
   //  xxx
   // o
   glTexSubImage2D(GL_TEXTURE_2D, 0,
		   tex->x - 1, tex->y + im->cache_entry.h,
                  1, 1,
		   COLOR_FORMAT, PIXEL_FORMAT,
		   im->image.data + ((im->cache_entry.h - 1) * im->cache_entry.w));
   //  xxx
   //  xxx
   //     o
   glTexSubImage2D(GL_TEXTURE_2D, 0,
		   tex->x + im->cache_entry.w, tex->y + im->cache_entry.h,
                  1, 1,
		   COLOR_FORMAT, PIXEL_FORMAT,
		   im->image.data + ((im->cache_entry.h - 1) * im->cache_entry.w) + (im->cache_entry.w - 1));
   if (tex->pt->texture != tex->gc->shader.cur_tex)
     {
        glBindTexture(GL_TEXTURE_2D, tex->gc->shader.cur_tex);
     }
}

static void
pt_unref(Evas_GL_Texture_Pool *pt)
{
   pt->references--;
   if (pt->references > 0) return;
   if (pt->whole)
     {
        pt->gc->tex.whole = eina_list_remove(pt->gc->tex.whole, pt);
     }
   else
     {
        pt->gc->tex.atlas [pt->slot][pt->fslot] =
          eina_list_remove(pt->gc->tex.atlas[pt->slot][pt->fslot], pt);
     }
   glDeleteTextures(1, &(pt->texture));
   free(pt);
}

void
evas_gl_common_texture_free(Evas_GL_Texture *tex)
{
   if (!tex) return;
   tex->references--;
   if (tex->references > 0) return;
   tex->pt->allocations = eina_list_remove(tex->pt->allocations, tex);
   pt_unref(tex->pt);
   if (tex->ptu) pt_unref(tex->ptu);
   if (tex->ptv) pt_unref(tex->ptv);
   free(tex);
}

Evas_GL_Texture *
evas_gl_common_texture_alpha_new(Evas_GL_Context *gc, DATA8 *pixels, int w, int h, int fh)
{
   Evas_GL_Texture *tex;
   Eina_List *l_after = NULL;
   int u = 0, v = 0;
   int tw = 4096;

   tex = calloc(1, sizeof(Evas_GL_Texture));
   if (!tex) return NULL;
   
   tex->gc = gc;
   tex->references = 1;
   if (tw > gc->info.max_texture_size) tw = gc->info.max_texture_size;
   tex->pt = _pool_tex_find(gc, w + 3, fh, GL_ALPHA,
                            &u, &v, &l_after, tw);
   if (!tex->pt)
     {
        free(tex);
        return NULL;
     }
   tex->x = u + 1;
   tex->y = v;
   tex->w = w;
   tex->h = h;
   if (l_after)
     tex->pt->allocations = eina_list_append_relative(tex->pt->allocations, tex, l_after);
   else
     tex->pt->allocations = eina_list_prepend(tex->pt->allocations, tex);
   tex->pt->references++;
   evas_gl_common_texture_alpha_update(tex, pixels, w, h, fh);
   return tex;
}

void
evas_gl_common_texture_alpha_update(Evas_GL_Texture *tex, DATA8 *pixels, int w, int h, int fh)
{
   glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
#ifdef GL_UNPACK_ROW_LENGTH   
   glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif   
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   glTexSubImage2D(GL_TEXTURE_2D, 0,
		   tex->x, tex->y, w, h,
		   GL_ALPHA, GL_UNSIGNED_BYTE,
		   pixels);
   if (tex->pt->texture != tex->gc->shader.cur_tex)
     {
        glBindTexture(GL_TEXTURE_2D, tex->gc->shader.cur_tex);
     }
}

Evas_GL_Texture *
evas_gl_common_texture_yuv_new(Evas_GL_Context *gc, DATA8 **rows, int w, int h)
{
   Evas_GL_Texture *tex;
   Eina_List *l_after = NULL;
   int u = 0, v = 0;

   tex = calloc(1, sizeof(Evas_GL_Texture));
   if (!tex) return NULL;
   
   tex->gc = gc;
   tex->references = 1;
   tex->pt = _pool_tex_new(gc, w + 1, h  + 1, GL_LUMINANCE);
   gc->tex.whole = eina_list_prepend(gc->tex.whole, tex->pt);
   tex->pt->slot = -1;
   tex->pt->fslot = -1;
   tex->pt->whole = 1;
   tex->ptu = _pool_tex_new(gc, (w / 2) + 1, (h / 2)  + 1, GL_LUMINANCE);
   gc->tex.whole = eina_list_prepend(gc->tex.whole, tex->ptu);
   tex->ptu->slot = -1;
   tex->ptu->fslot = -1;
   tex->ptu->whole = 1;
   tex->ptv = _pool_tex_new(gc, (w / 2) + 1, (h / 2)  + 1, GL_LUMINANCE);
   gc->tex.whole = eina_list_prepend(gc->tex.whole, tex->ptv);
   tex->ptv->slot = -1;
   tex->ptv->fslot = -1;
   tex->ptv->whole = 1;
   tex->x = 0;
   tex->y = 0;
   tex->w = w;
   tex->h = h;
   tex->pt->allocations = eina_list_prepend(tex->pt->allocations, tex);
   tex->ptu->allocations = eina_list_prepend(tex->ptu->allocations, tex);
   tex->ptv->allocations = eina_list_prepend(tex->ptv->allocations, tex);
   tex->pt->references++;
   tex->ptu->references++;
   tex->ptv->references++;
   evas_gl_common_texture_yuv_update(tex, rows, w, h);
   return tex;
}

void
evas_gl_common_texture_yuv_update(Evas_GL_Texture *tex, DATA8 **rows, int w, int h)
{
   // FIXME: works on lowest size 4 pixel high buffers. must also be multiple of 2
#ifdef GL_UNPACK_ROW_LENGTH
   glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[1] - rows[0]);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
   glTexSubImage2D(GL_TEXTURE_2D, 0,
		   0, 0, w, h,
                   GL_LUMINANCE, GL_UNSIGNED_BYTE,
		   rows[0]);
   glBindTexture(GL_TEXTURE_2D, tex->ptu->texture);
   glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[h + 1] - rows[h]);
   glTexSubImage2D(GL_TEXTURE_2D, 0,
		   0, 0, w / 2, h / 2,
                   GL_LUMINANCE, GL_UNSIGNED_BYTE,
		   rows[h]);
   glBindTexture(GL_TEXTURE_2D, tex->ptv->texture);
   glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[h + (h / 2) + 1] - rows[h + (h / 2)]);
   glTexSubImage2D(GL_TEXTURE_2D, 0,
		   0, 0, w / 2, h / 2,
                   GL_LUMINANCE, GL_UNSIGNED_BYTE,
		   rows[h + (h / 2)]);
#else
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
   if ((rows[1] - rows[0]) == w)
     glTexSubImage2D(GL_TEXTURE_2D, 0,
                     0, 0, w, h,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE,
                     rows[0]);
   else
     {
        int y;
        
        for (y = 0; y < h; y++)
          glTexSubImage2D(GL_TEXTURE_2D, 0,
                          0, y, w, 1,
                          GL_LUMINANCE, GL_UNSIGNED_BYTE,
                          rows[y]);
     }

   glBindTexture(GL_TEXTURE_2D, tex->ptu->texture);
   if ((rows[h + 1] - rows[h]) == (w / 2))
     glTexSubImage2D(GL_TEXTURE_2D, 0,
                     0, 0, w / 2, h / 2,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE,
                     rows[h]);
   else
     {
        int y;
        
        for (y = 0; y < (h / 2); y++)
          glTexSubImage2D(GL_TEXTURE_2D, 0,
                          0, y, w / 2, 1,
                          GL_LUMINANCE, GL_UNSIGNED_BYTE,
                          rows[h + y]);
     }
   
   glBindTexture(GL_TEXTURE_2D, tex->ptv->texture);
   if ((rows[h + (h / 2) + 1] - rows[h + (h / 2)]) == (w / 2))
     glTexSubImage2D(GL_TEXTURE_2D, 0,
                     0, 0, w / 2, h / 2,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE,
                     rows[h + (h / 2)]);
   else
     {
        int y;
        
        for (y = 0; y < (h / 2); y++)
          glTexSubImage2D(GL_TEXTURE_2D, 0,
                          0, y, w / 2, 1,
                          GL_LUMINANCE, GL_UNSIGNED_BYTE,
                          rows[h + (h / 2) + y]);
     }
#endif   
   if (tex->pt->texture != tex->gc->shader.cur_tex)
     {
        glBindTexture(GL_TEXTURE_2D, tex->gc->shader.cur_tex);
     }
}
