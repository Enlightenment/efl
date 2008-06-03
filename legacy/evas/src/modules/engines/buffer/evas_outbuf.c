#include "evas_common.h"
#include "evas_engine.h"
#include <sys/time.h>

void
evas_buffer_outbuf_buf_init(void)
{
}

void
evas_buffer_outbuf_buf_free(Outbuf *buf)
{
   if (buf->priv.back_buf)
     {
        evas_cache_image_drop(&buf->priv.back_buf->cache_entry);
     }
   free(buf);
}

Outbuf *
evas_buffer_outbuf_buf_setup_fb(int w, int h, Outbuf_Depth depth, void *dest, int dest_row_bytes, int use_color_key, DATA32 color_key, int alpha_level,
				void * (*new_update_region) (int x, int y, int w, int h, int *row_bytes),
				void   (*free_update_region) (int x, int y, int w, int h, void *data)
				)
{
   Outbuf *buf;
   int y;
   int bpp;
   
   buf = calloc(1, sizeof(Outbuf));
   if (!buf) return NULL;

   buf->w = w;
   buf->h = h;
   buf->depth = depth;

   buf->dest = dest;
   buf->dest_row_bytes = dest_row_bytes;

   buf->alpha_level = alpha_level;
   buf->color_key = color_key;
   buf->use_color_key = use_color_key;

   buf->func.new_update_region = new_update_region;
   buf->func.free_update_region = free_update_region;

   bpp = sizeof(DATA32);
   if ((buf->depth == OUTBUF_DEPTH_RGB_24BPP_888_888) ||
       (buf->depth == OUTBUF_DEPTH_BGR_24BPP_888_888))
     bpp = 3;

   if ((buf->depth == OUTBUF_DEPTH_ARGB_32BPP_8888_8888) &&
       (buf->dest) && (buf->dest_row_bytes == (buf->w * sizeof(DATA32))))
     {
	for (y = 0; y < h; y++)
	  memset(((unsigned char *)(buf->dest)) + (y * buf->dest_row_bytes), 
		 0, w * bpp);
	buf->priv.back_buf = (RGBA_Image *) evas_cache_image_data(evas_common_image_cache_get(),
                                                                  w, h,
                                                                  buf->dest,
                                                                  1, EVAS_COLORSPACE_ARGB8888);
     }
   else if ((buf->depth == OUTBUF_DEPTH_RGB_32BPP_888_8888) &&
       (buf->dest) && (buf->dest_row_bytes == (buf->w * sizeof(DATA32))))
     {
        buf->priv.back_buf = (RGBA_Image *) evas_cache_image_data(evas_common_image_cache_get(),
                                                                  w, h,
                                                                  buf->dest,
                                                                  0, EVAS_COLORSPACE_ARGB8888);
     }

   return buf;
}

RGBA_Image *
evas_buffer_outbuf_buf_new_region_for_update(Outbuf *buf, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch)
{
   RGBA_Image *im;
   DATA32 *ptr;

   if (buf->priv.back_buf)
     {
	*cx = x; *cy = y; *cw = w; *ch = h;
	if (buf->priv.back_buf->cache_entry.flags.alpha)
	  {
	     int  ww = w;
	     ptr = buf->priv.back_buf->image.data + (y * buf->priv.back_buf->cache_entry.w) + x;
	     while (h--)
	       {
		  while (w--)
		    *ptr++ = 0;
		  w = ww;
		  ptr += (buf->priv.back_buf->cache_entry.w - w);
	       }
	  }
	return buf->priv.back_buf;
     }
   else
     {
	*cx = 0; *cy = 0; *cw = w; *ch = h;
	im = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
        if (im)
          {
	     if (((buf->depth == OUTBUF_DEPTH_ARGB_32BPP_8888_8888)) ||
		 ((buf->depth == OUTBUF_DEPTH_BGRA_32BPP_8888_8888)))
	       {
		  im->cache_entry.flags.alpha = 1;
                  im = (RGBA_Image *) evas_cache_image_size_set(&im->cache_entry, w, h);
                  if (im)
                    {
                       memset(im->image.data, 0, w * h * sizeof(DATA32));
                    }
               }
          }
     }
   return im;
}

void
evas_buffer_outbuf_buf_free_region_for_update(Outbuf *buf, RGBA_Image *update)
{
   if (update != buf->priv.back_buf) evas_cache_image_drop(&update->cache_entry);
}

void
evas_buffer_outbuf_buf_push_updated_region(Outbuf *buf, RGBA_Image *update, int x, int y, int w, int h)
{
   /* copy update image to out buf & convert */
   switch (buf->depth)
     {
      case OUTBUF_DEPTH_RGB_24BPP_888_888:
	/* copy & pack into 24bpp - if colorkey is enabled... etc. */
	  {
	     DATA8 thresh;
	     int xx, yy;
	     int row_bytes;
	     DATA8 *dest;
	     DATA32 colorkey;
	     DATA32 *src;
	     DATA8 *dst;

	     colorkey = buf->color_key;
	     thresh = buf->alpha_level;
	     row_bytes = buf->dest_row_bytes;
	     dest = (DATA8 *)(buf->dest) + (y * row_bytes) + (x * 3);
	     if (buf->func.new_update_region)
	       {
		  dest = buf->func.new_update_region(x, y, w, h, &row_bytes);
	       }
	     if (!dest) break;
	     if (buf->use_color_key)
	       {
		  for (yy = 0; yy < h; yy++)
		    {
		       dst = dest + (yy * row_bytes);
		       src = update->image.data + (yy * update->cache_entry.w);
		       for (xx = 0; xx < w; xx++)
			 {
			    if (A_VAL(src) > thresh)
			      {
				 *dst++ = R_VAL(src);
				 *dst++ = G_VAL(src);
				 *dst++ = B_VAL(src);
			      }
			    else
			      {
				 *dst++ = R_VAL(&colorkey);
				 *dst++ = G_VAL(&colorkey);
				 *dst++ = B_VAL(&colorkey);
			      }
			    src++;
			 }
		    }
	       }
	     else
	       {
		  for (yy = 0; yy < h; yy++)
		    {
		       dst = dest + (yy * row_bytes);
		       src = update->image.data + (yy * update->cache_entry.w);
		       for (xx = 0; xx < w; xx++)
			 {
			    *dst++ = R_VAL(src);
			    *dst++ = G_VAL(src);
			    *dst++ = B_VAL(src);
			    src++;
			 }
		    }
	       }
	     if (buf->func.free_update_region)
	       {
		  buf->func.free_update_region(x, y, w, h, dest);
	       }
	  }
	break;
      case OUTBUF_DEPTH_BGR_24BPP_888_888:
	/* copy & pack into 24bpp - if colorkey is enabled... etc. */
	  {
	     DATA8 thresh;
	     int xx, yy;
	     int row_bytes;
	     DATA8 *dest;
	     DATA32 colorkey;
	     DATA32 *src;
	     DATA8 *dst;

	     colorkey = buf->color_key;
	     thresh = buf->alpha_level;
	     row_bytes = buf->dest_row_bytes;
	     dest = (DATA8 *)(buf->dest) + (y * row_bytes) + (x * 3);
	     if (buf->func.new_update_region)
	       {
		  dest = buf->func.new_update_region(x, y, w, h, &row_bytes);
	       }
	     if (!dest) break;
	     if (buf->use_color_key)
	       {
		  for (yy = 0; yy < h; yy++)
		    {
		       dst = dest + (yy * row_bytes);
		       src = update->image.data + (yy * update->cache_entry.w);
		       for (xx = 0; xx < w; xx++)
			 {
			    if (A_VAL(src) > thresh)
			      {
				 *dst++ = B_VAL(src);
				 *dst++ = G_VAL(src);
				 *dst++ = R_VAL(src);
			      }
			    else
			      {
				 *dst++ = B_VAL(&colorkey);
				 *dst++ = G_VAL(&colorkey);
				 *dst++ = R_VAL(&colorkey);
			      }
			    src++;
			 }
		    }
	       }
	     else
	       {
		  for (yy = 0; yy < h; yy++)
		    {
		       dst = dest + (yy * row_bytes);
		       src = update->image.data + (yy * update->cache_entry.w);
		       for (xx = 0; xx < w; xx++)
			 {
			    *dst++ = B_VAL(src);
			    *dst++ = G_VAL(src);
			    *dst++ = R_VAL(src);
			    src++;
			 }
		    }
	       }
	     if (buf->func.free_update_region)
	       {
		  buf->func.free_update_region(x, y, w, h, dest);
	       }
	  }
	break;
      case OUTBUF_DEPTH_RGB_32BPP_888_8888:
      case OUTBUF_DEPTH_ARGB_32BPP_8888_8888:
	  {
	     DATA32 *dest, *src, *dst;
	     int yy, row_bytes;

	     row_bytes = buf->dest_row_bytes;
	     dest = (DATA8 *)(buf->dest) + (y * row_bytes) + (x * 4);
	     if (buf->func.new_update_region)
	       {
		  dest = buf->func.new_update_region(x, y, w, h, &row_bytes);
	       }
	     /* no need src == dest */
	     if (!buf->priv.back_buf)
	       {
		  Gfx_Func_Copy func;
		  
		  func = evas_common_draw_func_copy_get(w, 0);
		  if (func)
		    {
		       for (yy = 0; yy < h; yy++)
			 {
			    src = update->image.data + (yy * update->cache_entry.w);
			    dst = (DATA8 *)(buf->dest) + ((y + yy) * row_bytes);
			    func(src, dst, w);
			 }
		       
		    }
	       }
	     if (buf->func.free_update_region)
	       {
		  buf->func.free_update_region(x, y, w, h, dest);
	       }
	  }
	break;
      case OUTBUF_DEPTH_BGR_32BPP_888_8888:
	  {
	     DATA32 *src, *dst;
	     DATA8 *dest;
	     int xx, yy, row_bytes;
	     
	     row_bytes = buf->dest_row_bytes;
	     dest = (DATA8 *)(buf->dest) + (y * row_bytes) + (x * 4);
	     if (buf->func.new_update_region)
	       {
		  dest = buf->func.new_update_region(x, y, w, h, &row_bytes);
	       }
	     for (yy = 0; yy < h; yy++)
	       {
		  dst = dest + (yy * row_bytes);
		  src = update->image.data + (yy * update->cache_entry.w);
		  for (xx = 0; xx < w; xx++)
		    {
		       A_VAL(dst) = B_VAL(src);
		       R_VAL(dst) = G_VAL(src);
		       G_VAL(dst) = R_VAL(src);
		       dst++;
		       src++;
		    }
	       }
	     if (buf->func.free_update_region)
	       {
		  buf->func.free_update_region(x, y, w, h, dest);
	       }
	 }
	break;
      case OUTBUF_DEPTH_BGRA_32BPP_8888_8888:
	  {
	     DATA32 *src, *dst;
	     DATA8 *dest;
	     int xx, yy, row_bytes;
	     
	     row_bytes = buf->dest_row_bytes;
	     dest = (DATA8 *)(buf->dest) + (y * row_bytes) + (x * 4);
	     if (buf->func.new_update_region)
	       {
		  dest = buf->func.new_update_region(x, y, w, h, &row_bytes);
	       }
	     for (yy = 0; yy < h; yy++)
	       {
		  dst = dest + (yy * row_bytes);
		  src = update->image.data + (yy * update->cache_entry.w);
		  for (xx = 0; xx < w; xx++)
		    {
		       A_VAL(dst) = B_VAL(src);
		       R_VAL(dst) = G_VAL(src);
		       G_VAL(dst) = R_VAL(src);
		       dst++;
		       src++;
		    }
	       }
	     if (buf->func.free_update_region)
	       {
		  buf->func.free_update_region(x, y, w, h, dest);
	       }
	 }
	break;
      default:
	break;
     }
}
