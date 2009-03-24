#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_X11.h"

static Eina_Hash *_xr_fg_pool = NULL;

XR_Font_Surface *
_xre_xcb_font_surface_new(Ximage_Info *xinf, RGBA_Font_Glyph *fg)
{
   char             buf[256];
   char             buf2[256];
   uint32_t         values[3];
   XR_Font_Surface *fs;
   DATA8           *data;
   Ximage_Image    *xim;
   Eina_Hash       *pool;
   uint32_t         mask;
   int              w;
   int              h;
   int              pitch;

   data = fg->glyph_out->bitmap.buffer;
   w = fg->glyph_out->bitmap.width;
   h = fg->glyph_out->bitmap.rows;
   pitch = fg->glyph_out->bitmap.pitch;
   if (pitch < w) pitch = w;
   if ((w <= 0) || (h <= 0)) return NULL;

   if (fg->ext_dat)
     {
	fs = fg->ext_dat;
	if ((fs->xinf->x11.connection == xinf->x11.connection) &&
            (fs->xinf->x11.root == xinf->x11.root))
	  return fs;
	snprintf(buf, sizeof(buf), "@%p@/@%x@", fs->xinf->x11.connection, fs->xinf->x11.root);
	pool = eina_hash_find(_xr_fg_pool, buf);
	if (pool)
	  {
	     snprintf(buf, sizeof(buf), "%p", fg);
	     fs = eina_hash_find(pool, buf);
	     if (fs) return fs;
	  }
     }

   fs = calloc(1, sizeof(XR_Font_Surface));
   if (!fs) return NULL;

   fs->xinf = xinf;
   fs->fg = fg;
   fs->xinf->references++;
   fs->w = w;
   fs->h = h;

   snprintf(buf, sizeof(buf), "@%p@/@%x@", fs->xinf->x11.connection, fs->xinf->x11.root);
   pool = eina_hash_find(_xr_fg_pool, buf);
   if (!pool) pool = eina_hash_string_superfast_new(NULL);
   snprintf(buf2, sizeof(buf2), "%p", fg);
   eina_hash_add(pool, buf2, fs);
   if (!_xr_fg_pool) _xr_fg_pool = eina_hash_string_superfast_new(NULL);
   eina_hash_add(_xr_fg_pool, buf, pool);

   fs->draw = xcb_generate_id(xinf->x11.connection);
   xcb_create_pixmap(xinf->x11.connection, ((xcb_render_pictforminfo_t *)xinf->x11.fmt8)->depth, fs->draw, xinf->x11.root, w, h);

   mask = XCB_RENDER_CP_REPEAT | XCB_RENDER_CP_DITHER | XCB_RENDER_CP_COMPONENT_ALPHA;
   values[0] = 0;
   values[1] = 0;
   values[2] = 0;
   fs->pic = xcb_generate_id(xinf->x11.connection);
   xcb_render_create_picture(xinf->x11.connection, fs->pic, fs->draw, ((xcb_render_pictforminfo_t *)xinf->x11.fmt8)->id, mask, values);

   xim = _xr_xcb_image_new(fs->xinf, w, h, ((xcb_render_pictforminfo_t *)xinf->x11.fmt8)->depth);
   if ((fg->glyph_out->bitmap.num_grays == 256) &&
       (fg->glyph_out->bitmap.pixel_mode == ft_pixel_mode_grays))
     {
	int x, y;
	DATA8 *p1, *p2;

	for (y = 0; y < h; y++)
	  {
	     p1 = data + (pitch * y);
	     p2 = ((DATA8 *)xim->data) + (xim->line_bytes * y);
	     for (x = 0; x < w; x++)
	       {
		  *p2 = *p1;
		  p1++;
		  p2++;
	       }
	  }

     }
   else
     {
        DATA8      *tmpbuf = NULL, *dp, *tp, bits;
	int         bi, bj, end;
	const DATA8 bitrepl[2] = {0x0, 0xff};

	tmpbuf = alloca(w);
	  {
	     int    x, y;
	     DATA8 *p1, *p2;

	     for (y = 0; y < h; y++)
	       {
		  p1 = tmpbuf;
		  p2 = ((DATA8 *)xim->data) + (xim->line_bytes * y);
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
	  }
     }
   _xr_xcb_image_put(xim, fs->draw, 0, 0, w, h);
   return fs;
}

static Eina_Bool
_xre_xcb_font_pool_cb(const Eina_Hash *hash, const void *key, void *data, void *fdata)
{
   char             buf[256];
   Eina_Hash       *pool;
   XR_Font_Surface *fs;

   fs = fdata;
   pool = data;
   snprintf(buf, sizeof(buf), "@%p@/@%x@", fs->xinf->x11.connection, fs->xinf->x11.root);
   eina_hash_del(pool, buf, fs);
   if (!hash) hash = eina_hash_string_superfast_new(NULL);
   eina_hash_modify(hash, key, pool);
   return 1;
}

void
_xre_xcb_font_surface_free(XR_Font_Surface *fs)
{
   if (!fs) return;
   eina_hash_foreach(_xr_fg_pool, _xre_xcb_font_pool_cb, fs);
   xcb_free_pixmap(fs->xinf->x11.connection, fs->draw);
   xcb_render_free_picture(fs->xinf->x11.connection, fs->pic);
   _xr_xcb_image_info_free(fs->xinf);
   free(fs);
}

void
_xre_xcb_font_surface_draw(Ximage_Info *xinf __UNUSED__, RGBA_Image *surface, RGBA_Draw_Context *dc, RGBA_Font_Glyph *fg, int x, int y)
{
   XR_Font_Surface *fs;
   Xrender_Surface *target_surface;
   xcb_rectangle_t  rect;
   int              r;
   int              g;
   int              b;
   int              a;

   fs = fg->ext_dat;
   if (!fs || !fs->xinf || !dc || !dc->col.col) return;
   target_surface = (Xrender_Surface *)(surface->image.data);
   a = (dc->col.col >> 24) & 0xff;
   r = (dc->col.col >> 16) & 0xff;
   g = (dc->col.col >> 8 ) & 0xff;
   b = (dc->col.col      ) & 0xff;
   if ((fs->xinf->mul_r != r) || (fs->xinf->mul_g != g) ||
       (fs->xinf->mul_b != b) || (fs->xinf->mul_a != a))
     {
	fs->xinf->mul_r = r;
	fs->xinf->mul_g = g;
	fs->xinf->mul_b = b;
	fs->xinf->mul_a = a;
	_xr_xcb_render_surface_solid_rectangle_set(fs->xinf->mul, r, g, b, a, 0, 0, 1, 1);
     }
   rect.x = x;
   rect.y = y;
   rect.width = fs->w;
   rect.height = fs->h;
   if (dc->clip.use)
     {
	RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.width, rect.height,
			   dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
     }
   xcb_render_set_picture_clip_rectangles(target_surface->xinf->x11.connection,
                                          target_surface->x11.xcb.pic, 0, 0, 1, &rect);
   xcb_render_composite(fs->xinf->x11.connection, XCB_RENDER_PICT_OP_OVER,
                        fs->xinf->mul->x11.xcb.pic,
                        fs->pic,
                        target_surface->x11.xcb.pic,
                        0, 0,
                        0, 0,
                        x, y,
                        fs->w, fs->h);
}
