#include "evas_common.h"
#include "evas_engine.h"
#include "evas_private.h"
#include "Evas_Engine_Software_Qtopia.h"
#include <sys/time.h>
#include <sys/utsname.h>

void
evas_qtopia_outbuf_software_qtopia_init(void)
{
}

void
evas_qtopia_outbuf_software_qtopia_free(Outbuf *buf)
{
   free(buf);
}

Outbuf *
evas_qtopia_outbuf_software_qtopia_setup_q(int w, int h, int rot, Outbuf_Depth depth, QWidget *target)
{
   Outbuf *buf;

   buf = calloc(1, sizeof(Outbuf));
   if (!buf)
     {
	free(buf);
	return NULL;
     }

   buf->w = w;
   buf->h = h;
   buf->rot = rot;
   buf->priv.target = target;
     {
	QT_Direct *direct;

	direct = evas_qt_main_direct_start(buf->priv.target);
	if (direct)
	  {
	     buf->w = direct->location.w;
	     buf->h = direct->location.h;
	     buf->rot = direct->fb.rotation;
	     evas_qt_main_direct_stop(direct);
	  }
     }
   return buf;
}

void
evas_qtopia_outbuf_software_qtopia_blit(Outbuf *buf, int src_x, int src_y, int w, int h, int dst_x, int dst_y)
{
}

void
evas_qtopia_outbuf_software_qtopia_update(Outbuf *buf, int x, int y, int w, int h)
{
}

RGBA_Image *
evas_qtopia_outbuf_software_qtopia_new_region_for_update(Outbuf *buf, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch)
{
   RGBA_Image *im;

   *cx = 0; *cy = 0; *cw = w; *ch = h;
   im = evas_common_image_create(w, h);
   return im;
}

void
evas_qtopia_outbuf_software_qtopia_free_region_for_update(Outbuf *buf, RGBA_Image *update)
{
   evas_common_image_free(update);
}

void
evas_qtopia_outbuf_software_qtopia_push_updated_region(Outbuf *buf, RGBA_Image *update, int x, int y, int w, int h)
{
   Gfx_Func_Convert conv_func;
   DATA8 *data;
   QT_Direct *d;

   data = NULL;
   conv_func = NULL;

   d = evas_qt_main_direct_start(buf->priv.target);
   if (d)
     {
	int i;

	for (i = 0; i < d->rects.count; i++)
	  {
	     int fb_bytes;
	     DATA8 *fb_mem;
	     int ox, oy, ow, oh;
	     int rx, ry, rw, rh;

	     ox = x; oy = y; ow = w; oh = h;
	     rx = d->rects.r[i].x - d->location.x;
	     ry = d->rects.r[i].y - d->location.y;
	     rw = d->rects.r[i].w;
	     rh = d->rects.r[i].h;
	     if (x < rx)
	       {
		  w += (x - rx);
		  x = rx;
	       }
	     if (y < ry)
	       {
		  h += (y - ry);
		  y = ry;
	       }
	     if ((w <= 0) || (h <= 0)) goto nope;
	     if ((x + w) > (rx + rw)) w = (rx + rw - x);
	     if ((y + h) > (ry + rh)) h = (ry + rh - y);
	     if ((w <= 0) || (h <= 0)) goto nope;
	     fb_bytes = d->fb.bpp / 8;
	     if (buf->rot == 0)
	       {
		  fb_mem = d->fb.data +
		    ((d->location.y * d->fb.width * fb_bytes) +
		     (d->location.x * fb_bytes));
		  data = fb_mem + (fb_bytes * (x + (y * d->fb.width)));
		  conv_func = evas_common_convert_func_get(data, w, h, d->fb.bpp,
					       d->fb.mask.r, d->fb.mask.g, d->fb.mask.b,
					       PAL_MODE_NONE, buf->rot);
	       }
	     else if (buf->rot == 180)
	       {
		 // FIXME
	       }
	     else if (buf->rot == 270)
	       {
                  fb_mem = d->fb.data +
		    (fb_bytes *
		     ((d->location.x * d->fb.width) +
		      (d->fb.width - d->location.y - d->location.h)));
		  data = fb_mem + (fb_bytes * (buf->h - y - h + (x * d->fb.width)));
		  conv_func = evas_common_convert_func_get(data, h, w, d->fb.bpp,
					       d->fb.mask.r, d->fb.mask.g, d->fb.mask.b,
					       PAL_MODE_NONE, buf->rot);
	       }
	     else if (buf->rot == 90)
	       {
		  // FIXME: wrong fb_mem
		  fb_mem = d->fb.data +
		    ((d->location.y * d->fb.width * fb_bytes) +
		     (d->location.x * fb_bytes));
		  data = fb_mem + (fb_bytes * (y + ((buf->w - x - w) * d->fb.width)));
		  conv_func = evas_common_convert_func_get(data, h, w, d->fb.bpp,
					       d->fb.mask.r, d->fb.mask.g, d->fb.mask.b,
					       PAL_MODE_NONE, buf->rot);
	       }
	     if (conv_func)
	       {
		  DATA32 *src_data;

		  if (buf->rot == 0)
		    {
		       src_data = update->image->data +
			 (ow * (y - oy)) + (x - ox);
		       conv_func(src_data, data,
				 ow - w,
				 d->fb.width - w,
				 w, h,
				 x, y, NULL);
		    }
		  else if (buf->rot == 180)
		    {
		      // FIXME
		    }
		  else if (buf->rot == 270)
		    {
		       src_data = update->image->data +
			 (ow * (y - oy)) + (x - ox);
		       conv_func(src_data, data,
				 ow - w,
				 d->fb.width - h,
				 h, w,
				 x, y, NULL);
		    }
		  else if (buf->rot == 90)
		    {
		       src_data = update->image->data +
			 (ow * (y - oy)) + (x - ox);
		       conv_func(src_data, data,
				 ow - w,
				 d->fb.width - h,
				 h, w,
				 x, y, NULL);
		    }
	       }
	     nope:;
	     x = ox; y = oy; w = ow; h = oh;
	  }
	d->updated.x = x; d->updated.y = y; d->updated.w = w; d->updated.h = h;
	evas_qt_main_direct_stop(d);
     }
}

void
evas_qtopia_outbuf_software_qtopia_reconfigure(Outbuf *buf, int w, int h, int rot, Outbuf_Depth depth)
{
   if ((w == buf->w) && (h == buf->h) &&
       (rot == buf->rot) && (depth == buf->depth))
     return;
   buf->w = w;
   buf->h = h;
   buf->rot = rot;
}

int
evas_qtopia_outbuf_software_qtopia_get_width(Outbuf *buf)
{
   return buf->w;
}

int
evas_qtopia_outbuf_software_qtopia_get_height(Outbuf *buf)
{
   return buf->h;
}

Outbuf_Depth
evas_qtopia_outbuf_software_qtopia_get_depth(Outbuf *buf)
{
   return buf->depth;
}

int
evas_qtopia_outbuf_software_qtopia_get_rot(Outbuf *buf)
{
   return buf->rot;
}

int
evas_qtopia_outbuf_software_qtopia_get_have_backbuf(Outbuf *buf)
{
   return 0;
}

void
evas_qtopia_outbuf_software_qtopia_set_have_backbuf(Outbuf *buf, int have_backbuf)
{
}
