#include "evas_common.h"
#include "evas_engine.h"
#include <sys/time.h>
#include <sys/utsname.h>

void
evas_fb_outbuf_fb_init(void)
{
}

void
evas_fb_outbuf_fb_free(Outbuf *buf)
{
   /* FIXME: impliment */
   printf("destroying fb info.. not implemented!!!! WARNING. LEAK!\n");
   if (buf->priv.back_buf)
     evas_cache_image_drop(&buf->priv.back_buf->cache_entry);
   free(buf);
}

Outbuf *
evas_fb_outbuf_fb_setup_fb(int w, int h, int rot, Outbuf_Depth depth, int vt_no, int dev_no, int refresh)
{
   /* create outbuf struct */
   /* setup window and/or fb */
   /* if (dithered) create backbuf */
   Outbuf *buf;
   int fb_fd = -1;
   int fb_depth;

   fb_depth = -1;
   if (depth == OUTBUF_DEPTH_RGB_16BPP_565_565_DITHERED) fb_depth = 16;
   else if (depth == OUTBUF_DEPTH_RGB_16BPP_555_555_DITHERED) fb_depth = 15;
   else if (depth == OUTBUF_DEPTH_RGB_16BPP_565_444_DITHERED) fb_depth = 16;
   else if (depth == OUTBUF_DEPTH_RGB_16BPP_444_444_DITHERED) fb_depth = 12;
   else if (depth == OUTBUF_DEPTH_RGB_32BPP_888_8888) fb_depth = 32;
   else if (depth == OUTBUF_DEPTH_INHERIT) fb_depth = 0;
   buf = calloc(1, sizeof(Outbuf));
   if (!buf)
     return NULL;

   fb_init(vt_no, dev_no);
   if (rot == 0 || rot == 180)
     buf->priv.fb.fb = fb_setmode(w, h, fb_depth, refresh);
   else if (rot == 90 || rot == 270)
     buf->priv.fb.fb = fb_setmode(h, w, fb_depth, refresh);
   if (!buf->priv.fb.fb) buf->priv.fb.fb = fb_getmode();
   if (!buf->priv.fb.fb)
     {
	free(buf);
	return NULL;
     }
   fb_fd = fb_postinit(buf->priv.fb.fb);

   if (rot == 0 || rot == 180)
     {
 	buf->w = buf->priv.fb.fb->width;
	buf->h = buf->priv.fb.fb->height;
     }
   else if (rot == 90 || rot == 270)
     {
 	buf->w = buf->priv.fb.fb->height;
	buf->h = buf->priv.fb.fb->width;
     }

   buf->depth = depth;
   buf->rot = rot;

     {
	Gfx_Func_Convert conv_func;
	int i;

	buf->priv.mask.r = 0;
	for (i = 0; i < (int)buf->priv.fb.fb->fb_var.red.length; i++)
	  buf->priv.mask.r |= (1 << (buf->priv.fb.fb->fb_var.red.offset + i));
	buf->priv.mask.g = 0;
	for (i = 0; i < (int)buf->priv.fb.fb->fb_var.green.length; i++)
	  buf->priv.mask.g |= (1 << (buf->priv.fb.fb->fb_var.green.offset + i));
	buf->priv.mask.b = 0;
	for (i = 0; i < (int)buf->priv.fb.fb->fb_var.blue.length; i++)
	  buf->priv.mask.b |= (1 << (buf->priv.fb.fb->fb_var.blue.offset + i));

	conv_func = NULL;
	if (buf->rot == 0 || buf->rot == 180)
	  conv_func = evas_common_convert_func_get(0, buf->w, buf->h,
				       buf->priv.fb.fb->fb_var.bits_per_pixel,
				       buf->priv.mask.r,
				       buf->priv.mask.g,
				       buf->priv.mask.b,
				       PAL_MODE_NONE,
				       buf->rot);
	else if (buf->rot == 90 || buf->rot == 270)
	  conv_func = evas_common_convert_func_get(0, buf->h, buf->w,
				       buf->priv.fb.fb->fb_var.bits_per_pixel,
				       buf->priv.mask.r,
				       buf->priv.mask.g,
				       buf->priv.mask.b,
				       PAL_MODE_NONE,
				       buf->rot);
       if (!conv_func)
	  {
	     free(buf);
	     return NULL;
	  }
     }
//   if (buf->priv.fb.fb->fb_var.bits_per_pixel < 24)
//     buf->priv.back_buf = evas_common_image_create(buf->w, buf->h);

   return buf;
}

void
evas_fb_outbuf_fb_blit(Outbuf *buf, int src_x, int src_y, int w, int h, int dst_x, int dst_y)
{
   if (buf->priv.back_buf)
     {
	evas_common_blit_rectangle(buf->priv.back_buf, buf->priv.back_buf,
		       src_x, src_y, w, h, dst_x, dst_y);
	evas_fb_outbuf_fb_update(buf, dst_x, dst_y, w, h);
     }
   else
     {
	if (buf->priv.fb.fb)
	  {
	     /* FIXME: need to impliment an fb call for "copy area" */
	  }
     }
}

void
evas_fb_outbuf_fb_update(Outbuf *buf, int x, int y, int w, int h)
{
   if (!(buf->priv.back_buf)) return;
   if (buf->priv.fb.fb)
     {
	Gfx_Func_Convert conv_func;
	DATA8 *data;

	data = NULL;
	conv_func = NULL;
        if (buf->rot == 0)
	  {
	     data = (DATA8 *)buf->priv.fb.fb->mem + buf->priv.fb.fb->mem_offset +
	       buf->priv.fb.fb->bpp *
	       (x + (y * buf->priv.fb.fb->width));
	     conv_func = evas_common_convert_func_get(data, w, h, buf->priv.fb.fb->fb_var.bits_per_pixel,
					  buf->priv.mask.r, buf->priv.mask.g,
					  buf->priv.mask.b, PAL_MODE_NONE,
					  buf->rot);
	  }
        else if (buf->rot == 180)
          {
             data = (DATA8 *)buf->priv.fb.fb->mem + buf->priv.fb.fb->mem_offset +
               buf->priv.fb.fb->bpp *
               (buf->w - x - w + ((buf->h - y - h) * buf->priv.fb.fb->width));
             conv_func = evas_common_convert_func_get(data, w, h, buf->priv.fb.fb->fb_var.bits_per_pixel,
                                          buf->priv.mask.r, buf->priv.mask.g,
                                          buf->priv.mask.b, PAL_MODE_NONE,
                                          buf->rot);
          }
	else if (buf->rot == 270)
	  {
	     data = (DATA8 *)buf->priv.fb.fb->mem + buf->priv.fb.fb->mem_offset +
	       buf->priv.fb.fb->bpp *
	       (buf->h - y - h + (x * buf->priv.fb.fb->width));
	     conv_func = evas_common_convert_func_get(data, h, w, buf->priv.fb.fb->fb_var.bits_per_pixel,
					  buf->priv.mask.r, buf->priv.mask.g,
					  buf->priv.mask.b, PAL_MODE_NONE,
					  buf->rot);
	  }
	else if (buf->rot == 90)
	  {
	     data = (DATA8 *)buf->priv.fb.fb->mem + buf->priv.fb.fb->mem_offset +
	       buf->priv.fb.fb->bpp *
	       (y + ((buf->w - x - w) * buf->priv.fb.fb->width));
	     conv_func = evas_common_convert_func_get(data, h, w, buf->priv.fb.fb->fb_var.bits_per_pixel,
					  buf->priv.mask.r, buf->priv.mask.g,
					  buf->priv.mask.b, PAL_MODE_NONE,
					  buf->rot);
	  }
	if (conv_func)
	  {
	     DATA32 *src_data;

	     src_data = buf->priv.back_buf->image.data + (y * buf->w) + x;
	     if (buf->rot == 0 || buf->rot == 180)
	       {
		  conv_func(src_data, data,
			    buf->w - w,
			    buf->priv.fb.fb->width - w,
			    w, h,
			    x, y, NULL);
	       }
	     else if (buf->rot == 90 || buf->rot == 270)
	       {
		  conv_func(src_data, data,
			    buf->w - w,
			    buf->priv.fb.fb->width - h,
			    h, w,
			    x, y, NULL);
	       }
	  }
     }
}

RGBA_Image *
evas_fb_outbuf_fb_new_region_for_update(Outbuf *buf, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch)
{
   if (buf->priv.back_buf)
     {
	*cx = x; *cy = y; *cw = w; *ch = h;
	return buf->priv.back_buf;
     }
   else
     {
	RGBA_Image *im;

	*cx = 0; *cy = 0; *cw = w; *ch = h;
	im = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
        im->cache_entry.flags.alpha = 1;
        im = (RGBA_Image *) evas_cache_image_size_set(&im->cache_entry, w, h);

        /* handle framebuffers with alpha channel */
        if (buf->priv.fb.fb->fb_var.transp.length > 0) {
           memset(im->image.data, 0, w * h * sizeof(DATA32));
        }
        return im;
     }
   return NULL;
}

void
evas_fb_outbuf_fb_free_region_for_update(Outbuf *buf, RGBA_Image *update)
{
   if (update != buf->priv.back_buf) evas_cache_image_drop(&update->cache_entry);
}

void
evas_fb_outbuf_fb_push_updated_region(Outbuf *buf, RGBA_Image *update, int x, int y, int w, int h)
{
   if (!buf->priv.fb.fb) return;
   if (buf->priv.back_buf)
     {
	if (update != buf->priv.back_buf)
	  evas_common_blit_rectangle(update, buf->priv.back_buf,
			 0, 0, w, h, x, y);
	evas_fb_outbuf_fb_update(buf, x, y, w, h);
     }
   else
     {
	Gfx_Func_Convert conv_func;
	DATA8 *data;

	data = NULL;
	conv_func = NULL;
	if (buf->rot == 0)
	  {
	     data = (DATA8 *)buf->priv.fb.fb->mem +
	       buf->priv.fb.fb->mem_offset +
	       buf->priv.fb.fb->bpp *
	       (x + (y * buf->priv.fb.fb->width));
	     conv_func = evas_common_convert_func_get(data, w, h,
					  buf->priv.fb.fb->fb_var.bits_per_pixel,
					  buf->priv.mask.r, buf->priv.mask.g,
					  buf->priv.mask.b, PAL_MODE_NONE,
					  buf->rot);
	  }
        else if (buf->rot == 180)  
          {
             data = (DATA8 *)buf->priv.fb.fb->mem +
               buf->priv.fb.fb->mem_offset +
               buf->priv.fb.fb->bpp *  
               (buf->w - x - w + ((buf->h - y - h) * buf->priv.fb.fb->width));
             conv_func = evas_common_convert_func_get(data, w, h,
                                          buf->priv.fb.fb->fb_var.bits_per_pixel,
                                          buf->priv.mask.r, buf->priv.mask.g,
                                          buf->priv.mask.b, PAL_MODE_NONE,
                                          buf->rot);
          }
	else if (buf->rot == 270)
	  {
	     data = (DATA8 *)buf->priv.fb.fb->mem +
	       buf->priv.fb.fb->mem_offset +
	       buf->priv.fb.fb->bpp *
	       (buf->h - y - h + (x * buf->priv.fb.fb->width));
	     conv_func = evas_common_convert_func_get(data, h, w,
					  buf->priv.fb.fb->fb_var.bits_per_pixel,
					  buf->priv.mask.r, buf->priv.mask.g,
					  buf->priv.mask.b, PAL_MODE_NONE,
					  buf->rot);
	  }
	else if (buf->rot == 90)
	  {
	     data = (DATA8 *)buf->priv.fb.fb->mem +
	       buf->priv.fb.fb->mem_offset +
	       buf->priv.fb.fb->bpp *
	       (y + ((buf->w - x - w) * buf->priv.fb.fb->width));
	     conv_func = evas_common_convert_func_get(data, h, w,
					  buf->priv.fb.fb->fb_var.bits_per_pixel,
					  buf->priv.mask.r, buf->priv.mask.g,
					  buf->priv.mask.b, PAL_MODE_NONE,
					  buf->rot);
	  }
	if (conv_func)
	  {
	     DATA32 *src_data;

	     src_data = update->image.data;
	     if (buf->rot == 0 || buf->rot == 180)
	       {
		  conv_func(src_data, data,
			    0,
			    buf->priv.fb.fb->width - w,
			    w, h,
			    x, y, NULL);
	       }
	     else if (buf->rot == 90 || buf->rot == 270)
	       {
		  conv_func(src_data, data,
			    0,
			    buf->priv.fb.fb->width - h,
			    h, w,
			    x, y, NULL);
	       }
	  }
     }
}

void
evas_fb_outbuf_fb_reconfigure(Outbuf *buf, int w, int h, int rot, Outbuf_Depth depth)
{
   if ((w == buf->w) && (h == buf->h) &&
       (rot == buf->rot) && (depth == buf->depth))
     return;
   if (buf->priv.back_buf)
     {
	evas_cache_image_drop(&buf->priv.back_buf->cache_entry);
	buf->priv.back_buf = NULL;
     }
   if (buf->priv.fb.fb)
     {
	/* FIXME: impliment this */
     }
   /* if backbuf delet it */
   /* resize window or reset fb mode */
   /* if (dithered) create new backbuf */
}

int
evas_fb_outbuf_fb_get_width(Outbuf *buf)
{
   return buf->w;
}

int
evas_fb_outbuf_fb_get_height(Outbuf *buf)
{
   return buf->h;
}

Outbuf_Depth
evas_fb_outbuf_fb_get_depth(Outbuf *buf)
{
   return buf->depth;
}

int
evas_fb_outbuf_fb_get_rot(Outbuf *buf)
{
   return buf->rot;
}

int
evas_fb_outbuf_fb_get_have_backbuf(Outbuf *buf)
{
   if (buf->priv.back_buf) return 1;
   return 0;
}

void
evas_fb_outbuf_fb_set_have_backbuf(Outbuf *buf, int have_backbuf)
{
   if (buf->priv.back_buf)
     {
	if (have_backbuf) return;
        evas_cache_image_drop(&buf->priv.back_buf->cache_entry);
	buf->priv.back_buf = NULL;
     }
   else
     {
	if (!have_backbuf) return;
	if (buf->priv.fb.fb)
	  {
	     if (buf->priv.fb.fb->fb_var.bits_per_pixel  < 24)
	       {
		  buf->priv.back_buf = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
                  buf->priv.back_buf = (RGBA_Image *) evas_cache_image_size_set(&buf->priv.back_buf->cache_entry, buf->w, buf->h);
	       }
	  }
     }
}
