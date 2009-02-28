#include "evas_gl_private.h"

static Evas_GL_Font_Texture_Pool_Allocation *_evas_gl_font_texture_pool_request(Evas_GL_Context *gc, int w, int h);
static void                                  _evas_gl_font_texture_pool_relinquish(Evas_GL_Font_Texture_Pool_Allocation *fa);
static int                                   _evas_gl_font_texture_pool_rect_find(Evas_GL_Font_Texture_Pool *fp, int w, int h, int *x, int *y);

Evas_GL_Font_Texture *
evas_gl_font_texture_new(Evas_GL_Context *gc, RGBA_Font_Glyph *fg)
{
   Evas_GL_Font_Texture *ft;
   DATA8 *data;
   int w, h, j;

   int nw;
   DATA8 *ndata;

   if (fg->ext_dat) return fg->ext_dat;
   
   w = fg->glyph_out->bitmap.width;
   h = fg->glyph_out->bitmap.rows;
   
   if ((w == 0) || (h == 0)) return NULL;
   ft = calloc(1, sizeof(Evas_GL_Font_Texture));
   if (!ft) return NULL;

   data = fg->glyph_out->bitmap.buffer;
   j = fg->glyph_out->bitmap.pitch;
   if (j < w) j = w;

   ft->gc = gc;

   /* bug bug! glTexSubImage2D need a multiple of 4 pixels horizontally! :( */
   nw = ((w + 3) / 4 ) * 4;
   ndata = malloc(nw *h);
   if (!ndata)
     {
	free(ft);
	return NULL;
     }
   if (fg->glyph_out->bitmap.num_grays == 256)
     {
	int x, y;
	DATA8 *p1, *p2;

	for (y = 0; y < h; y++)
	  {
	     p1 = data + (j * y);
	     p2 = ndata + (nw * y);
	     for (x = 0; x < w; x++)
	       {
		  *p2 = *p1;
		  p1++;
		  p2++;
	       }
	  }
     }
   else if (fg->glyph_out->bitmap.num_grays == 0)
     {
	DATA8 *tmpbuf = NULL, *dp, *tp, bits;
	int bi, bj, end;
	const DATA8 bitrepl[2] = {0x0, 0xff};
	
	tmpbuf = malloc(w);
	if (tmpbuf)
	  {
	     int x, y;
	     DATA8 *p1, *p2;
	     
	     for (y = 0; y < h; y++)
	       {
		  p1 = tmpbuf;
		  p2 = ndata + (nw * y);
		  tp = tmpbuf;
		  dp = data + (y * fg->glyph_out->bitmap.pitch);
		  for (bi = 0; bi < w; bi += 8)
		    {
		       bits = *dp;
		       if ((w - bi) < 8) end = w - bi;
		       else end = 8;
		       for (bj = 0; bj < end; bj++)
			 {
			    *tp = bitrepl[(bits >> (7 - bj)) & 0x1];
			    tp++;
			 }
		       dp++;
		    }
		  for (x = 0; x < w; x++)
		    {
		       *p2 = *p1;
		       p1++;
		       p2++;
		    }
	       }
	     free(tmpbuf);
	  }
     }
   
   /* where in pool texture does this live */
   ft->w = w;
   ft->h = h;
   ft->aw = nw;
   ft->ah = h;

   ft->alloc = _evas_gl_font_texture_pool_request(gc, ft->aw, ft->ah);
   if (!ft->alloc)
     {
	free(ndata);
	free(ft);
	return NULL;
     }
   ft->x = ft->alloc->x;
   ft->y = ft->alloc->y;
   ft->pool = ft->alloc->pool;
   ft->texture =  ft->pool->texture;
   if (ft->pool->rectangle)
     {
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, ft->texture);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
			ft->x, ft->y, nw, ft->h,
			GL_ALPHA, GL_UNSIGNED_BYTE, ndata);
     }
   else
     {
	glBindTexture(GL_TEXTURE_2D, ft->texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0,
			ft->x, ft->y, nw, ft->h,
			GL_ALPHA, GL_UNSIGNED_BYTE, ndata);
     }
   if (ndata) free(ndata);
   if (gc->texture)
     {
	if (gc->texture) gc->texture->references--;
	gc->texture = NULL;
     }
   gc->font_texture = ft->texture;
   gc->font_texture_rectangle = ft->pool->rectangle;
   gc->change.texture = 1;
   if (ft->pool->rectangle)
     {
	ft->tx1 = ft->x;
	ft->ty1 = ft->y;
	ft->tx2 = ft->x + ft->w;
	ft->ty2 = ft->y + ft->h;
     }
   else
     {
	ft->tx1 = (double)(ft->x        ) / (double)(ft->pool->w);
	ft->ty1 = (double)(ft->y        ) / (double)(ft->pool->h);
	ft->tx2 = (double)(ft->x + ft->w) / (double)(ft->pool->w);
	ft->ty2 = (double)(ft->y + ft->h) / (double)(ft->pool->h);
     }

   return ft;
}

void
evas_gl_font_texture_free(Evas_GL_Font_Texture *ft)
{
   if (!ft) return;
   if (ft->gc->font_texture == ft->texture)
     {
	ft->gc->font_texture = 0;
	ft->gc->change.texture = 1;
     }
   _evas_gl_font_texture_pool_relinquish(ft->alloc);
   free(ft);
}

void
evas_gl_font_texture_draw(Evas_GL_Context *gc, void *surface __UNUSED__, RGBA_Draw_Context *dc, RGBA_Font_Glyph *fg, int x, int y)
{
   Evas_GL_Font_Texture *ft;

   if (dc != gc->dc)
	return;

   /* 35 */
   ft = fg->ext_dat;
   if (!ft) return;
//   if (surface == 0)
     {
	int r, g, b, a;

	a = (dc->col.col >> 24) & 0xff;
	if (a == 0) return;
	r = (dc->col.col >> 16) & 0xff;
	g = (dc->col.col >> 8 ) & 0xff;
	b = (dc->col.col      ) & 0xff;
	/* have to un-premul the color - as we are using blend mode 2 (non-premul blend) */
	r = (r * 255) / a;
	g = (g * 255) / a;
	b = (b * 255) / a;
	evas_gl_common_context_color_set(gc, r, g, b, a);
	if (dc->clip.use)
	  evas_gl_common_context_clip_set(gc, 1,
					  dc->clip.x, dc->clip.y,
					  dc->clip.w, dc->clip.h);
	else
	  evas_gl_common_context_clip_set(gc, 0,
					  0, 0, 0, 0);
	evas_gl_common_context_blend_set(gc, 2);
	evas_gl_common_context_read_buf_set(gc, GL_BACK);
	evas_gl_common_context_write_buf_set(gc, GL_BACK);
     }
   /* 32 */
   evas_gl_common_context_font_texture_set(gc, ft);
   /* 32 */
   glBegin(GL_QUADS);
   glTexCoord2d(ft->tx1, ft->ty1); glVertex2i(x        , y        );
   glTexCoord2d(ft->tx2, ft->ty1); glVertex2i(x + ft->w, y        );
   glTexCoord2d(ft->tx2, ft->ty2); glVertex2i(x + ft->w, y + ft->h);
   glTexCoord2d(ft->tx1, ft->ty2); glVertex2i(x        , y + ft->h);
   glEnd();
   /* 28 */
}

static Evas_GL_Font_Texture_Pool_Allocation *
_evas_gl_font_texture_pool_request(Evas_GL_Context *gc, int w, int h)
{
   Eina_List *l;
   Evas_GL_Font_Texture_Pool_Allocation *fa;
   Evas_GL_Font_Texture_Pool *fp;
   int minw = 256;
   int minh = 256;
   int shift;

   EINA_LIST_FOREACH(gc->tex_pool, l, fp)
     {
	int x, y;

	if (_evas_gl_font_texture_pool_rect_find(fp, w, h, &x, &y))
	  {
	     fa = calloc(1, sizeof(Evas_GL_Font_Texture_Pool_Allocation));
	     if (!fa) return NULL;
	     fa->pool = fp;
	     fa->x = x;
	     fa->y = y;
	     fa->w = w;
	     fa->h = h;
	     fp->allocations = eina_list_prepend(fp->allocations, fa);
	     if (eina_error_get())
	       {
		  free(fa);
		  return NULL;
	       }
	     fp->references++;
	     return fa;
	  }
     }
   /* need new font texture pool entry */
   /* minimum size either minw x minh OR the size of glyph up to power 2 */
   if (w > minw)
     {
	minw = w;
	shift = 1; while (minw > shift) shift = shift << 1; minw = shift;
     }
   if (h > minh)
     {
	minh = h;
	shift = 1; while (minh > shift) shift = shift << 1; minh = shift;
     }

   fp = calloc(1, sizeof(Evas_GL_Font_Texture_Pool));
   if (!fp) return NULL;
   gc->tex_pool = eina_list_append(gc->tex_pool, fp);
   if (eina_error_get())
     {
	free(fp);
	return NULL;
     }
   fp->gc = gc;
   fp->w = minw;
   fp->h = minh;
   if (gc->ext.nv_texture_rectangle) fp->rectangle = 1;

   /* we dont want this mipmapped if sgis_generate_mipmap will mipmap it */
   if (gc->ext.sgis_generate_mipmap)
     glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
//   glEnable(GL_TEXTURE_2D);
   if (fp->rectangle)
     {
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	glGenTextures(1, &(fp->texture));
	/* FIXME check gl error */

	glBindTexture(GL_TEXTURE_RECTANGLE_NV, fp->texture);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
		     GL_ALPHA4, fp->w, fp->h, 0,
		     GL_ALPHA, GL_UNSIGNED_BYTE, NULL);
	/* FIXME check gl error */
     }
   else
     {
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &(fp->texture));
	/* FIXME check gl error */

	glBindTexture(GL_TEXTURE_2D, fp->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0,
		     GL_ALPHA4, fp->w, fp->h, 0,
		     GL_ALPHA, GL_UNSIGNED_BYTE, NULL);
	/* FIXME check gl error */
     }

   /* new allocation entirely */
   fa = calloc(1, sizeof(Evas_GL_Font_Texture_Pool_Allocation));
   if (!fa)
     {
	gc->tex_pool = eina_list_remove(gc->tex_pool, fp);
	glDeleteTextures(1, &(fp->texture));
	free(fp);
	return NULL;
     }
   fa->pool = fp;
   fa->x = 0;
   fa->y = 0;
   fa->w = w;
   fa->h = h;
   fp->allocations = eina_list_prepend(fp->allocations, fa);
   if (eina_error_get())
     {
	printf("alloc prob\n");
	gc->tex_pool = eina_list_remove(gc->tex_pool, fp);
	glDeleteTextures(1, &(fp->texture));
	free(fa);
	free(fp);
	return NULL;
     }
   fp->references++;
   return fa;
}

static void
_evas_gl_font_texture_pool_relinquish(Evas_GL_Font_Texture_Pool_Allocation *fa)
{
   fa->pool->allocations = eina_list_remove(fa->pool->allocations, fa);
   fa->pool->references--;
   if (fa->pool->references <= 0)
     {
	fa->pool->gc->tex_pool =
	  eina_list_remove(fa->pool->gc->tex_pool, fa->pool);
	glDeleteTextures(1, &(fa->pool->texture));
	free(fa->pool);
     }
   free(fa);
}

static int
_evas_gl_font_texture_pool_rect_find(Evas_GL_Font_Texture_Pool *fp,
				     int w, int h,
				     int *x, int *y)
{
   Eina_List *l;
   Evas_GL_Font_Texture_Pool_Allocation *fa;

   if ((w > fp->w) || (h > fp->h)) return 0;
   EINA_LIST_FOREACH(fp->allocations, l, fa)
     {
	Eina_List *l2;
	Evas_GL_Font_Texture_Pool_Allocation *fa2;
	int tx, ty, tw, th;
	int t1, t2;
	int intersects;

	t1 = t2 = 1;
	if ((fa->x + fa->w + w) > fp->w) t1 = 0;
	if ((fa->y + h) > fp->h) t1 = 0;
	if ((fa->y + fa->h + h) > fp->h) t2 = 0;
	if ((fa->x + w) > fp->w) t2 = 0;
	intersects = 0;
	if (t1)
	  {
	     /* 1. try here:
	      * +----++--+
	      * |AAAA||??|
	      * |AAAA|+--+
	      * |AAAA|
	      * +----+
	      */
	     tx = fa->x + fa->w;
	     ty = fa->y;
	     tw = w;
	     th = h;
	     EINA_LIST_FOREACH(fp->allocations, l2, fa2)
	       {
		  int rx, ry, rw, rh;

		  /* dont do the rect we are just using as our offset */
		  if (l2 == l) continue;
		  rx = fa2->x;
		  ry = fa2->y;
		  rw = fa2->w;
		  rh = fa2->h;
		  if (RECTS_INTERSECT(tx, ty, tw, th, rx, ry, rw, rh))
		    {
		       intersects = 1;
		       break;
		    }
	       }
	     if (!intersects)
	       {
		  *x = tx;
		  *y = ty;
		  return 1;
	       }
	  }
	intersects = 0;
	if (t2)
	  {
	     /* 2. try here:
	      * +----+
	      * |AAAA|
	      * |AAAA|
	      * |AAAA|
	      * +----+
	      * +--+
	      * |??|
	      * +--+
	      */
	     tx = fa->x;
	     ty = fa->y + fa->h;
	     tw = w;
	     th = h;
	     EINA_LIST_FOREACH(fp->allocations, l2, fa2)
	       {
		  int rx, ry, rw, rh;

		  /* dont do the rect we are just using as our offset */
		  if (l2 == l) continue;
		  /* hmmm crash here on mga... l2->data seems broken */
		  /* so far it looks like memory corruption, but i can't */
		  /* use valgrind to inspect any further due to the dri */
		  /* hardware stuff :( */

		  rx = fa2->x;
		  ry = fa2->y;
		  rw = fa2->w;
		  rh = fa2->h;
		  if (RECTS_INTERSECT(tx, ty, tw, th, rx, ry, rw, rh))
		    {
		       intersects = 1;
		       break;
		    }
	       }
	     if (!intersects)
	       {
		  *x = tx;
		  *y = ty;
		  return 1;
	       }
	  }
     }
   return 0;
}
